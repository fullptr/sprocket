#include "Scene3DRenderer.h"

#include <Sprocket/Graphics/asset_manager.h>
#include <Sprocket/Graphics/buffer.h>
#include <Sprocket/Graphics/camera.h>
#include <Sprocket/Graphics/open_gl.h>
#include <Sprocket/Graphics/render_context.h>
#include <Sprocket/Utility/Hashing.h>
#include <Sprocket/Utility/Maths.h>
#include <Sprocket/Utility/views.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace spkt {
namespace {

std::array<glm::mat4, MAX_BONES> DefaultBoneTransforms() {
    std::array<glm::mat4, MAX_BONES> arr;
    std::ranges::fill(arr, glm::mat4(1.0));
    return arr;
};

void upload_uniforms(
    const shader& shader,
    const spkt::registry& registry,
    const glm::mat4& proj,
    const glm::mat4& view
)
{
    shader.bind();
    
    // Load point lights to shader
    std::array<glm::vec3, MAX_NUM_LIGHTS> positions = {};
    std::array<glm::vec3, MAX_NUM_LIGHTS> colours = {};
    std::array<float, MAX_NUM_LIGHTS> brightnesses = {};

    for (auto [index, data] : registry.view_get<LightComponent, Transform3DComponent>()
                            | std::views::take(MAX_NUM_LIGHTS)
                            | spkt::views::enumerate())
    {
        auto [light, transform] = data;
        positions[index] = transform.position;
        colours[index] = light.colour;
        brightnesses[index] = light.brightness;
    }

    shader.load("u_light_pos", positions);
    shader.load("u_light_colour", colours);
    shader.load("u_light_brightness", brightnesses);
}

void upload_material(
    const shader& shader,
    const material& material,
    asset_manager* assetManager
)
{
    assetManager->get<texture>(material.albedoMap).bind(ALBEDO_SLOT);
    assetManager->get<texture>(material.normalMap).bind(NORMAL_SLOT);
    assetManager->get<texture>(material.metallicMap).bind(METALLIC_SLOT);
    assetManager->get<texture>(material.roughnessMap).bind(ROUGHNESS_SLOT);

    shader.load("u_use_albedo_map", material.useAlbedoMap ? 1.0f : 0.0f);
    shader.load("u_use_normal_map", material.useNormalMap ? 1.0f : 0.0f);
    shader.load("u_use_metallic_map", material.useMetallicMap ? 1.0f : 0.0f);
    shader.load("u_use_roughness_map", material.useRoughnessMap ? 1.0f : 0.0f);

    shader.load("u_albedo", material.albedo);
    shader.load("u_roughness", material.roughness);
    shader.load("u_metallic", material.metallic);
}

}

Scene3DRenderer::Scene3DRenderer(asset_manager* assetManager)
    : d_assetManager(assetManager)
    , d_staticShader("Resources/Shaders/Entity_PBR_Static.vert", "Resources/Shaders/Entity_PBR.frag")
    , d_animatedShader("Resources/Shaders/Entity_PBR_Animated.vert", "Resources/Shaders/Entity_PBR.frag")
    , d_instanceBuffer()
{
    d_staticShader.load("u_albedo_map", ALBEDO_SLOT);
    d_staticShader.load("u_normal_map", NORMAL_SLOT);
    d_staticShader.load("u_metallic_map", METALLIC_SLOT);
    d_staticShader.load("u_roughness_map", ROUGHNESS_SLOT);
    d_staticShader.load("shadow_map", SHADOW_MAP_SLOT);

    d_animatedShader.load("u_albedo_map", ALBEDO_SLOT);
    d_animatedShader.load("u_normal_map", NORMAL_SLOT);
    d_animatedShader.load("u_metallic_map", METALLIC_SLOT);
    d_animatedShader.load("u_roughness_map", ROUGHNESS_SLOT);
    d_animatedShader.load("shadow_map", SHADOW_MAP_SLOT);
}

void Scene3DRenderer::EnableShadows(const shadow_map& shadowMap)
{
    d_staticShader.load("u_light_proj_view", shadowMap.get_light_proj_view());
    d_animatedShader.load("u_light_proj_view", shadowMap.get_light_proj_view());
    shadowMap.get_texture().bind(SHADOW_MAP_SLOT);
}

void Scene3DRenderer::Draw(
    const spkt::registry& registry,
    const glm::mat4& proj,
    const glm::mat4& view)
{
    begin_frame(proj, view);
    spkt::render_context rc;
    rc.face_culling(true);
    rc.depth_testing(true);

    if (auto a = registry.find<AmbienceComponent>(); registry.valid(a)) {
        const auto& ambience = registry.get<AmbienceComponent>(a);
        set_ambience(ambience.colour, ambience.brightness);
    }

    if (auto s = registry.find<SunComponent>(); registry.valid(s)) {
        const auto& sun = registry.get<SunComponent>(s);
        set_sunlight(sun.colour, sun.direction, sun.brightness);
    }

    std::array<glm::vec3, MAX_NUM_LIGHTS> positions = {};
    std::array<glm::vec3, MAX_NUM_LIGHTS> colours = {};
    std::array<float, MAX_NUM_LIGHTS> brightnesses = {};
    for (auto [index, data] : registry.view_get<LightComponent, Transform3DComponent>()
                            | std::views::take(MAX_NUM_LIGHTS)
                            | spkt::views::enumerate())
    {
        auto [light, transform] = data;
        positions[index] = transform.position;
        colours[index] = light.colour;
        brightnesses[index] = light.brightness;
    }
    set_lights(positions, colours, brightnesses);

    for (auto [mc, tc] : registry.view_get<StaticModelComponent, Transform3DComponent>()) {
        draw_static_mesh(
            tc.position, tc.orientation, tc.scale,
            mc.mesh, mc.material
        );
    }

    for (auto [mc, tc] : registry.view_get<AnimatedModelComponent, Transform3DComponent>()) {
        draw_animated_mesh(
            tc.position, tc.orientation, tc.scale,
            mc.mesh, mc.material,
            mc.animation_name, mc.animation_time
        );
    }

    for (auto [ps] : registry.view_get<ParticleSingleton>()) {
        std::vector<spkt::model_instance> instance_data(NUM_PARTICLES);
        for (const auto& particle : *ps.particles) {
            if (particle.life > 0.0) {
                instance_data.push_back({particle.position, {0.0, 0.0, 0.0, 1.0}, particle.scale});
            }
        }
        draw_particles(instance_data);
    };

    end_frame();
}

void Scene3DRenderer::Draw(const spkt::registry& registry, spkt::entity camera)
{
    auto [tc, cc] = registry.get_all<spkt::Transform3DComponent, spkt::Camera3DComponent>(camera);
    glm::mat4 view = spkt::make_view(tc.position, tc.orientation, cc.pitch);
    glm::mat4 proj = spkt::make_proj(cc.fov);
    Draw(registry, proj, view);
}

void Scene3DRenderer::begin_frame(const glm::mat4& proj, const glm::mat4& view)
{
    assert(!d_frame_data);
    d_frame_data = frame_data{};
    d_staticShader.bind();
    d_staticShader.load("u_proj_matrix", proj);
    d_staticShader.load("u_view_matrix", view);
    d_animatedShader.bind();
    d_animatedShader.load("u_proj_matrix", proj);
    d_animatedShader.load("u_view_matrix", view);
}

void Scene3DRenderer::end_frame()
{
    assert(d_frame_data);

    d_staticShader.bind();
    for (const auto& [key, data] : d_frame_data->static_mesh_draw_commands) {
        const auto& mesh = d_assetManager->get<static_mesh>(key.first);
        const auto& mat = d_assetManager->get<material>(key.second);

        upload_material(d_staticShader, mat, d_assetManager);
        d_instanceBuffer.set_data(data);
        spkt::draw(mesh, &d_instanceBuffer);
    }
    d_staticShader.unbind();

    d_frame_data = std::nullopt;
}

void Scene3DRenderer::set_ambience(const glm::vec3& colour, const float brightness)
{
    d_staticShader.bind();
    d_staticShader.load("u_ambience_colour", colour);
    d_staticShader.load("u_ambience_brightness", brightness);
    d_animatedShader.bind();
    d_animatedShader.load("u_ambience_colour", colour);
    d_animatedShader.load("u_ambience_brightness", brightness);
}

void Scene3DRenderer::set_sunlight(
    const glm::vec3& colour, const glm::vec3& direction, const float brightness)
{
    d_staticShader.bind();
    d_staticShader.load("u_sun_colour", colour);
    d_staticShader.load("u_sun_direction", direction);
    d_staticShader.load("u_sun_brightness", brightness);
    d_animatedShader.bind();
    d_animatedShader.load("u_sun_colour", colour);
    d_animatedShader.load("u_sun_direction", direction);
    d_animatedShader.load("u_sun_brightness", brightness);
}

void Scene3DRenderer::set_lights(
    std::span<const glm::vec3> positions,
    std::span<const glm::vec3> colours,
    std::span<const float> brightnesses)
{
    assert(positions.size() == colours.size());
    assert(positions.size() == brightnesses.size());
    assert(positions.size() <= MAX_NUM_LIGHTS);
    d_staticShader.bind();
    d_staticShader.load("u_light_pos", positions);
    d_staticShader.load("u_light_colour", colours);
    d_staticShader.load("u_light_brightness", brightnesses);
    d_animatedShader.bind();
    d_animatedShader.load("u_light_pos", positions);
    d_animatedShader.load("u_light_colour", colours);
    d_animatedShader.load("u_light_brightness", brightnesses);
}

void Scene3DRenderer::draw_static_mesh(
    const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale,
    const std::string& mesh, const std::string& material)
{
    assert(d_frame_data);
    d_frame_data->static_mesh_draw_commands[{mesh, material}].push_back({position, orientation, scale});
}

void Scene3DRenderer::draw_animated_mesh(
    const glm::vec3& position, const glm::quat& orientation, const glm::vec3& scale,
    const std::string& mesh, const std::string& material,
    const std::string& animation_name, float animation_time)
{
    assert(d_frame_data);
    d_animatedShader.bind();
    const auto& mesh_obj = d_assetManager->get<animated_mesh>(mesh);
    const auto& mat = d_assetManager->get<spkt::material>(material);
    upload_material(d_animatedShader, mat, d_assetManager);

    d_animatedShader.load("u_model_matrix", Maths::Transform(position, orientation, scale));
    
    auto poses = mesh_obj.get_pose(animation_name, animation_time);
    poses.resize(MAX_BONES, glm::mat4(1.0));
    d_animatedShader.load("u_bone_transforms", poses);

    spkt::draw(mesh_obj);
    d_animatedShader.unbind();
}

void Scene3DRenderer::draw_particles(std::span<const spkt::model_instance> particles)
{
    d_staticShader.bind();
    d_instanceBuffer.set_data(particles);

    // TODO: Un-hardcode this mesh, do when cleaning up the rendering.
    spkt::draw(d_assetManager->get<static_mesh>("Resources/Models/Particle.obj"), &d_instanceBuffer);
    d_staticShader.unbind();
}

}
#include "Anvil.h"

#include <Anvil/Inspector.h>
#include <Anvil/systems.h>
#include <Anvil/particle_system.h>
#include <Anvil/physics_system.h>

#include <Sprocket/Core/input_codes.h>
#include <Sprocket/Core/log.h>
#include <Sprocket/Graphics/camera.h>
#include <Sprocket/Graphics/material.h>
#include <Sprocket/Graphics/render_context.h>
#include <Sprocket/Scene/Loader.h>
#include <Sprocket/Scene/meta.h>
#include <Sprocket/UI/ImGuiXtra.h>
#include <Sprocket/Utility/FileBrowser.h>
#include <Sprocket/Utility/Maths.h>
#include <Sprocket/Vendor/imgui/imgui.h>

#include <glm/glm.hpp>

#include <string_view>
#include <ranges>

namespace {

template <typename T>
T& get_singleton(spkt::registry& reg)
{
    return reg.get<T>(reg.find<T>());
}

std::string entiy_name(spkt::registry& registry, spkt::entity entity)
{
    if (registry.has<spkt::NameComponent>(entity)) {
        return registry.get<spkt::NameComponent>(entity).name;
    }
    return "Entity";
}

bool SubstringCI(std::string_view string, std::string_view substr) {
    auto it = std::search(
        string.begin(), string.end(),
        substr.begin(), substr.end(),
        [] (char c1, char c2) { return std::toupper(c1) == std::toupper(c2); }
    );
    return it != string.end();
}

void draw_colliders(
    const spkt::geometry_renderer& renderer,
    const spkt::registry& registry,
    const glm::mat4& proj,
    const glm::mat4& view)
{
    spkt::render_context rc;
    rc.wireframe(true);

    renderer.begin_frame(proj, view);

    const auto& make_transform = [](const auto& a, const auto& b) {
        using namespace spkt::Maths;
        return Transform(a.position, a.orientation) * Transform(b.position, b.orientation);
    };

    for (auto [bc, tc] : registry.view_get<spkt::BoxCollider3DComponent, spkt::Transform3DComponent>()) {
        const glm::vec3 scale = bc.applyScale ? bc.halfExtents * tc.scale : bc.halfExtents;      
        renderer.draw_box(make_transform(tc, bc), scale);
    }

    for (auto [sc, tc] : registry.view_get<spkt::SphereCollider3DComponent, spkt::Transform3DComponent>()) {
        renderer.draw_sphere(make_transform(tc, sc), sc.radius);
    }

    for (auto [cc, tc] : registry.view_get<spkt::CapsuleCollider3DComponent, spkt::Transform3DComponent>()) {
        renderer.draw_capsule(make_transform(tc, cc), cc.radius, cc.height);
    }

    renderer.end_frame();
}

}

Anvil::Anvil(spkt::window* window)
    : d_window(window)
    , d_asset_manager()
    , d_entity_renderer(&d_asset_manager)
    , d_skybox_renderer(&d_asset_manager)
    , d_skybox({
        "Resources/Textures/Skybox/Skybox_X_Pos.png",
        "Resources/Textures/Skybox/Skybox_X_Neg.png",
        "Resources/Textures/Skybox/Skybox_Y_Pos.png",
        "Resources/Textures/Skybox/Skybox_Y_Neg.png",
        "Resources/Textures/Skybox/Skybox_Z_Pos.png",
        "Resources/Textures/Skybox/Skybox_Z_Neg.png"
    })
    , d_editor_camera(d_window, {0.0, 0.0, 0.0})
    , d_viewport(1280, 720)
    , d_ui(d_window)
{
    d_window->set_cursor_visibility(true);

    d_scene = std::make_shared<spkt::scene>();
    spkt::load_registry_from_file(d_sceneFile, d_scene->registry);
    d_activeScene = d_scene;
}

void Anvil::on_event(spkt::event& event)
{
    using namespace spkt;

    if (auto data = event.get_if<keyboard_pressed_event>()) {
        if (data->key == Keyboard::ESC) {
            if (d_playingGame) {
                d_playingGame = false;
                d_activeScene = d_scene;
                d_window->set_cursor_visibility(true);
            }
            else if (d_selected != spkt::null) {
                d_selected = spkt::null;
            }
            else if (d_window->is_fullscreen()) {
                d_window->set_windowed(1280, 720);
            }
            event.consume();
        } else if (data->key == Keyboard::F11) {
            if (d_window->is_fullscreen()) {
                d_window->set_windowed(1280, 720);
            }
            else {
                d_window->set_fullscreen();
            }
            event.consume();
        }
    }

    d_ui.on_event(event);
    d_activeScene->on_event(event);
    if (!d_playingGame) {
        d_editor_camera.on_event(event);
    }
}

void Anvil::on_update(double dt)
{
    auto& registry = d_activeScene->registry;

    d_ui.on_update(dt);

    if (d_paused) {
        return;
    }

    d_activeScene->on_update(dt);

    if (d_is_viewport_focused && !d_playingGame) {
        d_editor_camera.on_update(dt);
    }
}

glm::mat4 Anvil::get_proj_matrix() const
{
    if (!d_playingGame) { return d_editor_camera.Proj(); }

    const auto& reg = d_activeScene->registry;
    auto [tc, cc] = reg.get_all<spkt::Transform3DComponent, spkt::Camera3DComponent>(d_runtimeCamera);
    return spkt::make_proj(cc.fov);
}

glm::mat4 Anvil::get_view_matrix() const
{
    if (!d_playingGame) { return d_editor_camera.View(); }

    const auto& reg = d_activeScene->registry;
    auto [tc, cc] = reg.get_all<spkt::Transform3DComponent, spkt::Camera3DComponent>(d_runtimeCamera);
    return spkt::make_view(tc.position, tc.orientation, cc.pitch);
}

void Anvil::on_render()
{
    using namespace spkt::Maths;
    auto& registry = d_activeScene->registry;

    // If the size of the viewport has changed since the previous frame, recreate
    // the framebuffer.
    if (d_viewport_size != d_viewport.size() && d_viewport_size.x > 0 && d_viewport_size.y > 0) {
        d_viewport.resize(d_viewport_size.x, d_viewport_size.y);
    }

    d_viewport.bind();

    glm::mat4 proj = get_proj_matrix();
    glm::mat4 view = get_view_matrix();

    d_entity_renderer.Draw(registry, proj, view);
    d_skybox_renderer.Draw(d_skybox, proj, view);

    if (d_showColliders) {
        draw_colliders(d_geometry_renderer, registry, proj, view);
    }

    d_viewport.unbind();


    d_ui.StartFrame();

    ImGui::DockSpaceOverViewport();

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New")) {
                std::string file = spkt::SaveFile(d_window, "*.yaml");
                if (!file.empty()) {
                    spkt::log::info("Creating {}...", d_sceneFile);
                    d_sceneFile = file;
                    d_activeScene = d_scene = std::make_shared<spkt::scene>();
                    spkt::log::info("...done!");
                }
            }
            if (ImGui::MenuItem("Open")) {
                std::string file = spkt::OpenFile(d_window, "*.yaml");
                if (!file.empty()) {
                    spkt::log::info("Loading {}...", d_sceneFile);
                    d_sceneFile = file;
                    d_activeScene = d_scene = std::make_shared<spkt::scene>();
                    spkt::load_registry_from_file(file, d_scene->registry);
                    spkt::log::info("...done!");
                }
            }

            const auto entity_filter = [](const spkt::registry& reg, spkt::entity entity) {
                return !reg.has<spkt::Runtime>(entity);
            };

            if (ImGui::MenuItem("Save")) {
                spkt::log::info("Saving {}...", d_sceneFile);
                spkt::save_registry_to_file(d_sceneFile, d_scene->registry, entity_filter);
                spkt::log::info("...done!");
            }
            if (ImGui::MenuItem("Save As")) {
                std::string file = spkt::SaveFile(d_window, "*.yaml");
                if (!file.empty()) {
                    spkt::log::info("Saving as {}...", file);
                    d_sceneFile = file;
                    spkt::save_registry_to_file(file, d_scene->registry, entity_filter);
                    spkt::log::info("...done!");
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Scene")) {
            if (ImGui::MenuItem("Run")) {
                d_activeScene = std::make_shared<spkt::scene>();

                anvil::input_system_init(d_activeScene->registry, d_window);
                spkt::copy_registry(d_scene->registry, d_activeScene->registry);

                d_activeScene->systems = {
                    physics_system,
                    anvil::particle_system,
                    anvil::script_system,
                    anvil::animation_system,
                    anvil::delete_below_50_system,
                    anvil::clear_events_system,
                    anvil::input_system_end
                };

                d_activeScene->event_handlers = {
                    anvil::input_system_on_event
                };

                d_playingGame = true;
                d_runtimeCamera = d_activeScene->registry.find<spkt::Camera3DComponent>();
                d_window->set_cursor_visibility(false);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    // VIEWPORT
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{0, 0});
    if (ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
        d_is_viewport_hovered = ImGui::IsWindowHovered();
        d_is_viewport_focused = ImGui::IsWindowFocused();
        d_ui.BlockEvents(!d_is_viewport_focused || !d_is_viewport_hovered);

        ImVec2 size = ImGui::GetContentRegionAvail();
        d_viewport_size = glm::ivec2{size.x, size.y};

        //auto viewportMouse = ImGuiXtra::GetMousePosWindowCoords();

        spkt::ImGuiXtra::Image(d_viewport.colour_texture());

        if (!is_game_running() && registry.valid(d_selected) && registry.has<spkt::Transform3DComponent>(d_selected)) {
            auto& c = registry.get<spkt::Transform3DComponent>(d_selected);
            auto tr = spkt::Maths::Transform(c.position, c.orientation, c.scale);
            spkt::ImGuiXtra::Guizmo(&tr, view, proj, d_inspector.Operation(), d_inspector.Mode());
            spkt::Maths::Decompose(tr, &c.position, &c.orientation, &c.scale);
        }
        ImGui::End();
    }
    ImGui::PopStyleVar();

    // INSPECTOR
    static bool showInspector = true;
    if (ImGui::Begin("Inspector", &showInspector)) {
        d_inspector.Show(*this);
        ImGui::End();
    }

    // EXPLORER
    static std::string search;
    static bool showExplorer = true;
    if (ImGui::Begin("Explorer", &showExplorer)) {
        spkt::ImGuiXtra::TextModifiable(search);
        ImGui::SameLine();
        if (ImGui::Button("X")) {
            search = "";
        }
        if (ImGui::BeginTabBar("##Tabs")) {

            if (ImGui::BeginTabItem("Entities")) {
                ImGui::BeginChild("Entity List");
                int i = 0;
                for (auto entity : registry.all()) {
                    if (SubstringCI(entiy_name(registry, entity), search)) {
                        ImGui::PushID(i);
                        if (ImGui::Selectable(entiy_name(registry, entity).c_str())) {
                            d_selected = entity;
                        }
                        ImGui::PopID();
                    }
                    ++i;
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            const auto hasher = [](const std::string& str) {
                return static_cast<int>(std::hash<std::string>{}(str));
            };

            if (ImGui::BeginTabItem("Materials")) {
                ImGui::BeginChild("Material List");
                for (auto& [file, material] : d_asset_manager.view<spkt::material>()) {
                    ImGui::PushID(hasher(file));
                    if (ImGui::CollapsingHeader(material.name.c_str())) {
                        ImGui::Text(file.c_str());
                        ImGui::Separator();

                        ImGui::PushID(hasher("Albedo"));
                        ImGui::Text("Albedo");
                        ImGui::Checkbox("Use Map", &material.useAlbedoMap);
                        if (material.useAlbedoMap) {
                            material_ui(material.albedoMap);
                        } else {
                            ImGui::ColorEdit3("##Albedo", &material.albedo.x);
                        }
                        ImGui::PopID();
                        ImGui::Separator();

                        ImGui::PushID(hasher("Normal"));
                        ImGui::Text("Normal");
                        ImGui::Checkbox("Use Map", &material.useNormalMap);
                        if (material.useNormalMap) {
                            material_ui(material.normalMap);
                        }
                        ImGui::PopID();
                        ImGui::Separator();

                        ImGui::PushID(hasher("Metallic"));
                        ImGui::Text("Metallic");
                        ImGui::Checkbox("Use Map", &material.useMetallicMap);
                        if (material.useMetallicMap) {
                            material_ui(material.metallicMap);
                        } else {
                            ImGui::DragFloat("##Metallic", &material.metallic, 0.01f, 0.0f, 1.0f);
                        }
                        ImGui::PopID();
                        ImGui::Separator();

                        ImGui::PushID(hasher("Roughness"));
                        ImGui::Text("Roughness");
                        ImGui::Checkbox("Use Map", &material.useRoughnessMap);
                        if (material.useRoughnessMap) {
                            material_ui(material.roughnessMap);
                        } else {
                            ImGui::DragFloat("##Roughness", &material.roughness, 0.01f, 0.0f, 1.0f);
                        }
                        ImGui::PopID();
                        ImGui::Separator();

                        if (ImGui::Button("Save")) {
                            spkt::material::save(file, material);
                        }
                    }
                    ImGui::PopID();
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::End();
    }

    if (ImGui::Begin("Options")) {
        ImGui::Checkbox("Show Colliders", &d_showColliders);

        spkt::for_each_component([&]<typename T>(spkt::reflcomp<T>&& refl) {
            std::string text = std::format("# {}: {}", refl.name, registry.view<T>().size());
            ImGui::Text(text.c_str());
        });

        ImGui::End();
    }

    d_ui.EndFrame();
}

void Anvil::material_ui(std::string& texture)
{
    if (ImGui::Button("X")) {
        texture = "";
    }
    ImGui::SameLine();
    spkt::ImGuiXtra::File("File", d_window, &texture, "*.png");
}

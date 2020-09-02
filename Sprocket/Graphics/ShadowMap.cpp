#include "ShadowMap.h"
#include "Components.h"
#include "RenderContext.h"

#include <glad/glad.h>

namespace Sprocket {
namespace {

std::shared_ptr<InstanceBuffer> GetInstanceBuffer()
{
    BufferLayout layout(sizeof(InstanceData), 3);
    layout.AddAttribute(DataType::FLOAT, 3, DataRate::INSTANCE);
    layout.AddAttribute(DataType::FLOAT, 4, DataRate::INSTANCE);
    layout.AddAttribute(DataType::FLOAT, 3, DataRate::INSTANCE);
    layout.AddAttribute(DataType::FLOAT, 1, DataRate::INSTANCE);
    layout.AddAttribute(DataType::FLOAT, 1, DataRate::INSTANCE);
    assert(layout.Validate());

    return std::make_shared<InstanceBuffer>(layout);
}

}

ShadowMap::ShadowMap(Window* window, ModelManager* modelManager)
    : d_window(window)
    , d_modelManager(modelManager)
    , d_shader("Resources/Shaders/ShadowMap.vert", "Resources/Shaders/ShadowMap.frag")
    , d_lightViewMatrix() // Will be populated after starting a scene.
    , d_lightProjMatrix(Maths::Ortho(-25.0f, 25.0f, -25.0f, 25.0f, -20.0f, 20.0f))
    , d_shadowMap(window, 8192, 8192)
    , d_vao(std::make_unique<VertexArray>())
    , d_instanceBuffer(GetInstanceBuffer())
{
}

void ShadowMap::Draw(
    const Sun& sun,
    const Maths::vec3& centre,
    Scene& scene)
{
    RenderContext rc;
    rc.DepthTesting(true);
    rc.FaceCulling(true);

    d_lightViewMatrix = Maths::LookAt(centre - sun.direction, centre);

    d_shader.Bind();
    d_shader.LoadUniform("u_proj_matrix", d_lightProjMatrix);
    d_shader.LoadUniform("u_view_matrix", d_lightViewMatrix);

    d_shadowMap.Bind();
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
    glCullFace(GL_FRONT);

    std::string currentModel;
    scene.Each<TransformComponent, ModelComponent>([&](Entity& entity) {
        const auto& tc = entity.Get<TransformComponent>();
        const auto& mc = entity.Get<ModelComponent>();
        if (mc.model.empty()) { return; }

        if(mc.model != currentModel) {
            d_instanceBuffer->SetData(d_instanceData);
            d_vao->SetInstances(d_instanceBuffer);
            d_vao->Draw();
            d_instanceData.clear();

            d_vao->SetModel(d_modelManager->GetModel(mc.model));
            currentModel = mc.model;
        }

        d_instanceData.push_back({
            tc.position,
            tc.orientation,
            tc.scale,
            mc.shineDamper,
            mc.reflectivity
        });
    });

    d_instanceBuffer->SetData(d_instanceData);
    d_vao->SetInstances(d_instanceBuffer);
    d_vao->Draw();
    d_instanceData.clear();

    glCullFace(GL_BACK);
    d_shadowMap.Unbind();
    d_shader.Unbind();
}

Maths::mat4 ShadowMap::GetLightProjViewMatrix() const
{
    return d_lightProjMatrix * d_lightViewMatrix;
}

Texture ShadowMap::GetShadowMap() const
{
    return d_shadowMap.GetShadowMap();
}

}
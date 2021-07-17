#pragma once
#include <Sprocket/Graphics/AssetManager.h>
#include <Sprocket/Graphics/Buffer.h>
#include <Sprocket/Graphics/Shader.h>
#include <Sprocket/Graphics/ShadowMap.h>
#include <Sprocket/Graphics/VertexArray.h>
#include <Sprocket/Scene/ecs.h>

#include <memory>

namespace spkt {

class Scene3DRenderer
// Renders a scene as a 3D Scene. This makes use of components such as
// Transform3DComponent and ModelComponent, and will ignore 2D components
// such as Transform2DComponent and SpriteComponent.
{
public:
    // PBR Texture Slots
    static constexpr int ALBEDO_SLOT = 0;
    static constexpr int NORMAL_SLOT = 1;
    static constexpr int METALLIC_SLOT = 2;
    static constexpr int ROUGHNESS_SLOT = 3;

    // Shadow Map Texture Slot
    static constexpr int SHADOW_MAP_SLOT = 4;

    // Animation Data
    static constexpr int MAX_BONES = 50;

private:
    AssetManager*    d_assetManager;

    Shader d_staticShader;
    Shader d_animatedShader;

    std::unique_ptr<VertexArray> d_vao;
    
    std::unique_ptr<Buffer> d_instanceBuffer;

public:
    Scene3DRenderer(AssetManager* assetManager);

    void Draw(const spkt::registry& registry, spkt::entity camera);
    void Draw(const spkt::registry& registry, const glm::mat4& proj, const glm::mat4& view);

    void EnableShadows(const ShadowMap& shadowMap);

    Shader& GetShader() { return d_staticShader; }
};

}
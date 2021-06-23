#pragma once
#include "Maths.h"
#include <queue>
#include <string>
#include <utility>
#include <array>

#include "apecs.hpp"

namespace Sprocket {

// Components
struct TemporaryComponent
{
};

struct NameComponent
{
    std::string name = "Entity";
};

struct Transform2DComponent
{
    glm::vec2 position = {0.0f, 0.0f};
    float rotation = 0.0f;
    glm::vec2 scale = {1.0f, 1.0f};
};

struct Transform3DComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};

struct ModelComponent
{
    std::string mesh = "";
    std::string material = "";
};

struct RigidBody3DComponent
{
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    bool gravity = true;
    bool frozen = false;
    float bounciness = 0.5f;
    float frictionCoefficient = 0.3f;
    float rollingResistance = 0.0f;
    glm::vec3 force = {0.0f, 0.0f, 0.0f};
    bool onFloor = false;
};

struct BoxCollider3DComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    float mass = 1.0f;
    glm::vec3 halfExtents = {0.0f, 0.0f, 0.0f};
    bool applyScale = true;
};

struct SphereCollider3DComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    float mass = 1.0f;
    float radius = 1.0f;
};

struct CapsuleCollider3DComponent
{
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::quat orientation = {0.0f, 0.0f, 0.0f, 1.0f};
    float mass = 1.0f;
    float radius = 1.0f;
    float height = 1.0f;
};

struct ScriptComponent
{
    std::string script = "";
    bool active = true;
};

struct Camera3DComponent
{
    glm::mat4 projection = {};
    float fov = 70.0f;
    float pitch = 0.0f;
};

struct SelectComponent
{
    bool selected = false;
    bool hovered = false;
};

struct PathComponent
{
    std::queue<glm::vec3> markers = {};
    float speed = 0.0f;
};

struct GridComponent
{
    int x = 0;
    int z = 0;
};

struct LightComponent
{
    glm::vec3 colour = {1.0f, 1.0f, 1.0f};
    float brightness = 1.0f;
};

struct SunComponent
{
    glm::vec3 colour = {1.0f, 1.0f, 1.0f};
    float brightness = 1.0f;
    glm::vec3 direction = {0.0f, -1.0f, 0.0f};
    bool shadows = false;
};

struct AmbienceComponent
{
    glm::vec3 colour = {1.0f, 1.0f, 1.0f};
    float brightness = 1.0f;
};

struct ParticleComponent
{
    float interval = 1.0f;
    glm::vec3 velocity = {0.0f, 0.0f, 0.0f};
    float velocityNoise = 0.0f;
    glm::vec3 acceleration = {0.0f, -9.81f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    float life = 1.0f;
    float accumulator = 0.0f;
};

struct MeshAnimationComponent
{
    std::string name = "";
    float time = 0.0f;
    float speed = 1.0f;
};

struct Singleton
{
};

struct CollisionSingleton
{
    std::vector<std::pair<apx::entity, apx::entity>> collisions = {};
};

struct InputSingleton
{
    std::array<bool, 512> keyboard = {};
    std::array<bool, 5> mouse = {};
    std::array<bool, 5> mouse_click = {};
    std::array<bool, 5> mouse_unclick = {};
};


}

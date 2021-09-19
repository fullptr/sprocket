#include "Inspector.h"

#include <Anvil/Anvil.h>

#include <Sprocket/Scene/ecs.h>
#include <Sprocket/Scene/Loader.h>
#include <Sprocket/Scene/meta.h>
#include <Sprocket/UI/DevUI.h>
#include <Sprocket/UI/ImGuiXtra.h>
#include <Sprocket/Utility/Maths.h>

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

namespace {

template <typename T>
struct inspector_display;

template <>
struct inspector_display<spkt::Runtime>
{
    static void draw(Anvil& editor, spkt::Runtime& c)
    {
    }
};

template <>
struct inspector_display<spkt::Singleton>
{
    static void draw(Anvil& editor, spkt::Singleton& c)
    {
    }
};

template <>
struct inspector_display<spkt::Event>
{
    static void draw(Anvil& editor, spkt::Event& c)
    {
    }
};

template <>
struct inspector_display<spkt::NameComponent>
{
    static void draw(Anvil& editor, spkt::NameComponent& c)
    {
        spkt::ImGuiXtra::TextModifiable(c.name);
    }
};

template <>
struct inspector_display<spkt::Transform2DComponent>
{
    static void draw(Anvil& editor, spkt::Transform2DComponent& c)
    {
        ImGui::DragFloat2("Position", &c.position.x, 0.1f);
        ImGui::DragFloat("Rotation", &c.rotation, 0.01f);
        ImGui::DragFloat2("Scale", &c.scale.x, 0.1f);
    }
};

template <>
struct inspector_display<spkt::Transform3DComponent>
{
    static void draw(Anvil& editor, spkt::Transform3DComponent& c)
    {
        ImGui::DragFloat3("Position", &c.position.x, 0.1f);
        spkt::ImGuiXtra::Euler("Orientation", &c.orientation);
        ImGui::DragFloat3("Scale", &c.scale.x, 0.1f);
    }
};

template <>
struct inspector_display<spkt::StaticModelComponent>
{
    static void draw(Anvil& editor, spkt::StaticModelComponent& c)
    {
        spkt::ImGuiXtra::File("Mesh", editor.window(), &c.mesh, "*.obj");
        spkt::ImGuiXtra::File("Material", editor.window(), &c.material, "*.yaml");
    }
};

template <>
struct inspector_display<spkt::AnimatedModelComponent>
{
    static void draw(Anvil& editor, spkt::AnimatedModelComponent& c)
    {
        spkt::ImGuiXtra::File("Mesh", editor.window(), &c.mesh, "*.obj");
        spkt::ImGuiXtra::File("Material", editor.window(), &c.material, "*.yaml");
        spkt::ImGuiXtra::TextModifiable(c.animation_name);
        ImGui::DragFloat("Animation Time", &c.animation_time, 0.01f);
        ImGui::DragFloat("Animation Speed", &c.animation_speed, 0.01f);
    }
};

template <>
struct inspector_display<spkt::RigidBody3DComponent>
{
    static void draw(Anvil& editor, spkt::RigidBody3DComponent& c)
    {
        ImGui::DragFloat3("Velocity", &c.velocity.x, 0.1f);
        ImGui::Checkbox("Gravity", &c.gravity);
        ImGui::Checkbox("Frozen", &c.frozen);
        ImGui::SliderFloat("Bounciness", &c.bounciness, 0.0f, 1.0f);
        ImGui::SliderFloat("Friction Coefficient", &c.frictionCoefficient, 0.0f, 1.0f);
        ImGui::SliderFloat("Rolling Resistance", &c.rollingResistance, 0.0f, 1.0f);
        ImGui::DragFloat3("Force", &c.force.x, 0.1f);
        ImGui::Checkbox("OnFloor", &c.onFloor);
        ;
    }
};

template <>
struct inspector_display<spkt::BoxCollider3DComponent>
{
    static void draw(Anvil& editor, spkt::BoxCollider3DComponent& c)
    {
        ImGui::DragFloat3("Position", &c.position.x, 0.1f);
        spkt::ImGuiXtra::Euler("Orientation", &c.orientation);
        ImGui::DragFloat("Mass", &c.mass, 0.01f);
        ImGui::DragFloat3("Half Extents", &c.halfExtents.x, 0.1f);
        ImGui::Checkbox("Apply Scale", &c.applyScale);
        ;
    }
};

template <>
struct inspector_display<spkt::SphereCollider3DComponent>
{
    static void draw(Anvil& editor, spkt::SphereCollider3DComponent& c)
    {
        ImGui::DragFloat3("Position", &c.position.x, 0.1f);
        spkt::ImGuiXtra::Euler("Orientation", &c.orientation);
        ImGui::DragFloat("Mass", &c.mass, 0.01f);
        ImGui::DragFloat("Radius", &c.radius, 0.01f);
        ;
    }
};

template <>
struct inspector_display<spkt::CapsuleCollider3DComponent>
{
    static void draw(Anvil& editor, spkt::CapsuleCollider3DComponent& c)
    {
        ImGui::DragFloat3("Position", &c.position.x, 0.1f);
        spkt::ImGuiXtra::Euler("Orientation", &c.orientation);
        ImGui::DragFloat("Mass", &c.mass, 0.01f);
        ImGui::DragFloat("Radius", &c.radius, 0.01f);
        ImGui::DragFloat("Height", &c.height, 0.01f);
        ;
    }
};

template <>
struct inspector_display<spkt::ScriptComponent>
{
    static void draw(Anvil& editor, spkt::ScriptComponent& c)
    {
        spkt::ImGuiXtra::File("Script", editor.window(), &c.script, "*.lua");
        ImGui::Checkbox("Active", &c.active);
        ;
    }
};

template <>
struct inspector_display<spkt::Camera3DComponent>
{
    static void draw(Anvil& editor, spkt::Camera3DComponent& c)
    {
        ;
        ImGui::DragFloat("FOV", &c.fov, 0.01f);
        ImGui::DragFloat("Pitch", &c.pitch, 0.01f);
    }
};

template <>
struct inspector_display<spkt::PathComponent>
{
    static void draw(Anvil& editor, spkt::PathComponent& c)
    {
        ;
        ImGui::DragFloat("Speed", &c.speed, 0.01f);
    }
};

template <>
struct inspector_display<spkt::LightComponent>
{
    static void draw(Anvil& editor, spkt::LightComponent& c)
    {
        ImGui::ColorEdit3("Colour", &c.colour.r);
        ImGui::DragFloat("Brightness", &c.brightness, 0.01f);
    }
};

template <>
struct inspector_display<spkt::SunComponent>
{
    static void draw(Anvil& editor, spkt::SunComponent& c)
    {
        ImGui::ColorEdit3("Colour", &c.colour.r);
        ImGui::DragFloat("Brightness", &c.brightness, 0.01f);
        ImGui::DragFloat3("Direction", &c.direction.x, 0.1f);
        ImGui::Checkbox("Shadows", &c.shadows);
    }
};

template <>
struct inspector_display<spkt::AmbienceComponent>
{
    static void draw(Anvil& editor, spkt::AmbienceComponent& c)
    {
        ImGui::ColorEdit3("Colour", &c.colour.r);
        ImGui::DragFloat("Brightness", &c.brightness, 0.01f);
    }
};

template <>
struct inspector_display<spkt::ParticleComponent>
{
    static void draw(Anvil& editor, spkt::ParticleComponent& c)
    {
        ImGui::DragFloat("Interval", &c.interval, 0.01f);
        ImGui::DragFloat3("Velocity", &c.velocity.x, 0.1f);
        ImGui::DragFloat("Velocity Noise", &c.velocityNoise, 0.01f);
        ImGui::DragFloat3("Acceleration", &c.acceleration.x, 0.1f);
        ImGui::DragFloat3("Scale", &c.scale.x, 0.1f);
        ImGui::DragFloat("Life", &c.life, 0.01f);
        ImGui::DragFloat("Accumulator", &c.accumulator, 0.01f);
    }
};

template <>
struct inspector_display<spkt::CollisionEvent>
{
    static void draw(Anvil& editor, spkt::CollisionEvent& c)
    {
        ;
        ;
    }
};

template <>
struct inspector_display<spkt::PhysicsSingleton>
{
    static void draw(Anvil& editor, spkt::PhysicsSingleton& c)
    {
        ;
    }
};

template <>
struct inspector_display<spkt::InputSingleton>
{
    static void draw(Anvil& editor, spkt::InputSingleton& c)
    {
        ;
    }
};

template <>
struct inspector_display<spkt::GameGridSingleton>
{
    static void draw(Anvil& editor, spkt::GameGridSingleton& c)
    {
        ;
        ;
        ;
        ;
    }
};

template <>
struct inspector_display<spkt::TileMapSingleton>
{
    static void draw(Anvil& editor, spkt::TileMapSingleton& c)
    {
        ;
    }
};

template <>
struct inspector_display<spkt::CameraSingleton>
{
    static void draw(Anvil& editor, spkt::CameraSingleton& c)
    {
        ;
    }
};

template <>
struct inspector_display<spkt::ParticleSingleton>
{
    static void draw(Anvil& editor, spkt::ParticleSingleton& c)
    {
        ;
        ;
    }
};


}

void Inspector::Show(Anvil& editor)
{
    spkt::registry& registry = editor.active_scene()->registry;
    spkt::entity entity = editor.selected();

    if (!registry.valid(entity)) {
        if (ImGui::Button("New Entity")) {
            entity = registry.create();
            editor.set_selected(entity);
        }
        return;
    }
    int count = 0;

    ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1.0), "ID: %llu", entity);

    if (registry.has<spkt::Runtime>(entity)) {
        auto& c = registry.get<spkt::Runtime>(entity);
        if (ImGui::CollapsingHeader("Runtime")) {
            ImGui::PushID(count++);
            inspector_display<spkt::Runtime>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::Runtime, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::Runtime>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::Singleton>(entity)) {
        auto& c = registry.get<spkt::Singleton>(entity);
        if (ImGui::CollapsingHeader("Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::Singleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::Singleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::Singleton>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::Event>(entity)) {
        auto& c = registry.get<spkt::Event>(entity);
        if (ImGui::CollapsingHeader("Event")) {
            ImGui::PushID(count++);
            inspector_display<spkt::Event>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::Event, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::Event>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::NameComponent>(entity)) {
        auto& c = registry.get<spkt::NameComponent>(entity);
        if (ImGui::CollapsingHeader("Name")) {
            ImGui::PushID(count++);
            inspector_display<spkt::NameComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::NameComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::NameComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::Transform2DComponent>(entity)) {
        auto& c = registry.get<spkt::Transform2DComponent>(entity);
        if (ImGui::CollapsingHeader("Transform 2D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::Transform2DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::Transform2DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::Transform2DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::Transform3DComponent>(entity)) {
        auto& c = registry.get<spkt::Transform3DComponent>(entity);
        if (ImGui::CollapsingHeader("Transform 3D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::Transform3DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::Transform3DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::Transform3DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::StaticModelComponent>(entity)) {
        auto& c = registry.get<spkt::StaticModelComponent>(entity);
        if (ImGui::CollapsingHeader("Static Model")) {
            ImGui::PushID(count++);
            inspector_display<spkt::StaticModelComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::StaticModelComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::StaticModelComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::AnimatedModelComponent>(entity)) {
        auto& c = registry.get<spkt::AnimatedModelComponent>(entity);
        if (ImGui::CollapsingHeader("Animated Model")) {
            ImGui::PushID(count++);
            inspector_display<spkt::AnimatedModelComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::AnimatedModelComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::AnimatedModelComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::RigidBody3DComponent>(entity)) {
        auto& c = registry.get<spkt::RigidBody3DComponent>(entity);
        if (ImGui::CollapsingHeader("Rigid Body 3D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::RigidBody3DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::RigidBody3DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::RigidBody3DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::BoxCollider3DComponent>(entity)) {
        auto& c = registry.get<spkt::BoxCollider3DComponent>(entity);
        if (ImGui::CollapsingHeader("Box Collider 3D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::BoxCollider3DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::BoxCollider3DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::BoxCollider3DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::SphereCollider3DComponent>(entity)) {
        auto& c = registry.get<spkt::SphereCollider3DComponent>(entity);
        if (ImGui::CollapsingHeader("Sphere Collider 3D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::SphereCollider3DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::SphereCollider3DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::SphereCollider3DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::CapsuleCollider3DComponent>(entity)) {
        auto& c = registry.get<spkt::CapsuleCollider3DComponent>(entity);
        if (ImGui::CollapsingHeader("Capsule Collider 3D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::CapsuleCollider3DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::CapsuleCollider3DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::CapsuleCollider3DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::ScriptComponent>(entity)) {
        auto& c = registry.get<spkt::ScriptComponent>(entity);
        if (ImGui::CollapsingHeader("Script")) {
            ImGui::PushID(count++);
            inspector_display<spkt::ScriptComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::ScriptComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::ScriptComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::Camera3DComponent>(entity)) {
        auto& c = registry.get<spkt::Camera3DComponent>(entity);
        if (ImGui::CollapsingHeader("Camera 3D")) {
            ImGui::PushID(count++);
            inspector_display<spkt::Camera3DComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::Camera3DComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::Camera3DComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::PathComponent>(entity)) {
        auto& c = registry.get<spkt::PathComponent>(entity);
        if (ImGui::CollapsingHeader("Path")) {
            ImGui::PushID(count++);
            inspector_display<spkt::PathComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::PathComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::PathComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::LightComponent>(entity)) {
        auto& c = registry.get<spkt::LightComponent>(entity);
        if (ImGui::CollapsingHeader("Light")) {
            ImGui::PushID(count++);
            inspector_display<spkt::LightComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::LightComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::LightComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::SunComponent>(entity)) {
        auto& c = registry.get<spkt::SunComponent>(entity);
        if (ImGui::CollapsingHeader("Sun")) {
            ImGui::PushID(count++);
            inspector_display<spkt::SunComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::SunComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::SunComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::AmbienceComponent>(entity)) {
        auto& c = registry.get<spkt::AmbienceComponent>(entity);
        if (ImGui::CollapsingHeader("Ambience")) {
            ImGui::PushID(count++);
            inspector_display<spkt::AmbienceComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::AmbienceComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::AmbienceComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::ParticleComponent>(entity)) {
        auto& c = registry.get<spkt::ParticleComponent>(entity);
        if (ImGui::CollapsingHeader("Particle")) {
            ImGui::PushID(count++);
            inspector_display<spkt::ParticleComponent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::ParticleComponent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::ParticleComponent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::CollisionEvent>(entity)) {
        auto& c = registry.get<spkt::CollisionEvent>(entity);
        if (ImGui::CollapsingHeader("Collision Event")) {
            ImGui::PushID(count++);
            inspector_display<spkt::CollisionEvent>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::CollisionEvent, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::CollisionEvent>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::PhysicsSingleton>(entity)) {
        auto& c = registry.get<spkt::PhysicsSingleton>(entity);
        if (ImGui::CollapsingHeader("Physics Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::PhysicsSingleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::PhysicsSingleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::PhysicsSingleton>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::InputSingleton>(entity)) {
        auto& c = registry.get<spkt::InputSingleton>(entity);
        if (ImGui::CollapsingHeader("Input Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::InputSingleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::InputSingleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::InputSingleton>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::GameGridSingleton>(entity)) {
        auto& c = registry.get<spkt::GameGridSingleton>(entity);
        if (ImGui::CollapsingHeader("Game Grid Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::GameGridSingleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::GameGridSingleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::GameGridSingleton>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::TileMapSingleton>(entity)) {
        auto& c = registry.get<spkt::TileMapSingleton>(entity);
        if (ImGui::CollapsingHeader("Tile Map Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::TileMapSingleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::TileMapSingleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::TileMapSingleton>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::CameraSingleton>(entity)) {
        auto& c = registry.get<spkt::CameraSingleton>(entity);
        if (ImGui::CollapsingHeader("Camera Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::CameraSingleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::CameraSingleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::CameraSingleton>(entity); }
            ImGui::PopID();
        }
    }

    if (registry.has<spkt::ParticleSingleton>(entity)) {
        auto& c = registry.get<spkt::ParticleSingleton>(entity);
        if (ImGui::CollapsingHeader("Particle Singleton")) {
            ImGui::PushID(count++);
            inspector_display<spkt::ParticleSingleton>::draw(editor, c);
            if constexpr (std::is_same_v<spkt::ParticleSingleton, spkt::Transform3DComponent>) {
                spkt::ImGuiXtra::GuizmoSettings(d_operation, d_mode, d_useSnap, d_snap);
            }
            if (ImGui::Button("Delete")) { registry.remove<spkt::ParticleSingleton>(entity); }
            ImGui::PopID();
        }
    }

    ImGui::Separator();

    if (ImGui::Button("Add Component")) {
        ImGui::OpenPopup("missing_components_list");
    }

    if (ImGui::BeginPopup("missing_components_list")) {
        spkt::for_each_reflect([&]<typename T>(spkt::reflection<T> refl) {
            std::string comp_name{refl.component_name};
            if (!registry.has<T>(entity) && ImGui::Selectable(comp_name.c_str())) {
                registry.add<T>(entity, {});
            }
        });
        ImGui::EndMenu();
    }
    ImGui::Separator();

    ImGui::Separator();
    if (ImGui::Button("Duplicate")) {
        spkt::entity copy = spkt::copy_entity(editor.active_scene()->registry, entity);
        editor.set_selected(copy);
    }
    if (ImGui::Button("Delete Entity")) {
        registry.destroy(entity);
        editor.clear_selected();
    }
}

#include "Selector.h"
#include "MousePicker.h"
#include "MouseEvent.h"

namespace Sprocket {

Selector::Selector(
    Window* window,
    Camera* camera,
    Lens* lens,
    PhysicsEngine* physicsEngine
)
    : d_window(window)
    , d_camera(camera)
    , d_lens(lens)
    , d_physicsEngine(physicsEngine)
    , d_enabled(false)
    , d_hoveredEntity(nullptr)
    , d_selectedEntity(nullptr)
{
}

void Selector::updateSystem(float dt)
{
    if (!d_enabled) {
        return;
    }

    Maths::vec3 rayStart = Maths::inverse(d_camera->view()) * Maths::vec4(0, 0, 0, 1);
    Maths::vec3 direction = MousePicker::getRay(d_window, d_camera, d_lens);
    auto hitEntity = d_physicsEngine->raycast(rayStart, direction);
    if (hitEntity && hitEntity->has<SelectComponent>()) {
        d_hoveredEntity = hitEntity;
    }
    else {
        d_hoveredEntity = nullptr;
    }
}

void Selector::updateEntity(float dt, Entity& entity)
{
    auto& selectData = entity.get<SelectComponent>();

    if (!d_enabled) {
        selectData.hovered = false;
        selectData.selected = false;
        return;
    }

    if (!entity.has<SelectComponent>()) {
        return;
    }

    if (d_hoveredEntity == &entity) {
        selectData.hovered = true;
    }
    else {
        selectData.hovered = false;
    }
}

bool Selector::handleEvent(const Event& event)
{
    if (!d_enabled) {
        return false;
    }

    if (auto e = event.as<MouseButtonPressedEvent>()) {
        if (d_selectedEntity != nullptr) {
            // Deselect the old entity.
            d_selectedEntity->get<SelectComponent>().selected = false;
        }

        d_selectedEntity = d_hoveredEntity;
        if (d_selectedEntity != nullptr) {
            d_selectedEntity->get<SelectComponent>().selected = true;
        }

        return true;
    }
    return false;
}

void Selector::clear()
{
    d_hoveredEntity = nullptr;
    d_selectedEntity = nullptr;
}

void Selector::deregisterEntity(const Entity& entity)
{
    clear();
}

}
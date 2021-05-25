#include "GameGrid.h"
#include "Camera.h"
#include "Components.h"
#include "Scene.h"
#include "MouseCodes.h"
#include "Log.h"

#include <random>
#include <cassert>

namespace Sprocket {

std::string Name(const spkt::entity& e) {
    if (e.has<NameComponent>()) {
        return e.get<NameComponent>().name;
    }
    return "Entity";
}

GameGrid::GameGrid(Window* window)
    : d_window(window)
    , d_hovered({0.0, 0.0})
    , d_selected({})
{
}

void GameGrid::on_startup(spkt::registry& registry, ev::Dispatcher& dispatcher)
{
    std::string gridSquare = "Resources/Models/Square.obj";

    d_hoveredSquare = apx::create_from(registry);
    auto& n1 = d_hoveredSquare.emplace<NameComponent>();
    d_hoveredSquare.emplace<TemporaryComponent>();
    n1.name = "Hovered Grid Highlighter";
    auto& tr1 = d_hoveredSquare.emplace<Transform3DComponent>();
    tr1.scale = {0.3f, 0.3f, 0.3f};
    auto& model1 = d_hoveredSquare.emplace<ModelComponent>();
    model1.mesh = gridSquare;

    d_selectedSquare = apx::create_from(registry);
    auto& n2 = d_selectedSquare.emplace<NameComponent>();
    d_selectedSquare.emplace<TemporaryComponent>();
    n2.name = "Selected Grid Highlighter";
    auto& tr2 = d_selectedSquare.emplace<Transform3DComponent>();
    tr2.scale = {0.5f, 0.5f, 0.5f};
    auto& model2 = d_selectedSquare.emplace<ModelComponent>();
    model2.mesh = gridSquare;

    dispatcher.subscribe<spkt::added<GridComponent>>([&](ev::Event& event, auto&& data) {
        spkt::entity entity = data.entity;
        auto& transform = entity.get<Transform3DComponent>();
        const auto& gc = entity.get<GridComponent>();

        assert(!d_gridEntities.contains({gc.x, gc.z}));
    
        transform.position.x = gc.x + 0.5f;
        transform.position.z = gc.z + 0.5f;
        d_gridEntities[{gc.x, gc.z}] = data.entity;
    });

    dispatcher.subscribe<spkt::removed<GridComponent>>([&](ev::Event& eventm, auto&& data) {
        auto& gc = data.entity.get<GridComponent>();

        auto it = d_gridEntities.find({gc.x, gc.z});
        if (it == d_gridEntities.end()) {
            log::warn("No entity exists at this coord!");
        }
        else {
            d_gridEntities.erase(it);
        }
    });

    dispatcher.subscribe<ev::MouseButtonPressed>([&](ev::Event& event, auto&& data) {
        if (data.button == Mouse::LEFT) {
            d_selected = d_hovered;
        } else {
            d_selected = std::nullopt;
        }
    });
}

void GameGrid::on_update(spkt::registry&, const ev::Dispatcher&, double dt)
{
    auto& camTr = d_camera.get<Transform3DComponent>();

    glm::vec3 cameraPos = camTr.position;
    glm::vec3 direction = Maths::GetMouseRay(
        d_window->GetMousePos(),
        d_window->Width(),
        d_window->Height(),
        MakeView(d_camera),
        MakeProj(d_camera)
    );

    float lambda = -cameraPos.y / direction.y;
    glm::vec3 mousePos = cameraPos + lambda * direction;
    d_hovered = {(int)std::floor(mousePos.x), (int)std::floor(mousePos.z)};

    d_hoveredSquare.get<Transform3DComponent>().position = { d_hovered.x + 0.5f, 0.05f, d_hovered.y + 0.5f };
    if (d_selected.has_value()) {
        d_selectedSquare.get<Transform3DComponent>().position = { d_selected.value().x + 0.5f, 0.05f, d_selected.value().y + 0.5f };
    } else {
        d_selectedSquare.get<Transform3DComponent>().position = { 0.5f, -1.0f, 0.5f };
    }
}

spkt::entity GameGrid::At(const glm::ivec2& pos) const
{
    auto it = d_gridEntities.find(pos);
    if (it != d_gridEntities.end()) {
        return it->second;
    }
    return spkt::null;
}

spkt::entity GameGrid::Hovered() const
{
    return At(d_hovered);
}

spkt::entity GameGrid::Selected() const
{
    if (d_selected.has_value()) {
        return At(d_selected.value());
    }
    return spkt::null;
}

}
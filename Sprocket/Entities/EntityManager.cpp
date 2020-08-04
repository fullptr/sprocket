#include "EntityManager.h"

namespace Sprocket {

EntityManager::EntityManager(const std::vector<EntitySystem*> systems)
    : d_systems(systems)
    , d_registry()
{
}

Entity EntityManager::NewEntity()
{
    auto e = d_registry.create();
    return Entity(&d_registry, e);
}

void EntityManager::OnStartup()
{
    for (auto& system : d_systems) {
        system->OnStartup(*this);
    }
}

void EntityManager::OnUpdate(double dt, bool active)
{
    for (auto system : d_systems) {
        system->OnUpdate(*this, dt, active);
    }
}

void EntityManager::OnEvent(Event& event)
{
    for (auto system : d_systems) {
        system->OnEvent(event);
    }
}

std::size_t EntityManager::Size() const
{
    return d_registry.alive();
}

}
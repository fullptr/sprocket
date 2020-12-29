#include "Scene.h"
#include "Components.h"
#include "Log.h"

namespace Sprocket {

void Scene::OnUpdate(double dt)
{
    for (auto& system : d_systems) {
        system->OnUpdate(*this, dt);
    }
}

void Scene::OnEvent(Event& event)
{
    for (auto& system : d_systems) {
        system->OnEvent(*this, event);
    }
}

std::size_t Scene::Size() const
{
    return d_registry.Size();
}

void Scene::Clear()
{
    d_registry.Clear();
    d_lookup.clear();
    d_systems.clear();
}

}

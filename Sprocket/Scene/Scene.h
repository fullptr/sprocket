#pragma once
#include "ECS.h"
#include "Components.h"
#include "Events.h"
#include "Window.h"

#include <memory>
#include <vector>
#include <string_view>
#include <functional>

namespace spkt {

template <typename Comp>
Comp& get_singleton(spkt::registry& registry)
{
    auto singleton = registry.find<Singleton>();
    assert(registry.valid(singleton));
    assert(registry.has<Comp>(singleton));
    return registry.get<Comp>(singleton);
}

class Scene
{
public:
    using system_t = std::function<void(spkt::registry&, double)>;

private:
    // Temporary, will be removed when then the InputSingleton gets updated via a system.
    Window* d_window;

    spkt::registry d_registry;
    std::vector<system_t> d_systems;

public:
    Scene(Window* window);
    ~Scene();

    spkt::registry& Entities() { return d_registry; }

    void add(const system_t& system);

    void Load(std::string_view file);

    void on_update(double dt);
    void on_event(ev::Event& event);

    std::size_t Size() const;

    template <typename... Comps>
    spkt::entity find(const std::function<bool(spkt::entity)>& function = [](spkt::entity) { return true; });

    template <typename... Comps>
    apx::generator<spkt::entity> view();
};

template <typename... Comps>
spkt::entity Scene::find(const std::function<bool(spkt::entity)>& function)
{
    for (auto entity : view<Comps...>()) {
        if (function(entity)) { return entity; }
    }
    return spkt::null;
}

template <typename... Comps>
apx::generator<spkt::entity> Scene::view()
{
    if constexpr (sizeof...(Comps) > 0) {
        for (auto id : d_registry.view<Comps...>()) {
            co_yield {d_registry, id};
        }
    } else {
        for (auto id : d_registry.all()) {
            co_yield {d_registry, id};
        }
    }
}

}

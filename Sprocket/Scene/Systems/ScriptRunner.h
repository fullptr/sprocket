#pragma once
#include "LuaEngine.h"
#include "EntitySystem.h"
#include "InputProxy.h"
#include "Window.h"

#include <unordered_map>

namespace Sprocket {

class ScriptRunner : public EntitySystem
{
    Window*    d_window;
    InputProxy d_input;

    std::unordered_map<ecs::Entity, std::pair<lua::LuaEngine, bool>> d_engines;

public:
    ScriptRunner(Window* window);

    void OnStartup(Scene& scene) override;
    void OnUpdate(Scene& scene, double dt) override;
    void OnEvent(Scene& scene, ev::Event& event) override;
};

}
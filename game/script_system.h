#pragma once
#include <game/ecs/lua_ecs.h>
#include <game/ecs/scene.h>

#include <sprocket/scripting/lua_input.h>
#include <sprocket/scripting/lua_maths.h>
#include <sprocket/scripting/lua_script.h>
#include <sprocket/utility/input_store.h>

#include <functional>
#include <memory>
#include <utility>
#include <vector>

inline void script_system(game::registry& registry, double dt)
{
    static constexpr const char* INIT_FUNCTION = "init";
    static constexpr const char* UPDATE_FUNCTION = "on_update";
    
    std::vector<std::function<void()>> commands;

    for (auto entity : registry.view<game::ScriptComponent>()) {
        auto& sc = registry.get<game::ScriptComponent>(entity);
        if (!sc.active) { continue; }

        if (!sc.script_runtime) {
            auto input_singleton = registry.find<game::InputSingleton>();
            auto& input = *registry.get<game::InputSingleton>(input_singleton).input_store;
            
            sc.script_runtime = std::make_shared<spkt::lua::script>(sc.script);
            spkt::lua::script& script = *sc.script_runtime;

            spkt::lua::load_maths(script);
            spkt::lua::load_input_store(script, input);
            game::load_registry(script, registry);

            if (script.has_function(INIT_FUNCTION)) {
                script.set_value("__command_list__", &commands);
                script.call_function<void>(INIT_FUNCTION, entity);
            }
        }
        else {
            spkt::lua::script& script = *sc.script_runtime;
            script.set_value("__command_list__", &commands);
            script.call_function<void>(UPDATE_FUNCTION, entity, dt);
        }
    }

    for (auto& command : commands) {
        command();
    }
}
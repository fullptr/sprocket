#include "ScriptRunner.h"
#include "Log.h"
#include "ECS.h"
#include "Scene.h"
#include "LuaScript.h"
#include "LuaLibrary.h"
#include "Components.h"

#include <utility>

namespace Sprocket {

ScriptRunner::ScriptRunner(Window* window)
    : d_window(window)
{
}

void ScriptRunner::OnStartup(Scene& scene)
{
    scene.Entities().OnAdd<ScriptComponent>([&](ecs::Entity entity) {
        lua::Script script(entity.Get<ScriptComponent>().script);
        lua::register_scene_functions(script, scene);
        lua::register_input_functions(script, d_input);
        lua::register_window_functions(script, *d_window);

        lua::register_entity_transformation_functions(script);
        lua::register_entity_component_functions(script);

        script.call_function<void>("Init", entity);
        script.print_globals();
        d_engines.emplace(entity, std::make_pair(std::move(script), true));
    });

    scene.Entities().OnRemove<ScriptComponent>([&](ecs::Entity entity) {
        auto it = d_engines.find(entity);
        if (it != d_engines.end()) {
            it->second.second = false; // alive = false
        }
    });
}

void ScriptRunner::OnUpdate(Scene& scene, double dt)
{
    // We delete scripts here rather then with OnRemove otherwise we would segfault if
    // a script tries to delete its own entity, which is functionality that we want to
    // support.
    for (auto it = d_engines.begin(); it != d_engines.end();) {
        auto& entity = it->first;
        auto& [script, alive] = it->second;

        if (alive) {
            if (entity.Get<ScriptComponent>().active) {
                script.call_function<void>("OnUpdate", entity, dt);
            }
            ++it;
        } else {
            it = d_engines.erase(it);
        }
    }
}

void ScriptRunner::OnEvent(Scene& scene, ev::Event& event)
{
    d_input.on_event(event);

    const auto handler = [&](lua::Script& script, const char* f, auto&&... args)
    {
        if (script.has_function(f) &&
            script.call_function<bool>(f, std::forward<decltype(args)>(args)...))
        {
            event.consume();
        }
    };

    for (auto& [entity, pair] : d_engines) {
        auto& [script, alive] = pair;
        if (!(alive && entity.Get<ScriptComponent>().active)) {
            continue;
        }

        if (auto x = event.get_if<ev::WindowResize>()) {
            handler(script, "OnWindowResizeEvent", x->width, x->height);
        }
        else if (auto x = event.get_if<ev::WindowGotFocus>()) {
            handler(script, "OnWindowGotFocusEvent");
        }
        else if (auto x = event.get_if<ev::WindowLostFocus>()) {
            handler(script, "OnWindowLostFocusEvent");
        }
        else if (auto x = event.get_if<ev::WindowMaximize>()) {
            handler(script, "OnWindowMaximizeEvent");
        }
        else if (auto x = event.get_if<ev::WindowMinimize>()) {
            handler(script, "OnWindowMinimizeEvent");
        }
        else if (auto x = event.get_if<ev::WindowClosed>()) {
            // pass
        }
        else if (auto x = event.get_if<ev::MouseButtonPressed>()) {
            handler(script, "OnMouseButtonPressedEvent", x->button, x->action, x->mods);
        }
        else if (auto x = event.get_if<ev::MouseButtonReleased>()) {
            handler(script, "OnMouseButtonReleasedEvent", x->button, x->action, x->mods);
        }
        else if (auto x = event.get_if<ev::MouseMoved>()) {
            handler(script, "OnMouseMovedEvent", x->x_pos, x->y_pos);
        }
        else if (auto x = event.get_if<ev::MouseScrolled>()) {
            handler(script, "OnMouseScrolledEvent", x->x_offset, x->y_offset);
        }
        else if (auto x = event.get_if<ev::KeyboardButtonPressed>()) {
            handler(script, "OnKeyboardButtonPressedEvent", x->key, x->scancode, x->mods);
        }
        else if (auto x = event.get_if<ev::KeyboardButtonReleased>()) {
            handler(script, "OnKeyboardButtonReleasedEvent", x->key, x->scancode, x->mods);
        }
        else if (auto x = event.get_if<ev::KeyboardButtonHeld>()) {
            handler(script, "OnKeyboardButtonHeldEvent", x->key, x->scancode, x->mods);
        }
        else if (auto x = event.get_if<ev::KeyboardTyped>()) {
            handler(script, "OnKeyboardKeyTypedEvent", x->key);
        }
        else {
            log::warn("Event with unknown type {}", event.type_name());
            return;
        }
    }
}

}
#pragma once
#include <Sprocket/Core/window.h>
#include <Sprocket/Scene/ecs.h>

namespace spkt {

class event;

// This is a special system which is designed to get input information (keyboard,
// mouse and window) into the ECS for systems to make use of. As such, it requires
// a function that hooks into the event loop. Also, because of frame sensitive information
// such as mouse offsets, this requires two "system" functions, a begin and end.

void input_system_init(spkt::registry& regsitry, spkt::window* window);

// Processes events to build up the InputSingleton for the current frame.
void input_system_on_event(spkt::registry& registry, spkt::event& event);

// Resets are per-frame information such as how much the mouse was scrolled or which
// button were pressed this frame. This should be the last system added to any scene.
void input_system_end(spkt::registry& registry, double dt);

}
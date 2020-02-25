#include "Utility/Keyboard.h"
#include "Utility/Log.h"
#include "Events/Event.h"
#include "Events/KeyboardEvent.h"

namespace Sprocket {
namespace Keyboard {

namespace {

bool s_initialised = false;

std::unordered_map<int, bool> s_pressedState;

void handleEvent(const Event& event)
{
    if (auto e = event.as<KeyboardButtonPressedEvent>()) {
        s_pressedState[e->key()] = true;
        SPKT_LOG_INFO("Key {} pressed", e->key());
    }
    else if (auto e = event.as<KeyboardButtonReleasedEvent>()) {
        s_pressedState[e->key()] = false;
        SPKT_LOG_INFO("Key {} released", e->key());
    }
}

}

void init(Window* window)
{
    window->registerKeyboardCallback([&](const Event& event) {
        handleEvent(event);
    });
    SPKT_LOG_INFO("Initialised Keyboard");
    s_initialised = true;
}

bool isKeyDown(int key)
{
    if (!s_initialised) {
        SPKT_LOG_ERROR("Keyboard not initalised! Tried to check key {}", key);
        return false;
    }

    auto it = s_pressedState.find(key);
    if (it == s_pressedState.end()) {
        s_pressedState[key] = false;
        return false;
    }

    return it->second;
}


}
}
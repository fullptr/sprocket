#pragma once
#include "Core/Window.h"
#include "Utility/Maths.h"

#include <utility>

namespace Sprocket {
namespace Mouse {

void init(Window* window);
    // Initialises the Keyboard module by reigstering a callback with the
    // window. This is how the Keyboard module will get updated.

Maths::vec2 getMousePos();
    // Returns the (x, y) of where the mouse is currently on the screen.

bool isButtonDown(int button);
    // Returns true if the specified button is pressed and false otherwise.

enum Button {
    LEFT = 0,
    RIGHT = 1,
    MIDDLE = 2
};

}
}
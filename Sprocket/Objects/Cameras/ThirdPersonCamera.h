#pragma once
#include "Window.h"
#include "Maths.h"
#include "Camera.h"
#include "KeyboardProxy.h"

namespace Sprocket {

class ThirdPersonCamera : public Camera
{
    Maths::vec3 d_position;
        // The position of the camera in the world.

    float d_absoluteVerticalPosition;
        // This gets updated by scrolling, and the d_position
        // smoothly transitions towards this.

    Maths::vec3 d_target;
        // The position in the world that the camera is looking at.

    float d_distance;
        // The distance from the target;

    float d_horiz;
        // Variable to parameterise horizonal movement.

    float d_rotationSpeed;
        // Speed the camera can rotate at.

    float d_movementSpeed;
        // Speed the camera can move around at.

    KeyboardProxy d_keyboard;

public:
    ThirdPersonCamera();

    Maths::mat4 view() const override;
    void update(float timeDelta) override;
    void handleEvent(Event& event) override;

    Maths::vec3 target() const { return d_target; }
};

}
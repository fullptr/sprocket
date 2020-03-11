#include "FirstPersonCamera.h"
#include "WindowEvent.h"

#include <cmath>
#include <algorithm>

namespace Sprocket {

FirstPersonCamera::FirstPersonCamera(float aspectRatio)
    : Camera()
    , d_sensitivity(0.15f)
    , d_fov(70.0f)
    , d_nearPlane(0.1f)
    , d_farPlane(1000.0f)
    , d_projectionMatrix(Maths::createProjectionMatrix(aspectRatio, d_fov, d_nearPlane, d_farPlane))
{
}

void FirstPersonCamera::update(Window* window, float timeDelta)
{
    float speed = 10.0f * timeDelta;

    Maths::vec3 forwards = d_direction;
    forwards.y = 0;
    Maths::normalise(forwards);

    Maths::vec3 up = {0.0f, 1.0f, 0.0f};
    Maths::vec3 right = cross(forwards, up);
    
    if (window->isKeyDown(Keyboard::W)){
        d_position += speed * forwards;
    }
    if (window->isKeyDown(Keyboard::S)){
        d_position -= speed * forwards;
    }
    if (window->isKeyDown(Keyboard::D)){
        d_position += speed * right;
    }
    if (window->isKeyDown(Keyboard::A)){
        d_position -= speed * right;
    }
    if (window->isKeyDown(Keyboard::SPACE)){
        d_position += speed * up;
    }
    if (window->isKeyDown(Keyboard::LSHIFT)){
        d_position -= speed * up;
    }
#if 0
    if (window->isKeyDown(Keyboard::R)) {
        d_fov += 5.0f;
        Maths::clamp(d_fov, 60, 120);
    }
    if (window->isKeyDown(Keyboard::F)) {
        d_fov -= 5.0f;
    }
#endif

    Maths::clamp(d_fov, 60, 120);

    Maths::vec2 offset = window->getMouseOffset();
    d_yaw += d_sensitivity * offset.x;
    d_pitch += d_sensitivity * offset.y;
    Maths::clamp(d_pitch, -89.0, 89.0);

    d_direction.x = Maths::sind(d_yaw) * Maths::cosd(d_pitch);
    d_direction.y = -Maths::sind(d_pitch);
    d_direction.z = -Maths::cosd(d_yaw) * Maths::cosd(d_pitch);
    Maths::normalise(d_direction);
}

void FirstPersonCamera::handleEvent(Window* window, const Event& event)
{
    if (auto e = event.as<WindowResizeEvent>()) {
        d_projectionMatrix = Maths::createProjectionMatrix(
            window->aspectRatio(),
            d_fov,
            d_nearPlane,
            d_farPlane
        );
    }
}

}
#pragma once
#include <anvil/ecs/ecs.h>

#include <sprocket/graphics/renderers/geometry_renderer.h>
#include <sprocket/graphics/renderers/pbr_renderer.h>

#include <glm/glm.hpp>

namespace anvil {

void draw_colliders(
    const spkt::geometry_renderer& renderer,
    const anvil::registry& registry,
    const glm::mat4& proj,
    const glm::mat4& view
);

void draw_scene(
    spkt::pbr_renderer& renderer,
    const anvil::registry& registry,
    const glm::mat4& proj,
    const glm::mat4& view
);

}
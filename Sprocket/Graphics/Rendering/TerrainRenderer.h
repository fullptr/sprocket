#pragma once
#include "Window.h"
#include "Terrain.h"
#include "Light.h"
#include "Camera.h"
#include "Lens.h"
#include "Shader.h"
#include "RenderOptions.h"

namespace Sprocket {

class TerrainRenderer
{
    const unsigned int MAX_NUM_LIGHTS = 5;
    
    Window* d_window;
        // Non-owning pointer to the window to draw in.

    Shader d_shader;
        // Shader used to draw entities.

public:
    TerrainRenderer(Window* window);

    void update(const Camera& camera,
                const Lens& lens,
                const Lights& lights);

    void draw(const Terrain& terrain);
};

}
#include "Log.h"
#include "Window.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Loader.h"
#include "Renderer.h"
#include "Model.h"
#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "Light.h"
#include "Layer.h"
#include "Scene.h"
#include "SceneData.h"
#include "LayerStack.h"
#include "Events/KeyboardEvent.h"

#include <vector>
#include <memory>

namespace Sprocket {

class GameLayer : public Layer
{
    Loader   d_loader;
    Renderer d_renderer;
    Camera   d_camera;
    Shader   d_shader;

    std::vector<Entity> d_entities;
    std::vector<Light>  d_lights;

public:
    GameLayer(Window* window) 
        : Layer(Status::NORMAL, false) 
        , d_loader()
        , d_renderer()
        , d_camera()
        , d_shader("Resources/Shaders/Basic.vert",
                   "Resources/Shaders/Basic.frag")
        , d_entities()
        , d_lights()
    {
        d_shader.loadProjectionMatrix(window->aspectRatio(), 70.0f, 0.1f, 1000.0f);
        
        Model quadModel = d_loader.loadModel("Resources/Models/Plane.obj");
        Model dragonModel = d_loader.loadModel("Resources/Models/Dragon.obj");

        Texture space = d_loader.loadTexture("Resources/Textures/PlainGray.PNG");
        Texture gray = d_loader.loadTexture("Resources/Textures/PlainGray.PNG");
        gray.reflectivity(3);
        gray.shineDamper(5);
        //space.reflectivity(3);
        //space.shineDamper(5);

        d_entities.push_back(Entity(dragonModel, gray, {0.0f, 0.0f, -1.0f}, Maths::vec3(0.0f), 0.1f));
        d_entities.push_back(Entity(quadModel, space, {0.0f, -1.0f, 0.0f}, Maths::vec3(0.0f), 20));
    
        d_lights.push_back(Light{{0.0f, 50.0f, 0.0f}, {0.5f, 0.4f, 0.4f}, {1.0f, 0.0f, 0.0f}});
        d_lights.push_back(Light{{5.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.3f, 0.0f}});
        d_lights.push_back(Light{{-5.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.3f, 0.0f}});
        d_lights.push_back(Light{{8.0f, 4.0f, 2.0f}, {0.3f, 0.8f, 0.2f}, {1.0f, 0.3f, 0.0f}});
    }

    bool handleEvent(const Event& event) override
    {
        if (auto e = event.as<KeyboardButtonPressedEvent>()) {
            if (e->key() == Keyboard::ESC) {
                if (d_status == Layer::Status::NORMAL) {
                    d_status = Layer::Status::PAUSED;
                }
                else if (d_status == Layer::Status::PAUSED) {
                    d_status = Layer::Status::NORMAL;
                }
            }
        }
        return false;
    }

    void update(SceneData* data) override
    {
        d_status = data->paused ? Status::PAUSED : Status::NORMAL;

        if (d_status == Status::NORMAL) {
            float tick = layerTicker();

            d_lights[1].position.z = 5 * std::sin(tick);
            d_lights[1].position.x = 5 * std::cos(tick);

            d_lights[2].position.z = 6 * std::sin(-1.5f * tick);
            d_lights[2].position.x = 6 * std::cos(-1.5f * tick);

            d_lights[3].position.z = 6 * std::sin(8.0f * tick);
            d_lights[3].position.x = 6 * std::cos(8.0f * tick);
        }

        d_camera.move(d_status == Status::NORMAL);
    }

    void draw() override
    {
        for (const auto& entity: d_entities) {
            d_renderer.render(entity, d_lights, d_camera, d_shader);
        }
    }
};

class UILayer : public Layer
{
public:
    UILayer() : Layer(Status::INACTIVE, true) {}

    bool handleEvent(const Event& event) override
    {
        return false;
    }

    void update(SceneData* data) override
    {
        d_status = data->paused ? Status::NORMAL : Status::INACTIVE;
    }

    void draw() override
    {
    }
};

}

int main(int argc, char* argv[])
{
    Sprocket::Log::init();
    SPKT_LOG_INFO("Version {}.{}.{}", 0, 0, 1);

    Sprocket::Window window;
    Sprocket::Keyboard::init(&window);
    Sprocket::Mouse::init(&window);

    Sprocket::LayerStack layerStack;
    layerStack.pushLayer(std::make_shared<Sprocket::GameLayer>(&window));
    layerStack.pushLayer(std::make_shared<Sprocket::UILayer>());

    Sprocket::SceneData sceneData;
    sceneData.name = "Sprocket";
    sceneData.window = &window;
    sceneData.type = Sprocket::SceneType::STAGE;
    sceneData.paused = false;

    Sprocket::Scene scene(sceneData, layerStack,
        [](const Sprocket::Event& event, Sprocket::SceneData* data){
            if (auto e = event.as<Sprocket::KeyboardButtonPressedEvent>()) {
                if (e->key() == Sprocket::Keyboard::ESC) {
                    data->paused = !data->paused;
                    data->window->setCursorVisibility(data->paused);
                }
            }
        });

    while (window.running()) {
        window.clear();

        scene.tick();

        window.onUpdate();
    }

    return 0;
}
#include "WorldLayer.h"

WorldLayer::WorldLayer(const Sprocket::CoreSystems& core) 
    : Sprocket::Layer(core, Status::NORMAL, false)
    , d_mode(Mode::OBSERVER)
    , d_entityRenderer(core.window)
    , d_terrainRenderer(core.window)
    , d_skyboxRenderer(core.window)
    , d_postProcessor(core.window->width(), core.window->height())
    , d_lens(core.window->aspectRatio())
    , d_camera(&d_observerCamera)
    , d_skybox({
        Sprocket::ModelManager::loadModel("Resources/Models/Skybox.obj"),
        Sprocket::CubeMap({
            "Resources/Textures/Skybox/Skybox_X_Pos.png",
            "Resources/Textures/Skybox/Skybox_X_Neg.png",
            "Resources/Textures/Skybox/Skybox_Y_Pos.png",
            "Resources/Textures/Skybox/Skybox_Y_Neg.png",
            "Resources/Textures/Skybox/Skybox_Z_Pos.png",
            "Resources/Textures/Skybox/Skybox_Z_Neg.png"
        })
    })
    , d_playerCamera(nullptr)
    , d_physicsEngine(Sprocket::Maths::vec3(0.0, -9.81, 0.0))
    , d_playerMovement(core.window)
    , d_selector(core.window, &d_editorCamera, &d_lens, &d_physicsEngine)
    , d_entityManager({&d_playerMovement, &d_physicsEngine, &d_selector})
    , d_observerCamera(core.window)
{
    using namespace Sprocket;

    d_playerMovement.enable(false);

    auto& entityManager = d_entityManager;

    Texture green("Resources/Textures/Green.PNG");
    Texture space("Resources/Textures/Space.PNG");
    Texture spaceSpec("Resources/Textures/SpaceSpec.PNG");
    Texture gray("Resources/Textures/PlainGray.PNG");

    Material dullGray;
    dullGray.texture = gray;

    Material shinyGray;
    shinyGray.texture = gray;
    shinyGray.reflectivity = 2.0f;
    shinyGray.shineDamper = 3.0f;

    Material field;
    field.texture = green;

    Material galaxy;
    galaxy.texture = space;
    galaxy.specularMap = spaceSpec;

    Material islandMaterial;
    islandMaterial.texture = Texture("Resources/Textures/FloatingIslandTex.png");

    auto platformModel = core.modelManager->loadModel("Platform", "Resources/Models/Platform.obj");
    auto crateModel = core.modelManager->loadModel("Crate", "Resources/Models/Cube.obj");
    auto sphereModel = core.modelManager->loadModel("Sphere", "Resources/Models/Sphere.obj");
    auto floatingIslandModel = core.modelManager->loadModel("Floating Island", "Resources/Models/FloatingIsland.obj");

    {
        auto platform = std::make_shared<Entity>();
        platform->name() = "Platform 1";
        platform->position() = {7.0, 0.0, -3.0};
        platform->orientation() = Maths::rotate({1, 0, 0}, 6.0f);
        
        auto model = platform->add<ModelComponent>();
        model->model = platformModel;
        model->material = dullGray;
        model->scale = 1.0f;

        auto coll = platform->add<ColliderComponent>();
        coll->bounciness = 0.0f;
        BoxCollider c;
        c.halfExtents = {6.224951f, 0.293629f, 16.390110f};
        coll->collider = c;

        platform->add<SelectComponent>();
        entityManager.addEntity(platform);
    }

    {
        auto platform = std::make_shared<Entity>();
        platform->name() = "Island";
        platform->position() = {40.0, -10.0, 0.0};
        
        auto model = platform->add<ModelComponent>();
        model->model = floatingIslandModel;
        model->material = islandMaterial;
        model->scale = 0.5f;

        platform->add<SelectComponent>();
        entityManager.addEntity(platform);
    }

    {
        auto platform = std::make_shared<Entity>();
        platform->name() = "Platform 2";
        platform->position() = {-5.0, 0.0, 5.0};

        auto model = platform->add<ModelComponent>();
        model->model = platformModel;
        model->material = dullGray;
        model->scale = 1.0f;

        auto coll = platform->add<ColliderComponent>();
        coll->bounciness = 0.0f;
        BoxCollider c;
        c.halfExtents = {6.224951f, 0.293629f, 16.390110f};
        coll->collider = c;

        platform->add<SelectComponent>();
        entityManager.addEntity(platform);
    }

    {
        auto platform = std::make_shared<Entity>();
        platform->name() = "Platform 3";
        platform->position() = {-5.0, 0.0, 5.0};

        Maths::quat orientation = Maths::identity;
        orientation = Maths::rotate(orientation, {0, 0, 1}, 80.0f);
        orientation = Maths::rotate(orientation, {0, 1, 0}, 90.0f);
        platform->orientation() = orientation;

        auto model = platform->add<ModelComponent>();
        model->model = platformModel;
        model->material = dullGray;
        model->scale = 1.0f;

        auto coll = platform->add<ColliderComponent>();
        coll->bounciness = 0.0f;
        coll->frictionCoefficient = 0.0f;
        BoxCollider c;
        c.halfExtents = {6.224951f, 0.293629f, 16.390110f};
        coll->collider = c;

        platform->add<SelectComponent>();
        entityManager.addEntity(platform);
    }

    {
        auto crate = std::make_shared<Entity>();
        crate->name() = "Crate 1";
        crate->position() = {-5.0, 2.0, -3.0};
        crate->orientation() = Maths::rotate({0, 1, 0}, 45.0f);

        auto model = crate->add<ModelComponent>();
        model->model = crateModel;
        model->material = galaxy;
        model->scale = 1.2f;

        auto coll = crate->add<ColliderComponent>();
        coll->bounciness = 0.0f;
        coll->frictionCoefficient = 0.0f;
        BoxCollider c;
        c.halfExtents = {1.2, 1.2, 1.2};
        coll->collider = c;

        crate->add<SelectComponent>();
        entityManager.addEntity(crate);
    }

    {
        auto crate = std::make_shared<Entity>();
        crate->name() = "Crate 2";
        crate->position() = {-1.0, 0.0, -3.0};
        crate->orientation() = Maths::rotate({0, 1, 0}, 75.0f);

        auto model = crate->add<ModelComponent>();
        model->model = crateModel;
        model->material = field;
        model->scale = 1.2f;

        auto coll = crate->add<ColliderComponent>();
        coll->mass = 1000.0f;
        coll->bounciness = 0.0f;
        coll->frictionCoefficient = 0.0f;
        BoxCollider c;
        c.halfExtents = {1.2, 1.2, 1.2};
        coll->collider = c;

        crate->add<SelectComponent>();
        entityManager.addEntity(crate);
    }

    {
        auto crate = std::make_shared<Entity>();
        crate->name() = "Movable Crate";
        crate->position() = {8.0, 5.0, 7.0};
        crate->orientation() = Maths::rotate({0, 1, 0}, 75.0f);

        auto model = crate->add<ModelComponent>();
        model->model = crateModel;
        model->material = field;
        model->scale = 1.2f;

        auto phys = crate->add<PhysicsComponent>();

        auto coll = crate->add<ColliderComponent>();
        coll->mass = 10000.0f;
        coll->bounciness = 0.0f;
        coll->frictionCoefficient = 0.2f;
        BoxCollider c;
        c.halfExtents = {1.2, 1.2, 1.2};
        coll->collider = c;

        crate->add<SelectComponent>();
        entityManager.addEntity(crate);
    }

    {
        auto player = std::make_shared<Entity>();
        player->name() = "Player";
        player->position() = {0.0f, 5.0f, 5.0f};

        auto model = player->add<ModelComponent>();
        model->model = crateModel;
        model->material = shinyGray;
        model->scale = 0.3f;

        auto physics = player->add<PhysicsComponent>();

        auto coll = player->add<ColliderComponent>();
        coll->mass = 60.0f;
        coll->rollingResistance = 1.0f;
        coll->frictionCoefficient = 0.4f;
        coll->bounciness = 0.0f;
        {
            CapsuleCollider c;
            c.radius = 0.5f;
            c.height = 1.0f;
            coll->collider = c;
        }

        player->add<PlayerComponent>();
        d_playerCamera.setPlayer(player.get());

        player->add<SelectComponent>();
        entityManager.addEntity(player);
    }

    for (int i = 0; i != 5; ++i)
    {
        auto sphere = std::make_shared<Entity>();
        std::stringstream ss;
        ss << "Sphere " << i;
        sphere->name() = ss.str();
        sphere->position() = {0.0f, (float)i * 10.0f + 5.0f, 0.0f};
        
        auto model = sphere->add<ModelComponent>();
        model->model = sphereModel;
        model->material = shinyGray;
        model->scale = 0.9f;

        auto physics = sphere->add<PhysicsComponent>();

        auto coll = sphere->add<ColliderComponent>();
        coll->mass = 20.0f;
        SphereCollider c;
        c.radius = 1;
        coll->collider = c;

        sphere->add<SelectComponent>();
        entityManager.addEntity(sphere);

        if (i == 4) {
            physics->velocity = {0, 20, 0};
        }
    }

    core.window->setCursorVisibility(false);

    d_lights.push_back({{0.0f, 50.0f, 0.0f}, {0.5f, 0.4f, 0.4f}, {1.0f, 0.0f, 0.0f}});
    d_lights.push_back({{5.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    d_lights.push_back({{-7.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
    d_lights.push_back({{8.0f, 4.0f, 2.0f}, {0.3f, 0.8f, 0.2f}, {1.0f, 0.0f, 0.0f}});
    d_lights.push_back({{40.0, 20.0, 0.0}, {0.8f, 0.8f, 0.8f}, {1.0f, 0.0f, 0.0f}});

    d_postProcessor.addEffect<GaussianVert>();
    d_postProcessor.addEffect<GaussianHoriz>();
}

void WorldLayer::handleEventImpl(Sprocket::Event& event)
{
    using namespace Sprocket;

    if (auto e = event.as<WindowResizeEvent>()) {
        d_postProcessor.setScreenSize(e->width(), e->height()); 
        SPKT_LOG_INFO("Resizing!");
    }

    d_camera->handleEvent(event);
    d_lens.handleEvent(event);
    d_entityManager.handleEvent(event);
}

void WorldLayer::updateImpl()
{
    using namespace Sprocket;
    d_status = d_paused ? Status::PAUSED : Status::NORMAL;

    d_entityRenderer.update(*d_camera, d_lens, d_lights);

    if (d_status == Status::NORMAL) {
        d_camera->update(deltaTime());
        d_core.window->setCursorVisibility(d_mouseRequired);
        d_entityManager.update(deltaTime());

        for (auto& [id, entity] : d_entityManager.entities()) {
            if (entity->has<PlayerComponent>() && entity->position().y < -2.0f) {
                entity->position() = {0, 3, 0};
                entity->get<PhysicsComponent>().velocity = {0, 0, 0};
            }
            if (entity->position().y < -50.0f) {
                entity->kill();
            }
        }
    }
}

void WorldLayer::drawImpl()
{
    if (d_paused) {
        d_postProcessor.bind();
    }
    
    d_skyboxRenderer.draw(d_skybox, *d_camera, d_lens);
    d_entityManager.draw(&d_entityRenderer);
    
    if (d_paused) {
        d_postProcessor.unbind();
        d_postProcessor.draw();
    }
}
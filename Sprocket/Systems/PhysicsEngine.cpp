#include "PhysicsEngine.h"
#include "Components.h"

#include <variant>

#include "reactphysics3d.h"

namespace Sprocket {
namespace {

rp3d::Vector3 convert(const Maths::vec3& v)
{
    return rp3d::Vector3(v.x, v.y, v.z);
}

rp3d::Vector2 convert(const Maths::vec2& v)
{
    return rp3d::Vector2(v.x, v.y);
}

Maths::vec3 convert(const rp3d::Vector3& v)
{
    return Maths::vec3(v.x, v.y, v.z);
}

Maths::vec2 convert(const rp3d::Vector2& v)
{
    return Maths::vec2(v.x, v.y);
}

Maths::quat convert(const rp3d::Quaternion& q)
{
    return Maths::quat(q.w, q.x, q.y, q.z);
}

rp3d::Quaternion convert(const Maths::quat& q)
{
    return rp3d::Quaternion(q.x, q.y, q.z, q.w);
}

rp3d::Transform transform(const Entity& entity)
{
    rp3d::Transform t;
    t.setPosition(convert(entity.position()));
    t.setOrientation(convert(entity.orientation()));
    return t;
}

class RaycastCB : public rp3d::RaycastCallback
{
    Entity* d_entity = nullptr;
    float d_fraction = 10.0f;

public:
    rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) override 
    {
        if (info.hitFraction < d_fraction) {  // This object is closer.
            d_fraction = info.hitFraction;
            d_entity = reinterpret_cast<Entity*>(info.body->getUserData());
        }
        return -1.0f;
    }

    Entity* entity() const { return d_entity; }
    float fraction() const { return d_fraction; }
};

class EventListener : public rp3d::EventListener
{
public:
    void rigidBodyUpdated(const rp3d::RigidBody* body) override
    {
        Entity* entity = reinterpret_cast<Entity*>(body->getUserData());
        entity->position() = convert(body->getTransform().getPosition());
        entity->orientation() = convert(body->getTransform().getOrientation());

        if (entity->has<PhysicsComponent>()) {
            entity->get<PhysicsComponent>().velocity = convert(body->getLinearVelocity());
        }
    }
};

float getSpeed(SpeedFactor s)
{
    switch (s) {
        case SpeedFactor::QUARTER:   return 0.25f;
        case SpeedFactor::HALF:      return 0.5f;
        case SpeedFactor::NORMAL:    return 1.0f;
        case SpeedFactor::DOUBLE:    return 2.0f;
        case SpeedFactor::QUADRUPLE: return 4.0f;
    }

    SPKT_LOG_ERROR("Speed not found! Returning 1.0f");
    return 1.0f;
};

}

struct PhysicsEngineImpl
{
    rp3d::DynamicsWorld world;

    std::unordered_map<
        std::size_t,
        std::shared_ptr<rp3d::CollisionShape>
    > collisionShapes;
        // This is just to manage the lifetimes of the collision bodies. 
        // May want to enhance this in the future for some optimising.

    std::unordered_map<
        std::size_t,
        rp3d::RigidBody*
    > rigidBodies; 
        // Lifetime of RidigBody managed by RapidPhysics3D?

    EventListener eventListener;

    PhysicsEngineImpl(const Maths::vec3& gravity)
        : world(convert(gravity))
    {}
};

PhysicsEngine::PhysicsEngine(const Maths::vec3& gravity)
    : d_impl(std::make_shared<PhysicsEngineImpl>(gravity))
    , d_timeStep(1.0f / 120.0f)
    , d_lastFrameLength(0)
    , d_speedFactor(SpeedFactor::NORMAL)
    , d_running(true)
{
    d_impl->world.setNbIterationsPositionSolver(5);
    d_impl->world.setNbIterationsVelocitySolver(8);
    d_impl->world.setEventListener(&d_impl->eventListener);
}

void PhysicsEngine::updateSystem(float dt)
{
    if (!d_running) {
        return;
    }

    float speedFactor = getSpeed(d_speedFactor);

    float frameLength = dt * speedFactor;
    d_lastFrameLength = 0;

    static float accumulator = 0.0f;
    accumulator += frameLength;

    // First update the Physics World.
    while (accumulator >= d_timeStep) {
        d_impl->world.update(d_timeStep);
        accumulator -= d_timeStep;
        d_lastFrameLength += d_timeStep;
    }
}

void PhysicsEngine::updateEntity(Entity& entity, float dt)
{
    if (!d_running) {
        return;
    }

    if (entity.has<PhysicsComponent>()) {
        const auto& physics = entity.get<PhysicsComponent>();
        auto bodyData = d_impl->rigidBodies[entity.id()];

        // Update the RigidBody corresponding to this Entity.
        bodyData->setTransform(transform(entity));

        bodyData->setMass(physics.mass);
        bodyData->setLinearVelocity(convert(physics.velocity));
        bodyData->enableGravity(physics.gravity);
        bodyData->setType(physics.stationary ? rp3d::BodyType::STATIC
                                             : rp3d::BodyType::DYNAMIC);

        auto& material = bodyData->getMaterial();
        material.setBounciness(physics.bounciness);
        material.setFrictionCoefficient(physics.frictionCoefficient);
        material.setRollingResistance(physics.rollingResistance);        
    }

    // Handle player movement updates.
    if (entity.has<PlayerComponent>()) {
        updatePlayer(entity, dt);
    }
}

void PhysicsEngine::registerEntity(const Entity& entity)
{
    if (!entity.has<PhysicsComponent>()) {
        return;  // Entity must have physics.
    }

    auto& physicsData = entity.get<PhysicsComponent>();
    auto& entry = d_impl->rigidBodies[entity.id()];

    entry = d_impl->world.createRigidBody(transform(entity));

    // Give each RigidBody a pointer back to the original Entity object.
    entry->setUserData(const_cast<void*>(reinterpret_cast<const void*>(&entity)));
    if (physicsData.stationary) {
        entry->setType(rp3d::BodyType::STATIC);
    }
    else {
        entry->setType(rp3d::BodyType::DYNAMIC);
    }

    if (entity.has<PlayerComponent>()) {
        entry->setAngularDamping(0.0f);
    }

    std::shared_ptr<rp3d::CollisionShape> collider = nullptr;
    if (auto data = std::get_if<BoxCollider>(&physicsData.collider)) {
        collider = std::make_shared<rp3d::BoxShape>(convert(data->halfExtents));
    }
    else if (auto data = std::get_if<SphereCollider>(&physicsData.collider)) {
        collider = std::make_shared<rp3d::SphereShape>(data->radius);
    }
    else if (auto data = std::get_if<CapsuleCollider>(&physicsData.collider)) {
        collider = std::make_shared<rp3d::CapsuleShape>(
            data->radius, data->height
        );
    }
    else {
        return; // No collision data.
    }

    auto ps = entry->addCollisionShape(
        collider.get(),
        rp3d::Transform::identity(),
        physicsData.mass
    );

    d_impl->collisionShapes[entity.id()] = collider;
}

void PhysicsEngine::deregisterEntity(const Entity& entity)
{
    if (!entity.has<PhysicsComponent>()) {
        // Entity must have physics.
        return; 
    }

    auto rigidBodyIt = d_impl->rigidBodies.find(entity.id());
    if (rigidBodyIt == d_impl->rigidBodies.end()) {
        return;  // Nothing to delete.
    }

    d_impl->world.destroyRigidBody(rigidBodyIt->second);

    // TODO: Clean up of collision bodies
}

void PhysicsEngine::running(bool isRunning)
{
    d_running = isRunning;
}

Entity* PhysicsEngine::raycast(const Maths::vec3& base,
                               const Maths::vec3& direction)
{
    Maths::vec3 d = direction;
    Maths::normalise(d);
    d *= 100.0f;

    rp3d::Vector3 start = convert(base);
    rp3d::Vector3 end = start + convert(d);

    rp3d::Ray ray(start, end);
    RaycastCB cb;
    d_impl->world.raycast(ray, &cb);
    return cb.entity();
}

void PhysicsEngine::updatePlayer(Entity& entity, float dt)
{
    if (d_lastFrameLength == 0) {
        return;  // Physics engine not advanced this frame.
    }

    auto& bodyData = d_impl->rigidBodies[entity.id()];
    
    const auto& player = entity.get<PlayerComponent>();
    const auto& physics = entity.get<PhysicsComponent>();

    // Raycast down to see if there is an object underneath
    auto aabb = bodyData->getAABB();
    rp3d::Vector3 playerBase = aabb.getCenter();
    playerBase.y = aabb.getMin().y;

    // On floor check
    rp3d::Vector3 up(0.0f, 1.0f, 0.0f);
    float delta = 0.1f;
    rp3d::Ray ray(playerBase + delta * up, playerBase - 2 * delta * up);
    RaycastCB cb;
    d_impl->world.raycast(ray, &cb);
    bool onFloor = cb.entity();

    // Keep player upright
    rp3d::Transform transform = bodyData->getTransform();
    rp3d::Quaternion q = rp3d::Quaternion::fromEulerAngles(
        0.0f, Maths::radians(player.yaw), 0.0f);
    transform.setOrientation(q);
    bodyData->setTransform(transform);
    entity.orientation() = convert(q);

    // Player Movement
    float cosYaw = Maths::cosd(player.yaw);
    float sinYaw = Maths::sind(player.yaw);
    rp3d::Vector3 forwards(-sinYaw, 0, -cosYaw);
    rp3d::Vector3 right(cosYaw, 0, -sinYaw);

    rp3d::Vector3 direction;
    if (player.movingForwards) { direction += forwards; }
    if (player.movingBackwards) { direction -= forwards; }
    if (player.movingRight) { direction += right; }
    if (player.movingLeft) { direction -= right; }
    direction.normalize();

    if (direction.length() != 0.0f || onFloor) {
        float speed = 3.0f;
        rp3d::Vector3 dv = (speed * direction) - convert(physics.velocity);
        dv.y = 0.0f;  // Only consider horizontal movement.
        float dtime = d_lastFrameLength;
        rp3d::Vector3 acceleration = dv / dtime;
        bodyData->applyForceToCenterOfMass(bodyData->getMass() * acceleration);
    }

    // Jumping
    if (onFloor && player.jumping) {
        float speed = 6.0f;
        rp3d::Vector3 dv = (speed - physics.velocity.y) * up;
        float dtime = d_lastFrameLength;
        rp3d::Vector3 acceleration = dv / dtime;
        bodyData->applyForceToCenterOfMass(bodyData->getMass() * acceleration);
    }
}

}
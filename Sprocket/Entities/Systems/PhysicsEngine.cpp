#include "PhysicsEngine.h"
#include "Log.h"
#include "EntityManager.h"
#include "TransformComponent.h"
#include "PhysicsComponent.h"
#include "PlayerComponent.h"

#include <variant>

#include "reactphysics3d.h"

namespace Sprocket {
namespace {

rp3d::Vector3 Convert(const Maths::vec3& v)
{
    return rp3d::Vector3(v.x, v.y, v.z);
}

rp3d::Vector2 Convert(const Maths::vec2& v)
{
    return rp3d::Vector2(v.x, v.y);
}

Maths::vec3 Convert(const rp3d::Vector3& v)
{
    return Maths::vec3(v.x, v.y, v.z);
}

Maths::vec2 Convert(const rp3d::Vector2& v)
{
    return Maths::vec2(v.x, v.y);
}

Maths::quat Convert(const rp3d::Quaternion& q)
{
    return Maths::quat(q.w, q.x, q.y, q.z);
}

rp3d::Quaternion Convert(const Maths::quat& q)
{
    return rp3d::Quaternion(q.x, q.y, q.z, q.w);
}

rp3d::Transform Convert(const TransformComponent& transform)
{
    rp3d::Transform t;
    t.setPosition(Convert(transform.position));
    t.setOrientation(Convert(transform.orientation));
    return t;
}

class RaycastCB : public rp3d::RaycastCallback
{
    Entity d_entity = Entity();
    float d_fraction = 10.0f;

public:
    rp3d::decimal notifyRaycastHit(const rp3d::RaycastInfo& info) override 
    {
        if (info.hitFraction < d_fraction) {  // This object is closer.
            d_fraction = info.hitFraction;
            d_entity = *static_cast<Entity*>(info.body->getUserData());
        }
        return -1.0f;
    }

    Entity GetEntity() const { return d_entity; }
    float Fraction() const { return d_fraction; }
};

float GetSpeed(SpeedFactor s)
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

struct EntityData
{
    Entity                                entity;
    rp3d::RigidBody*                      rigidBody;
    rp3d::ProxyShape*                     proxyShape     = nullptr;
    std::shared_ptr<rp3d::CollisionShape> collisionShape = nullptr;
};

struct PhysicsEngineImpl
{
    rp3d::DynamicsWorld world;

    std::unordered_map<uint32_t, EntityData> entityData;

    PhysicsEngineImpl(const Maths::vec3& gravity)
        : world(Convert(gravity))
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
}

void PhysicsEngine::OnStartup(EntityManager& manager)
{
    manager.OnAdd<PhysicsComponent>([&](Entity& entity) {
        assert(entity.Has<TransformComponent>());

        auto& transform = entity.Get<TransformComponent>();
        auto& physics = entity.Get<PhysicsComponent>();

        d_impl->entityData[entity.Id()].entity = entity;
        auto* e = &d_impl->entityData[entity.Id()].entity;

        auto& entry = d_impl->entityData[entity.Id()].rigidBody;
        entry = d_impl->world.createRigidBody(Convert(transform));

        // Give each RigidBody a ref back to the original Entity object.
        entry->setUserData(static_cast<void*>(e));
        entry->setType(physics.frozen ? rp3d::BodyType::STATIC : rp3d::BodyType::DYNAMIC);

        if (entity.Has<PlayerComponent>()) {
            entry->setAngularDamping(0.0f);
        }

        AddCollider(entity);
    });

    manager.OnRemove<PhysicsComponent>([&](Entity& entity) {
        auto& physics = entity.Get<PhysicsComponent>();

        auto rigidBodyIt = d_impl->entityData.find(entity.Id());
        d_impl->world.destroyRigidBody(rigidBodyIt->second.rigidBody);
        d_impl->entityData.erase(rigidBodyIt);
        // TODO: Clean up of collision bodies
    });
}

void PhysicsEngine::OnUpdate(EntityManager& manager, double dt)
{
    if (!d_running) { return; }

    // Pre Update
    manager.Each<TransformComponent, PhysicsComponent>([&] (Entity& entity) {
        const auto& transform = entity.Get<TransformComponent>();
        const auto& physics = entity.Get<PhysicsComponent>();
        auto bodyData = d_impl->entityData[entity.Id()].rigidBody;
        
        bodyData->setTransform(Convert(transform));
        bodyData->setLinearVelocity(Convert(physics.velocity));
        bodyData->enableGravity(physics.gravity);    
        bodyData->setType(physics.frozen ? rp3d::BodyType::STATIC : rp3d::BodyType::DYNAMIC);
        bodyData->setMass(physics.mass);

        auto& material = bodyData->getMaterial();
        material.setBounciness(physics.bounciness);
        material.setFrictionCoefficient(physics.frictionCoefficient);
        material.setRollingResistance(physics.rollingResistance);

        if (entity.Has<PlayerComponent>()) { // Handle player movement updates.
            UpdatePlayer(dt, entity);
        }
    });

    // Update System
    float speedFactor = GetSpeed(d_speedFactor);
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

    // Post Update
    manager.Each<TransformComponent, PhysicsComponent>([&] (Entity& entity) {
        auto& transform = entity.Get<TransformComponent>();
        auto& physics = entity.Get<PhysicsComponent>();
        const auto& bodyData = d_impl->entityData[entity.Id()].rigidBody;

        transform.position = Convert(bodyData->getTransform().getPosition());
        transform.orientation = Convert(bodyData->getTransform().getOrientation());
        physics.velocity = Convert(bodyData->getLinearVelocity());
    });
}

void PhysicsEngine::AddCollider(Entity entity)
{
    auto& physics = entity.Get<PhysicsComponent>();

    std::shared_ptr<rp3d::CollisionShape> collider = nullptr;
    if (physics.collider == Collider::BOX) {
        collider = std::make_shared<rp3d::BoxShape>(Convert(physics.halfExtents));
    }
    else if (physics.collider == Collider::SPHERE) {
        collider = std::make_shared<rp3d::SphereShape>(physics.radius);
    }
    else if (physics.collider == Collider::CAPSULE) {
        collider = std::make_shared<rp3d::CapsuleShape>(
            physics.radius, physics.height
        );
    }
    else {
        return; // No collision data.
    }
    d_impl->entityData[entity.Id()].collisionShape = collider;

    auto entry = d_impl->entityData[entity.Id()].rigidBody;
    d_impl->entityData[entity.Id()].proxyShape = entry->addCollisionShape(
        collider.get(),
        rp3d::Transform::identity(),
        physics.mass
    );
}

void PhysicsEngine::Running(bool isRunning)
{
    d_running = isRunning;
}

Entity PhysicsEngine::Raycast(const Maths::vec3& base,
                              const Maths::vec3& direction)
{
    Maths::vec3 d = direction;
    Maths::Normalise(d);
    d *= 100.0f;

    rp3d::Vector3 start = Convert(base);
    rp3d::Vector3 end = start + Convert(d);

    rp3d::Ray ray(start, end);
    RaycastCB cb;
    d_impl->world.raycast(ray, &cb);
    return cb.GetEntity();
}

void PhysicsEngine::UpdatePlayer(double dt, Entity entity)
{
    if (d_lastFrameLength == 0) {
        return;  // Physics engine not advanced this frame.
    }

    const auto& player = entity.Get<PlayerComponent>();
    const auto& physics = entity.Get<PhysicsComponent>();

    float mass = d_impl->entityData[entity.Id()].rigidBody->getMass();
        // Sum of all colliders plus rigid body.

    MakeUpright(entity, player.yaw);
    
    bool onFloor = IsOnFloor(entity);

    if (player.direction.length() != 0.0f || onFloor) {
        float speed = 3.0f;
        Maths::vec3 dv = (speed * player.direction) - physics.velocity;
        dv.y = 0.0f;  // Only consider horizontal movement.
        Maths::vec3 acceleration = dv / d_lastFrameLength;
        ApplyForce(entity, mass * acceleration);
    }

    // Jumping
    if (onFloor && player.jumping) {
        float speed = 6.0f;
        Maths::vec3 dv = (speed - physics.velocity.y) * Maths::vec3(0, 1, 0);
        Maths::vec3 acceleration = dv / d_lastFrameLength;
        ApplyForce(entity, mass * acceleration);
    }
}

void PhysicsEngine::ApplyForce(Entity entity, const Maths::vec3& force)
{
    auto& bodyData = d_impl->entityData[entity.Id()].rigidBody;
    bodyData->applyForceToCenterOfMass(Convert(force));
}

void PhysicsEngine::MakeUpright(Entity entity, float yaw)
{
    auto& bodyData = d_impl->entityData[entity.Id()].rigidBody;
    rp3d::Transform transform = bodyData->getTransform();
    rp3d::Quaternion q = rp3d::Quaternion::fromEulerAngles(
        0.0f, Maths::Radians(yaw), 0.0f);
    transform.setOrientation(q);
    bodyData->setTransform(transform);
    entity.Get<TransformComponent>().orientation = Convert(q);
}

bool PhysicsEngine::IsOnFloor(Entity entity) const
{
    auto& bodyData = d_impl->entityData[entity.Id()].rigidBody;

    // Get the point at the bottom of the rigid body.
    auto aabb = bodyData->getAABB();
    rp3d::Vector3 playerBase = aabb.getCenter();
    playerBase.y = aabb.getMin().y;

    // Raycast down from this point. We actually raycast from slightly
    // higher which works more consistently for some reason. TODO: Find
    // out why
    rp3d::Vector3 up(0.0f, 1.0f, 0.0f);
    float delta = 0.1f;
    rp3d::Ray ray(playerBase + delta * up, playerBase - 2 * delta * up);
    RaycastCB cb;
    d_impl->world.raycast(ray, &cb);

    auto e = cb.GetEntity();
    return !e.Null();
}

void PhysicsEngine::RefreshTransform(Entity entity)
{
    if (!entity.Has<PhysicsComponent>()) { return; }

    auto bodyData = d_impl->entityData[entity.Id()].rigidBody;
    bodyData->setTransform(Convert(entity.Get<TransformComponent>()));
}

}
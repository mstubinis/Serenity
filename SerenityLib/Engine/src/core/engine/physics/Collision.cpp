#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/physics/Collision.h>
#include <core/engine/math/Engine_Math.h>

#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/mesh/BuiltInMeshes.h>
#include <core/engine/system/Engine.h>
#include <core/engine/model/ModelInstance.h>
#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletDynamicsCommon.h>
#include <btBulletCollisionCommon.h>
#include <LinearMath/btIDebugDraw.h>

void Collision::internal_load_1(Engine::view_ptr<Collision> collision, CollisionType collisionType, Handle mesh) {
    auto body  = collision->m_Owner.getComponent<ComponentBody>();
    collision->m_DeferredMeshes.clear();
    auto scale = body->getScale();
    Engine::Physics::removeRigidBodyThreadSafe(*body);
    collision->internal_free_memory();

    collision->m_BtShape = std::unique_ptr<btCollisionShape>(Engine::priv::InternalMeshPublicInterface::BuildCollision(mesh, collisionType));
    body->rebuildRigidBody(true, true);
    body->setScale(scale);
    auto model = collision->m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}
void Collision::internal_load_2(Engine::view_ptr<Collision> collision, Engine::view_ptr<btCompoundShape> btCompound, std::vector<Engine::view_ptr<ModelInstance>> instances, float mass, CollisionType collisionType) {
    auto body  = collision->m_Owner.getComponent<ComponentBody>();
    auto scale = body->getScale();
    for (auto& instance : instances) {
        btCollisionShape* built_collision_shape = Engine::priv::InternalMeshPublicInterface::BuildCollision(instance, collisionType, true);
        btTransform localTransform(Engine::Math::glmToBTQuat(instance->orientation()), Engine::Math::btVectorFromGLM(instance->position()));
        built_collision_shape->setMargin(0.001f);
        built_collision_shape->calculateLocalInertia(mass, collision->m_BtInertia); //this is important
        btCompound->addChildShape(localTransform, built_collision_shape);
    }
    body->rebuildRigidBody(true, true);
    body->setScale(scale);

    auto model = collision->m_Owner.getComponent<ComponentModel>();
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
}

void Collision::internal_base_init(CollisionType type, float mass) {
    m_Type = type;
    setMass(mass);
}
Collision::Collision(ComponentBody& body)
    : m_Owner{ body.getOwner() }
{
    internal_base_init(body.getCollision()->getType(), body.mass());
}
Collision::Collision(ComponentBody& body, CollisionType type, Engine::view_ptr<ModelInstance> modelInstance, float mass)
    : m_Owner{ body.getOwner() }
{
    if (modelInstance) {
        Handle meshHandle = modelInstance->mesh();
        auto& mesh        = *meshHandle.get<Mesh>();
        if (!mesh.isLoaded() || !mesh.m_CPUData.m_CollisionFactory) {
            m_BtShape = std::unique_ptr<btCollisionShape>(Engine::priv::InternalMeshPublicInterface::BuildCollision(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type));
            m_DeferredMeshes.emplace_back(meshHandle);
            m_DeferredLoadingFunction = [type, this, mass, meshHandle]() {
                internal_load_1(this, type, meshHandle);
            };
            registerEvent(EventType::ResourceLoaded);
        }else{
            m_BtShape = std::unique_ptr<btCollisionShape>(Engine::priv::InternalMeshPublicInterface::BuildCollision(modelInstance, type));
        }
    }else{
        m_BtShape = std::unique_ptr<btCollisionShape>(Engine::priv::InternalMeshPublicInterface::BuildCollision(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type));
    }
    internal_base_init(type, mass);
}
Collision::Collision(ComponentBody& body, CollisionType type, Handle meshHandle, float mass)
    : m_Owner{ body.getOwner() }
{
    auto& mesh = *meshHandle.get<Mesh>();
    if (!mesh.isLoaded() || !mesh.m_CPUData.m_CollisionFactory) {
        m_BtShape = std::unique_ptr<btCollisionShape>(Engine::priv::InternalMeshPublicInterface::BuildCollision(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type));
        m_DeferredMeshes.emplace_back(meshHandle);
        m_DeferredLoadingFunction = [this, type, mass, meshHandle]() {
            internal_load_1(this, type, meshHandle);
        };
        registerEvent(EventType::ResourceLoaded);
    }else{
        m_BtShape = std::unique_ptr<btCollisionShape>(Engine::priv::InternalMeshPublicInterface::BuildCollision(meshHandle, type));
    }
    internal_base_init(type, mass);
}
Collision::Collision(ComponentBody& body, ComponentModel& modelComponent, float mass, CollisionType type)
    : m_Owner{ body.getOwner() }
{

    std::vector<Engine::view_ptr<ModelInstance>> modelInstances;
    modelInstances.reserve(modelComponent.getNumModels());
    for (size_t i = 0; i < modelComponent.getNumModels(); ++i) {
        modelInstances.emplace_back(&modelComponent.getModel(i));
    }
    btCompoundShape* btCompound = new btCompoundShape();
    std::vector<Engine::view_ptr<ModelInstance>> unfinishedModels;
    unfinishedModels.reserve(modelInstances.size());
    for (size_t i = 0; i < modelInstances.size(); ++i) {
        auto& instance  = *modelInstances[i];
        auto meshHandle = instance.mesh();
        auto& mesh      = *meshHandle.get<Mesh>();
        if (!mesh.isLoaded()) {
            m_DeferredMeshes.emplace_back(meshHandle);
            unfinishedModels.emplace_back(&instance);
        }else{
            btCollisionShape* built_collision_shape = Engine::priv::InternalMeshPublicInterface::BuildCollision(&instance, type, true);
            btTransform localTransform = btTransform(Engine::Math::glmToBTQuat(instance.orientation()), Engine::Math::btVectorFromGLM(instance.position()));
            built_collision_shape->setMargin(0.001f);
            if (built_collision_shape->getShapeType() != BroadphaseNativeTypes::EMPTY_SHAPE_PROXYTYPE) {
                built_collision_shape->calculateLocalInertia(mass, m_BtInertia); //this is important
            }
            btCompound->addChildShape(localTransform, built_collision_shape);
        }
    }
    if (unfinishedModels.size() > 0) {
        m_DeferredLoadingFunction = [this, mass, type, unfinishedModels]() {
            internal_load_2(this, (btCompoundShape*)m_BtShape.get(), unfinishedModels, mass, type);
        };
        registerEvent(EventType::ResourceLoaded);
    }
    btCompound->setMargin(0.001f);
    btCompound->recalculateLocalAabb();
    btCompound->setUserPointer(&body);
    internal_free_memory();
    m_BtShape = std::unique_ptr<btCollisionShape>(btCompound);
    setMass(mass);
}
Collision::~Collision() {
    internal_free_memory();
}
void Collision::internal_free_memory() {
    if (m_BtShape && m_BtShape->isCompound()) {
        btCompoundShape* compound = (btCompoundShape*)m_BtShape.get();
        int numChildren           = compound->getNumChildShapes();
        if (numChildren > 0) {
            for (int i = 0; i < numChildren; ++i) {
                btCollisionShape* child_shape = compound->getChildShape(i);
                SAFE_DELETE(child_shape);
            }
        }
    }  
    m_BtShape.reset(nullptr);
}
void Collision::setMass(float mass) noexcept {
    if (!m_BtShape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None) {
        return;
    }
    if (m_BtShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {    
        if (m_BtShape->isCompound()) {
            btCompoundShape* compound = (btCompoundShape*)m_BtShape.get();
            int numChildren = compound->getNumChildShapes();
            for (int i = 0; i < numChildren; ++i) {
                btCollisionShape* child_shape = compound->getChildShape(i);
                if(child_shape){
                    child_shape->calculateLocalInertia(mass, m_BtInertia);
                }
            }
        }    
        m_BtShape->calculateLocalInertia(mass, m_BtInertia);
    }
}
void Collision::onEvent(const Event& e) {
    if (e.type == EventType::ResourceLoaded && e.eventResource.resource->type() == ResourceType::Mesh && m_DeferredMeshes.size() > 0) {
        auto mesh = (Mesh*)e.eventResource.resource;
        std::erase_if(m_DeferredMeshes, [mesh](Mesh* deferred_mesh) {
            return (mesh == deferred_mesh || (deferred_mesh->isLoaded() && deferred_mesh->m_CPUData.m_CollisionFactory));
        });
        if (m_DeferredMeshes.size() == 0) {
            m_DeferredLoadingFunction();
            unregisterEvent(EventType::ResourceLoaded);
        }
    }
}
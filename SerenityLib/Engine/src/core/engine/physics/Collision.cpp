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

using namespace Engine;
using namespace Engine::priv;
using namespace std;

void Collision::DeferredLoading::load_1(Collision* collision, CollisionType collisionType, Mesh* mesh, float mass) {
    auto* body = collision->m_Owner.getComponent<ComponentBody>();

    collision->m_DeferredMeshes.clear();
    const auto scale = body->getScale();
    Physics::removeRigidBodyThreadSafe(*body);
    collision->free_memory();

    collision->m_BtShape = InternalMeshPublicInterface::BuildCollision(mesh, collisionType);
    body->rebuildRigidBody(true, true);
    body->setScale(scale);
    auto* model = collision->m_Owner.getComponent<ComponentModel>();
    if (model) {
        ComponentModel_Functions::CalculateRadius(*model);
    }
}
void Collision::DeferredLoading::load_2(Collision* collision, btCompoundShape* btCompound, vector<ModelInstance*> instances, float mass, CollisionType collisionType) {
    auto* body = collision->m_Owner.getComponent<ComponentBody>();
    btTransform localTransform;
    const auto scale = body->getScale();
    for (auto& instance : instances) {
        btCollisionShape* built_collision_shape = InternalMeshPublicInterface::BuildCollision(instance, collisionType, true);
        localTransform = btTransform(Math::glmToBTQuat(instance->orientation()), Math::btVectorFromGLM(instance->position()));
        built_collision_shape->setMargin(0.001f);
        built_collision_shape->calculateLocalInertia(mass, collision->m_BtInertia); //this is important
        btCompound->addChildShape(localTransform, built_collision_shape);
    }
    body->rebuildRigidBody(true, true);
    body->setScale(scale);

    auto* model = collision->m_Owner.getComponent<ComponentModel>();
    if (model) {
        ComponentModel_Functions::CalculateRadius(*model);
    }
}

void Collision::internal_base_init(CollisionType type, float mass) {
    m_Type = type;
    setMass(mass);
}
Collision::Collision(ComponentBody& body){
    m_Owner = body.getOwner();
    internal_base_init(body.getCollision()->getType(), body.mass());
}
Collision::Collision(ComponentBody& body, CollisionType type, ModelInstance* modelInstance, float mass){
    m_Owner = body.getOwner();
    m_BtShape = nullptr;
    if (modelInstance) {
        auto* mesh_ptr = modelInstance->mesh();
        if (*mesh_ptr == false || !mesh_ptr->m_CollisionFactory) {
            m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type);
            m_DeferredMeshes.push_back(mesh_ptr);
            m_DeferredLoading = [type, this, mass, mesh_ptr]() { Collision::DeferredLoading::load_1(this, type, mesh_ptr, mass); };
            registerEvent(EventType::ResourceLoaded);
        }else{
            m_BtShape = InternalMeshPublicInterface::BuildCollision(modelInstance, type);
        }
    }else{
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type);
    }
    internal_base_init(type, mass);
}
Collision::Collision(ComponentBody& body, CollisionType type, Mesh& mesh, float mass){
    m_Owner   = body.getOwner();
    m_BtShape = nullptr;
    if (mesh == false || !mesh.m_CollisionFactory) {
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type);
        auto* mesh_ptr = &mesh;
        m_DeferredMeshes.push_back(mesh_ptr);
        m_DeferredLoading = [type, this, mass, mesh_ptr]() { Collision::DeferredLoading::load_1(this, type, mesh_ptr, mass); };
        registerEvent(EventType::ResourceLoaded);
    }else{
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&mesh, type);
    }
    internal_base_init(type, mass);
}

Collision::Collision(ComponentBody& body, ComponentModel& modelComponent, float mass, CollisionType type){
    m_Owner   = body.getOwner();
    m_BtShape = nullptr;
    vector<ModelInstance*> modelInstances;
    modelInstances.reserve(modelComponent.getNumModels());
    for (size_t i = 0; i < modelComponent.getNumModels(); ++i) {
        modelInstances.push_back(&modelComponent.getModel(i));
    }
    btCompoundShape* btCompound = new btCompoundShape();
    vector<ModelInstance*> unfinishedModels;
    btTransform localTransform;
    for (size_t i = 0; i < modelInstances.size(); ++i) {
        auto& instance = *modelInstances[i];
        auto& mesh = *instance.mesh();

        if (mesh == false) {
            m_DeferredMeshes.push_back(&mesh);
            unfinishedModels.push_back(&instance);
        }else{
            btCollisionShape* built_collision_shape = InternalMeshPublicInterface::BuildCollision(&instance, type, true);
            localTransform = btTransform(Math::glmToBTQuat(instance.orientation()), Math::btVectorFromGLM(instance.position()));
            built_collision_shape->setMargin(0.001f);
            if(built_collision_shape->getShapeType() != BroadphaseNativeTypes::EMPTY_SHAPE_PROXYTYPE)
                built_collision_shape->calculateLocalInertia(mass, m_BtInertia); //this is important
            btCompound->addChildShape(localTransform, built_collision_shape);
        }
    }
    if (unfinishedModels.size() > 0) {
        m_DeferredLoading = [=]() { Collision::DeferredLoading::load_2(this, btCompound, unfinishedModels, mass, type); };
        registerEvent(EventType::ResourceLoaded);
    }
    btCompound->setMargin(0.001f);
    btCompound->recalculateLocalAabb();
    btCompound->setUserPointer(&body);
    free_memory();
    m_BtShape = btCompound;
    setMass(mass);
}
void Collision::free_memory() {
    if (m_BtShape && m_BtShape->isCompound()) {
        btCompoundShape* compound = static_cast<btCompoundShape*>(m_BtShape);
        const int numChildren = compound->getNumChildShapes();
        if (numChildren > 0) {
            for (int i = 0; i < numChildren; ++i) {
                btCollisionShape* child_shape = compound->getChildShape(i);
                SAFE_DELETE(child_shape);
            }
        }
    }  
    SAFE_DELETE(m_BtShape);
}
Collision::~Collision() {
    free_memory();
} 
Collision::Collision(Collision&& other) noexcept {
    m_Owner     = std::move(other.m_Owner);
    m_BtInertia = std::move(other.m_BtInertia);
    m_Type      = std::move(other.m_Type);
    m_BtShape   = std::exchange(other.m_BtShape, nullptr);
}
Collision& Collision::operator=(Collision&& other) noexcept {
    if (&other != this) {
        m_Owner     = std::move(other.m_Owner);
        m_BtInertia = std::move(other.m_BtInertia);
        m_Type      = std::move(other.m_Type);
        m_BtShape   = std::exchange(other.m_BtShape, nullptr);
    }
    return *this;
}
void Collision::setMass(float mass) {
    if (!m_BtShape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None) {
        return;
    }
    if (m_BtShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {    
        if (m_BtShape->isCompound()) {
            btCompoundShape* compound = static_cast<btCompoundShape*>(m_BtShape);
            const int numChildren = compound->getNumChildShapes();
            if (numChildren > 0) {
                for (int i = 0; i < numChildren; ++i) {
                    btCollisionShape* child_shape = compound->getChildShape(i);
                    if(child_shape){
                        child_shape->calculateLocalInertia(mass, m_BtInertia);
                    }
                }
            }
        }    
        m_BtShape->calculateLocalInertia(mass, m_BtInertia);
    }
}
void Collision::onEvent(const Event& event_) {
    if (event_.type == EventType::ResourceLoaded) {
        if (event_.eventResource.resource->type() == ResourceType::Mesh && m_DeferredMeshes.size() > 0) {
            auto* mesh = (Mesh*)event_.eventResource.resource;
            auto it = m_DeferredMeshes.begin();
            while (it != m_DeferredMeshes.end()) {
                Mesh& deferred_mesh = *(*it);
                if (mesh == (*it) || (deferred_mesh == true && deferred_mesh.m_CollisionFactory)) {
                    it = m_DeferredMeshes.erase(it);
                }else{
                    ++it;
                }
            }
            if (m_DeferredMeshes.size() == 0) {
                m_DeferredLoading();
                unregisterEvent(EventType::ResourceLoaded);
            }
        }
    }
}
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

void Collision::DeferredLoading::load_1(Collision* collision, const CollisionType::Type collisionType, Mesh* mesh, const float mass) {
    auto& body = *collision->m_Owner.getComponent<ComponentBody>();

    collision->m_DeferredMeshes.clear();
    const auto scale = body.getScale();
    Physics::removeRigidBodyThreadSafe(body);
    collision->free_memory();

    collision->m_BtShape = InternalMeshPublicInterface::BuildCollision(mesh, collisionType);
    body.rebuildRigidBody(true, true);
    body.setScale(scale);
    auto* model = collision->m_Owner.getComponent<ComponentModel>();
    if (model)
        ComponentModel_Functions::CalculateRadius(*model);
}
void Collision::DeferredLoading::load_2(Collision* collision, btCompoundShape* btCompound, vector<ModelInstance*> instances, const float mass, const CollisionType::Type collisionType) {
    auto& body = *collision->m_Owner.getComponent<ComponentBody>();
    btTransform localTransform;
    const auto scale = body.getScale();
    for (auto& instance : instances) {
        btCollisionShape* built_collision_shape = InternalMeshPublicInterface::BuildCollision(instance, collisionType, true);
        localTransform = btTransform(Math::glmToBTQuat(instance->orientation()), Math::btVectorFromGLM(instance->position()));
        built_collision_shape->setMargin(0.001f);
        built_collision_shape->calculateLocalInertia(mass, collision->m_BtInertia); //this is important
        btCompound->addChildShape(localTransform, built_collision_shape);
    }
    body.rebuildRigidBody(true, true);
    body.setScale(scale);

    auto* model = collision->m_Owner.getComponent<ComponentModel>();
    if (model)
        ComponentModel_Functions::CalculateRadius(*model);
}

void Collision::_baseInit(const CollisionType::Type type, const float mass) {
    m_Type = type;
    setMass(mass);
}
Collision::Collision(ComponentBody& body){
    m_Owner = body.getOwner();
    setMass(0.0f);
}
Collision::Collision(ComponentBody& body, const CollisionType::Type type, ModelInstance* modelInstance, const float mass){
    m_Owner = body.getOwner();
    m_BtShape = nullptr;
    if (modelInstance) {
        auto* mesh_ptr = modelInstance->mesh();
        if (*mesh_ptr == false || !mesh_ptr->m_CollisionFactory) {
            m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type);
            m_DeferredMeshes.push_back(mesh_ptr);
            m_DeferredLoading = [type, this, mass, mesh_ptr]() { Collision::DeferredLoading::load_1(this, type, mesh_ptr, mass); };
            registerEvent(EventType::MeshLoaded);
        }else{
            m_BtShape = InternalMeshPublicInterface::BuildCollision(modelInstance, type);
        }
    }else{
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type);
    }
    _baseInit(type, mass);
}
Collision::Collision(ComponentBody& body, const CollisionType::Type type, Mesh& mesh, const float mass){
    m_Owner = body.getOwner();
    m_BtShape = nullptr;
    if (mesh == false || !mesh.m_CollisionFactory) {
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), type);
        auto* mesh_ptr = &mesh;
        m_DeferredMeshes.push_back(mesh_ptr);
        m_DeferredLoading = [type, this, mass, mesh_ptr]() { Collision::DeferredLoading::load_1(this, type, mesh_ptr, mass); };
        registerEvent(EventType::MeshLoaded);
    }else{
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&mesh, type);
    }
    _baseInit(type, mass);
}

Collision::Collision(ComponentBody& body, ComponentModel& modelComponent, const float mass, const CollisionType::Type type){
    m_Owner = body.getOwner();
    m_BtShape = nullptr;
    vector<ModelInstance*> modelInstances;
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
            built_collision_shape->calculateLocalInertia(mass, m_BtInertia); //this is important
            btCompound->addChildShape(localTransform, built_collision_shape);
        }
    }
    if (unfinishedModels.size() > 0) {
        m_DeferredLoading = [=]() { Collision::DeferredLoading::load_2(this, btCompound, unfinishedModels, mass, type); };
        registerEvent(EventType::MeshLoaded);
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
    //destructor
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
void Collision::setMass(const float _mass) {
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
                        child_shape->calculateLocalInertia(_mass, m_BtInertia);
                    }
                }
            }
        }    
        m_BtShape->calculateLocalInertia(_mass, m_BtInertia);
    }
}
const btVector3& Collision::getBtInertia() const {
    return m_BtInertia;
}
btCollisionShape* Collision::getBtShape() const { 
    return m_BtShape; 
}
const CollisionType::Type& Collision::getType() const {
    return m_Type; 
}

void Collision::onEvent(const Event& event) {
    if (event.type == EventType::MeshLoaded && m_DeferredMeshes.size() > 0) {
        auto& ev = event.eventMeshLoaded;
        auto it  = m_DeferredMeshes.begin();
        while (it != m_DeferredMeshes.end()) {
            Mesh& deferred_mesh = *(*it);
            if (ev.mesh == (*it) || (deferred_mesh == true && deferred_mesh.m_CollisionFactory)) {
                it = m_DeferredMeshes.erase(it);
            }else{
                ++it;
            }
        }
        if (m_DeferredMeshes.size() == 0) {
            m_DeferredLoading();
            unregisterEvent(EventType::MeshLoaded);
        }
    }
}
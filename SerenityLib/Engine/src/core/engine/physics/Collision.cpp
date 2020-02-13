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

void Collision::DeferredLoading::load_1(Collision& collision, const CollisionType::Type collisionType, Mesh* mesh, const float& mass) {
    collision.m_DeferredMeshes.clear();
    const auto scale = collision.m_OwnerBody.getScale();
    Physics::removeRigidBodyThreadSafe(collision.m_OwnerBody);
    collision.free_memory();

    collision.m_BtShape = InternalMeshPublicInterface::BuildCollision(mesh, collisionType);
    collision.m_OwnerBody.rebuildRigidBody(true, true);
    collision.m_OwnerBody.setScale(scale);
    auto* model = collision.m_OwnerBody.getOwner().getComponent<ComponentModel>();
    if (model)
        ComponentModel_Functions::CalculateRadius(*model);
}
void Collision::DeferredLoading::load_2(Collision& collision, btCompoundShape* btCompound, vector<ModelInstance*> instances, const float& mass, const CollisionType::Type collisionType) {
    btTransform localTransform;
    const auto scale = collision.m_OwnerBody.getScale();
    for (auto& instance : instances) {
        btCollisionShape* built_collision_shape = InternalMeshPublicInterface::BuildCollision(instance, collisionType, true);
        localTransform = btTransform(Math::glmToBTQuat(instance->orientation()), Math::btVectorFromGLM(instance->position()));
        built_collision_shape->setMargin(0.001f);
        built_collision_shape->calculateLocalInertia(mass, collision.m_BtInertia); //this is important
        btCompound->addChildShape(localTransform, built_collision_shape);
    }
    collision.m_OwnerBody.rebuildRigidBody(true, true);
    collision.m_OwnerBody.setScale(scale);

    auto* model = collision.m_OwnerBody.getOwner().getComponent<ComponentModel>();
    if (model)
        ComponentModel_Functions::CalculateRadius(*model);
}

void Collision::_baseInit(const CollisionType::Type _type, const float& _mass) {
    m_BtInertia = btVector3(0.0f, 0.0f, 0.0f);
    m_Type = _type;
    setMass(_mass);
}
Collision::Collision(ComponentBody& body) : m_OwnerBody(body){
    m_BtShape = nullptr;
    m_BtInertia = btVector3(0.0f, 0.0f, 0.0f);
    m_Type = CollisionType::None;
    setMass(0.0f);
}
Collision::Collision(ComponentBody& body, const CollisionType::Type _type, ModelInstance* modelInstance, const float& _mass) : m_OwnerBody(body) {
    m_BtShape = nullptr;
    if (modelInstance) {
        auto* mesh_ptr = modelInstance->mesh();
        if (*mesh_ptr == false || !mesh_ptr->m_CollisionFactory) {
            m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), _type);
            m_DeferredMeshes.push_back(mesh_ptr);
            m_DeferredLoading = [_type, this, _mass, mesh_ptr]() { Collision::DeferredLoading::load_1(*this, _type, mesh_ptr, _mass); };
            registerEvent(EventType::MeshLoaded);
        }else{
            m_BtShape = InternalMeshPublicInterface::BuildCollision(modelInstance, _type);
        }
    }else{
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), _type);
    }
    _baseInit(_type, _mass);
}
Collision::Collision(ComponentBody& body, const CollisionType::Type _type, Mesh& mesh, const float& _mass) : m_OwnerBody(body) {
    m_BtShape = nullptr;
    if (mesh == false || !mesh.m_CollisionFactory) {
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), _type);
        auto* mesh_ptr = &mesh;
        m_DeferredMeshes.push_back(mesh_ptr);
        m_DeferredLoading = [_type, this, _mass, mesh_ptr]() { Collision::DeferredLoading::load_1(*this, _type, mesh_ptr, _mass); };
        registerEvent(EventType::MeshLoaded);
    }else{
        m_BtShape = InternalMeshPublicInterface::BuildCollision(&mesh, _type);
    }
    _baseInit(_type, _mass);
}

Collision::Collision(ComponentBody& body, ComponentModel& _modelComponent, const float& _mass, const CollisionType::Type _type) : m_OwnerBody(body) {
    m_BtShape = nullptr;
    vector<ModelInstance*> modelInstances;
    for (size_t i = 0; i < _modelComponent.getNumModels(); ++i) {
        modelInstances.push_back(&_modelComponent.getModel(i));
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
            btCollisionShape* built_collision_shape = InternalMeshPublicInterface::BuildCollision(&instance, _type, true);
            localTransform = btTransform(Math::glmToBTQuat(instance.orientation()), Math::btVectorFromGLM(instance.position()));
            built_collision_shape->setMargin(0.001f);
            built_collision_shape->calculateLocalInertia(_mass, m_BtInertia); //this is important
            btCompound->addChildShape(localTransform, built_collision_shape);
        }
    }
    if (unfinishedModels.size() > 0) {
        m_DeferredLoading = [=]() { Collision::DeferredLoading::load_2(*this, btCompound, unfinishedModels, _mass, _type); };
        registerEvent(EventType::MeshLoaded);
    }
    btCompound->setMargin(0.001f);
    btCompound->recalculateLocalAabb();
    btCompound->setUserPointer(&body);
    free_memory();
    m_BtShape = btCompound;
    setMass(_mass);
}
void Collision::free_memory() {
    auto* compound = dynamic_cast<btCompoundShape*>(m_BtShape);
    if (compound) {
        const auto numChildren = compound->getNumChildShapes();
        if (numChildren > 0) {
            for (int i = 0; i < numChildren; ++i) {
                auto* child_shape = compound->getChildShape(i);
                if (child_shape)
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
Collision::Collision(const Collision& other) : m_OwnerBody(other.m_OwnerBody) {
    //copy constructor
    m_BtInertia = other.m_BtInertia;
    m_Type      = other.m_Type;
    m_BtShape   = other.m_BtShape;
}
Collision& Collision::operator=(const Collision& other) {
    //copy assignment
    m_BtInertia = other.m_BtInertia;
    m_Type      = other.m_Type;
    m_BtShape   = other.m_BtShape;
    return *this;
}
Collision::Collision(Collision&& other) noexcept : m_OwnerBody(other.m_OwnerBody) {
    //move constructor
    m_BtInertia = std::move(other.m_BtInertia);
    m_Type      = std::move(other.m_Type);
    m_BtShape   = std::exchange(other.m_BtShape, nullptr);
}
Collision& Collision::operator=(Collision&& other) noexcept {
    //move assignment
    m_BtInertia = std::move(other.m_BtInertia);
    m_Type      = std::move(other.m_Type);
    m_BtShape   = std::exchange(other.m_BtShape, nullptr);
    return *this;
}
void Collision::setMass(const float _mass) {
    if (!m_BtShape || m_Type == CollisionType::TriangleShapeStatic || m_Type == CollisionType::None)
        return;
    if (m_BtShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
        /*
        auto* compound = dynamic_cast<btCompoundShape*>(m_BtShape);
        if (compound) {
            const auto numChildren = compound->getNumChildShapes();
            if (numChildren > 0) {
                for (int i = 0; i < numChildren; ++i) {
                    auto* child_shape = compound->getChildShape(i);
                    if(child_shape)
                        child_shape->calculateLocalInertia(_mass, m_BtInertia);
                }
            }
        }
        */
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

void Collision::onEvent(const Event& _event) {
    if (_event.type == EventType::MeshLoaded && m_DeferredMeshes.size() > 0) {
        auto& ev = _event.eventMeshLoaded;
        auto it = m_DeferredMeshes.begin();
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
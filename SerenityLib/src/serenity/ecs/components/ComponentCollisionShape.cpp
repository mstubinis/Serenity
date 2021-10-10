#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <BulletCollision/CollisionShapes/btEmptyShape.h>
#include <serenity/physics/PhysicsModule.h>

#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/BuiltInMeshes.h>
#include <serenity/system/Engine.h>

namespace {
    struct SingleData final {
        Handle         mesh;
        float          mass;
        CollisionType  collisionType;

        SingleData(Handle inMesh, float inMass, CollisionType inCollisionType)
            : mesh{ inMesh }
            , mass{ inMass }
            , collisionType{ inCollisionType }
        {}
    };
    struct MultiData final {
        std::vector<ModelInstance*>   instances;
        float                         mass;
        CollisionType                 collisionType;

        MultiData(std::vector<ModelInstance*>& inInstances, float inMass, CollisionType inCollisionType)
            : instances{ inInstances }
            , mass{ inMass }
            , collisionType{ inCollisionType }
        {}
    };

    std::unordered_map<Entity, SingleData>     DeferredLoadingCollisionsSingle;
    std::unordered_map<Entity, MultiData>      DeferredLoadingCollisionsMulti;

    void internal_cleanup_single_using_entity(Entity entity) {
        auto itrSingle = DeferredLoadingCollisionsSingle.begin();
        while (itrSingle != DeferredLoadingCollisionsSingle.end()) {
            if ((*itrSingle).first == entity) {
                itrSingle = DeferredLoadingCollisionsSingle.erase(itrSingle);
            } else {
                itrSingle++;
            }
        }
    }
    void internal_cleanup_multi_using_entity(Entity entity) {
        auto itrMulti = DeferredLoadingCollisionsMulti.begin();
        while (itrMulti != DeferredLoadingCollisionsMulti.end()) {
            if ((*itrMulti).first == entity) {
                itrMulti = DeferredLoadingCollisionsMulti.erase(itrMulti);
            } else {
                itrMulti++;
            }
        }
    }
    void deferred_Load_Single(Entity owner, Handle mesh, float mass, CollisionType collisionType) {
        if (!DeferredLoadingCollisionsSingle.contains(owner)) {
            DeferredLoadingCollisionsSingle.emplace(std::piecewise_construct, std::forward_as_tuple(owner), std::forward_as_tuple(mesh, mass, collisionType));
        }
    }
    void deferred_Load_Multiple(Entity owner, std::vector<ModelInstance*>& instances, float mass, CollisionType collisionType) {
        if (!DeferredLoadingCollisionsMulti.contains(owner)) {
            DeferredLoadingCollisionsMulti.emplace(std::piecewise_construct, std::forward_as_tuple(owner), std::forward_as_tuple(instances, mass, collisionType));
        }
    }
    void internal_load_single_mesh(Entity entity, CollisionType collisionType, Handle mesh, float mass) {
        auto collision = entity.getComponent<ComponentCollisionShape>();
        if (collision) {
            Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_single_mesh_impl(*collision, collisionType, mesh, mass);
        }
    }
    void internal_load_multiple_meshes(Entity entity, std::vector<ModelInstance*>& instances, float mass, CollisionType collisionType) {
        auto collision = entity.getComponent<ComponentCollisionShape>();
        if (collision) {
            Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_multiple_meshes_impl(*collision, instances, mass, collisionType);
        }
    }
}


ComponentCollisionShape::ComponentCollisionShape(Entity entity, CollisionType collisionType) 
    : m_Owner{ entity }
{
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    auto model     = m_Owner.getComponent<ComponentModel>();
    if (model) {
        if (model->getNumModels() >= 2) {
            setCollision(collisionType, *model, 0.0f);
        } else {
            setCollision(collisionType, &model->getModel(0), 0.0f);
        }
        if (rigidBody && !rigidBody->getBtBody()) {
            rigidBody->rebuildRigidBody(false);
        }
        internal_update_ptrs();
    }
}
ComponentCollisionShape::ComponentCollisionShape(Entity entity, CollisionType collisionType, Handle collisionMesh, float mass)
    : m_Owner{ entity }
{
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    setCollision(collisionType, collisionMesh, mass);
    if (rigidBody && !rigidBody->getBtBody()) {
        rigidBody->rebuildRigidBody(false);
    }
    internal_update_ptrs();
}
ComponentCollisionShape::ComponentCollisionShape(Entity entity, CollisionType collisionType, ComponentModel& model, float mass) 
    : m_Owner{ entity }
{
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    if (model.getNumModels() >= 2) {
        setCollision(collisionType, model, mass);
    } else {
        setCollision(collisionType, &model.getModel(0), mass);
    }
    if (rigidBody && !rigidBody->getBtBody()) {
        rigidBody->rebuildRigidBody(false);
    }
    internal_update_ptrs();
}


ComponentCollisionShape::~ComponentCollisionShape() {
    internal_free_memory();
}
ComponentCollisionShape::ComponentCollisionShape(ComponentCollisionShape&& other) noexcept
    : m_BtInertia       { std::move(other.m_BtInertia) }
    , m_ParentCompound  { std::move(other.m_ParentCompound) }
    , m_BtCollisionShape{ std::exchange(other.m_BtCollisionShape, nullptr) }
    , m_Owner           { std::exchange(other.m_Owner, Entity{}) }
{
    internal_update_ptrs();
}
ComponentCollisionShape& ComponentCollisionShape::operator=(ComponentCollisionShape&& other) noexcept {
    m_BtInertia        = std::move(other.m_BtInertia);
    m_ParentCompound   = std::move(other.m_ParentCompound);
    m_BtCollisionShape = std::exchange(other.m_BtCollisionShape, nullptr);
    m_Owner            = std::exchange(other.m_Owner, Entity{});

    internal_update_ptrs();
    return *this;
}
void ComponentCollisionShape::internal_free_memory() {
    if (m_BtCollisionShape){
        if (m_BtCollisionShape->isCompound()) {
            auto btCompound = (btCompoundShape*)m_BtCollisionShape;
            for (int i = 0; i < btCompound->getNumChildShapes(); ++i) {
                auto btChildShape = btCompound->getChildShape(i);
                if (btChildShape->getUserIndex2() == std::numeric_limits<int>().min()) {
                    SAFE_DELETE(btChildShape);
                }
            }
        }
        SAFE_DELETE(m_BtCollisionShape);
    }
}
void* ComponentCollisionShape::getCollisionUserPointer() const noexcept { 
    return m_BtCollisionShape->getUserPointer();
}
int ComponentCollisionShape::getCollisionUserIndex() const noexcept {
    return m_BtCollisionShape->getUserIndex();
}
int ComponentCollisionShape::getCollisionUserIndex2() const noexcept { 
    return m_BtCollisionShape->getUserIndex2();
}
void ComponentCollisionShape::setCollisionUserPointer(void* ptr) const noexcept { 
    m_BtCollisionShape->setUserPointer(ptr);
}
void ComponentCollisionShape::setCollisionUserIndex(int idx) const noexcept { 
    m_BtCollisionShape->setUserIndex(idx);
}
void ComponentCollisionShape::setCollisionUserIndex2(int idx) const noexcept { 
    m_BtCollisionShape->setUserIndex2(idx);
}
CollisionType ComponentCollisionShape::getType() const noexcept {
    return m_BtCollisionShape->getShapeType(); 
}
void ComponentCollisionShape::internal_update_ptrs() {
    if (m_Owner.null()) {
        return;
    }
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    if (rigidBody) {
        rigidBody->getBtBody()->setCollisionShape(m_BtCollisionShape);
        //Engine::Physics::cleanProxyFromPairs(rigidBody->getBtBody());
    }
}
void ComponentCollisionShape::setMargin(float margin) noexcept {
    ASSERT(margin >= 0.0f, __FUNCTION__ << "(): margin was negative!");
    m_BtCollisionShape->setMargin(margin);
}
void ComponentCollisionShape::calculateLocalInertia(float mass) noexcept {
    setMass(mass);
}
bool ComponentCollisionShape::promoteToCompoundShape(CollisionType collisionType) {
    if (getType() == CollisionType::COMPOUND_SHAPE_PROXYTYPE) {
        return false;
    }
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    auto oldShape  = m_BtCollisionShape;
    //m_BtCollisionShape.release();
    auto btCompound = new btCompoundShape{};
    btTransform tr;
    tr.setIdentity();
    btCompound->addChildShape(tr, oldShape);
    oldShape->setUserIndex2(std::numeric_limits<int>().min());
    m_BtCollisionShape = btCompound;
    internal_update_ptrs();
    return (rigidBody != nullptr);
}
bool ComponentCollisionShape::promoteToCompoundShape(CollisionType collisionType, const std::vector<Entity>& entities) {
    if (getType() == CollisionType::COMPOUND_SHAPE_PROXYTYPE) {
        return false;
    }
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    setCollision(collisionType, entities, rigidBody ? rigidBody->getMass() : 0.0f);
    internal_update_ptrs();
    return (entities.size() >= 2 && rigidBody != nullptr);
}
bool ComponentCollisionShape::addChildShape(ComponentCollisionShape& other) {
    if (getType() != CollisionType::COMPOUND_SHAPE_PROXYTYPE) {
        return false;
    }
    auto btCompound = (btCompoundShape*)m_BtCollisionShape;
    for (int i = 0; i < btCompound->getNumChildShapes(); ++i) {
        if (btCompound->getChildShape(i) == other.m_BtCollisionShape) {
            return false;
        }
    }
    btTransform tr;
    tr.setIdentity();
    auto transform = other.getOwner().getComponent<ComponentTransform>();
    if (transform) {
        tr.setOrigin(Engine::Math::toBT(transform->getLocalPosition()));
        tr.setRotation(Engine::Math::toBT(transform->getRotation()));
    }
    btCompound->addChildShape(tr, other.m_BtCollisionShape);
    btCompound->recalculateLocalAabb();
    btCompound->createAabbTreeFromChildren(); //TODO: is this needed?
    other.m_ParentCompound.compoundShape = btCompound;
    other.m_ParentCompound.index         = btCompound->getNumChildShapes() - 1;
    other.m_ParentCompound.parent        = m_Owner;
    internal_update_ptrs();
    other.internal_update_ptrs();
    return true;
}
bool ComponentCollisionShape::updateChildShapeTransform(const glm_mat4& transformMatrix) {
    auto parentShape = getParentCompoundShape();
    if (!parentShape) {
        return false;
    }
    btTransform tr;
    tr.setFromOpenGLMatrix(glm::value_ptr(transformMatrix));
    parentShape->updateChildTransform(getChildShapeIndex(), tr, true);
    return true;
}

void ComponentCollisionShape::setCollision(btCollisionShape* shape) {
    internal_free_memory();
    //SAFE_DELETE(m_BtCollisionShape);
    m_BtCollisionShape = shape;

    auto& deferredSystem = Engine::priv::ComponentCollisionShapeDeferredLoading::get();

    //we dont need the old collision that was going to be assigned to this entity once its loaded anymore since we are changing it
    internal_cleanup_single_using_entity(m_Owner);
    internal_cleanup_multi_using_entity(m_Owner);

    auto transform = m_Owner.getComponent<ComponentTransform>();
    //auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    auto model     = m_Owner.getComponent<ComponentModel>();
    //Engine::Physics::removeRigidBodyThreadSafe(rigidBody->getBtBody());
    //if (rigidBody) {
        //rigidBody->rebuildRigidBody(true);
    //}
    if (transform) {
        transform->setScale(transform->getScale());
    }
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    internal_update_ptrs();

}
void ComponentCollisionShape::setCollision(CollisionType collisionType, Handle meshHandle, float mass) {
    internal_free_memory();
    //SAFE_DELETE(m_BtCollisionShape);
    m_BtCollisionShape = nullptr;
    auto& mesh = *meshHandle.get<Mesh>();
    if (!mesh.isLoaded() /*|| !mesh.m_CPUData.m_CollisionFactory*/) {
        m_BtCollisionShape = Engine::priv::PublicMesh::BuildCollision(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), collisionType);
        deferred_Load_Single(m_Owner, meshHandle, mass, collisionType);
    } else {
        Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_single_mesh_impl(*this, collisionType, meshHandle, mass);
    }
}
void ComponentCollisionShape::setCollision(CollisionType collisionType, ModelInstance* modelInstance, float mass) {
    ComponentCollisionShape::setCollision(collisionType, modelInstance->getMesh(), mass);
}
void ComponentCollisionShape::setCollision(CollisionType collisionType, ComponentModel& componentModel, float mass) {
    if (componentModel.getNumModels() >= 2) {
        internal_free_memory();
        //SAFE_DELETE(m_BtCollisionShape);
        m_BtCollisionShape = new btCompoundShape{};
        std::vector<ModelInstance*> instances = Engine::create_and_reserve<std::vector<ModelInstance*>>(componentModel.getNumModels());
        bool allLoaded = true;
        for (int i = 0; i < componentModel.getNumModels(); ++i) {
            auto& instance = componentModel.getModel(i);
            if (!instance.getMesh().get<Mesh>()->isLoaded()) {
                allLoaded = false;
            }
            instances.push_back(&instance);
        }
        if (!allLoaded) {
            deferred_Load_Multiple(m_Owner, instances, mass, collisionType);
        } else {
            Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_multiple_meshes_impl(*this, instances, mass, collisionType);
        }
    } else {
        setCollision(collisionType, &componentModel.getModel(0), mass);
    }
}
void ComponentCollisionShape::setCollision(CollisionType collisionType, const std::vector<Entity>& entities, float mass) {
    std::vector<ModelInstance*> instances = Engine::create_and_reserve<std::vector<ModelInstance*>>(entities.size());
    bool allLoaded = true;
    for (const auto entity : entities) {
        auto componentModel = entity.getComponent<ComponentModel>();
        if (componentModel) {
            for (int i = 0; i < componentModel->getNumModels(); ++i) {
                auto& instance = componentModel->getModel(i);
                if (!instance.getMesh().get<Mesh>()->isLoaded()) {
                    allLoaded = false;
                }
                instances.push_back(&instance);
            }
        }
    }
    if (instances.size() >= 2) {
        internal_free_memory();
        //SAFE_DELETE(m_BtCollisionShape);
        m_BtCollisionShape = new btCompoundShape{};
        if (!allLoaded) {
            deferred_Load_Multiple(m_Owner, instances, mass, collisionType);
        } else {
            Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_multiple_meshes_impl(*this, instances, mass, collisionType);
        }
    } else {
        setCollision(collisionType, instances[0], mass);
    }
}
bool ComponentCollisionShape::isStaticTriangleType() const noexcept {
    CollisionType type = getType();
    return (type == CollisionType::TRIANGLE_MESH_SHAPE_PROXYTYPE ||
        type == CollisionType::SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE ||
        type == CollisionType::TRIANGLE_SHAPE_PROXYTYPE);
}
void ComponentCollisionShape::forcePhysicsSync() noexcept {
    auto transform = m_Owner.getComponent<ComponentTransform>();
    if (transform) {
        auto scl = transform->getScale();
        internal_setScale(scl.x, scl.y, scl.z);
    }
}
void ComponentCollisionShape::internal_setScale(float x, float y, float z) {
    auto model      = m_Owner.getComponent<ComponentModel>();
    //auto rigidBody  = m_Owner.getComponent<ComponentRigidBody>();
    if (m_BtCollisionShape) {
        m_BtCollisionShape->setLocalScaling(btVector3{ btScalar(x), btScalar(y), btScalar(z) });
    }
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    //if (rigidBody) {
    //    Engine::Physics::cleanProxyFromPairs(rigidBody->getBtBody());
    //}
}
void ComponentCollisionShape::setMass(float mass) {
    if (!m_BtCollisionShape || isStaticTriangleType() || getType() == CollisionType::INVALID_SHAPE_PROXYTYPE || getType() == CollisionType::EMPTY_SHAPE_PROXYTYPE) {
        return;
    }
    if (m_BtCollisionShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
        if (m_BtCollisionShape->isCompound()) {
            btCompoundShape* btCompound = (btCompoundShape*)m_BtCollisionShape;
            for (int i = 0; i < btCompound->getNumChildShapes(); ++i) {
                btCollisionShape* btChildShape = btCompound->getChildShape(i);
                if (btChildShape) {
                    btChildShape->calculateLocalInertia(mass, m_BtInertia);
                }
            }
        }
        m_BtCollisionShape->calculateLocalInertia(mass, m_BtInertia);
    }
}
void Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_single_mesh_impl(ComponentCollisionShape& collisionShape, CollisionType collisionType, Handle meshHandle, float mass) {
    auto transform = collisionShape.m_Owner.getComponent<ComponentTransform>();
    //auto rigidBody = collisionShape.m_Owner.getComponent<ComponentRigidBody>();
    auto model     = collisionShape.m_Owner.getComponent<ComponentModel>();
    //Engine::Physics::removeRigidBodyThreadSafe(rigidBody->getBtBody());
    collisionShape.internal_free_memory();
    SAFE_DELETE(collisionShape.m_BtCollisionShape);
    collisionShape.m_BtCollisionShape = Engine::priv::PublicMesh::BuildCollision(meshHandle, collisionType);
    //if (rigidBody) {
        //rigidBody->rebuildRigidBody(true);
        //rigidBody->getBtBody()->setCollisionShape(collisionShape.m_BtCollisionShape); //TODO: do we need this? (internal_update_ptrs() does this)
    //}
    if (transform) {
        transform->setScale(transform->getScale());
    }
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    collisionShape.internal_update_ptrs();
    collisionShape.calculateLocalInertia(mass);
}
void Engine::priv::ComponentCollisionShapeDeferredLoading::internal_load_multiple_meshes_impl(ComponentCollisionShape& collisionShape, std::vector<ModelInstance*>& instances, float mass, CollisionType collisionType) {
    auto transform  = collisionShape.m_Owner.getComponent<ComponentTransform>();
    auto rigidBody  = collisionShape.m_Owner.getComponent<ComponentRigidBody>();
    auto model      = collisionShape.m_Owner.getComponent<ComponentModel>();
    auto scale      = transform->getScale();
    auto btCompound = (btCompoundShape*)collisionShape.getBtShape();
    for (auto& instance : instances) {
        ASSERT(instance, __FUNCTION__ << "(): instance in instances was null!");
        auto transformChild = instance->getParent().getComponent<ComponentTransform>();
        btCollisionShape* newBtShape = Engine::priv::PublicMesh::BuildCollision(instance, collisionType, true);
        btTransform localTransform;
        if (transformChild && &(*transformChild) != &(*transform)) {
            auto rot = transformChild->getRotation() * instance->getRotation();
            auto pos = transformChild->getLocalPosition() * instance->getPosition();
            localTransform = btTransform{ Engine::Math::toBT(rot), Engine::Math::toBT(pos) };
        } else {
            localTransform = btTransform{ Engine::Math::toBT(instance->getRotation()), Engine::Math::toBT(instance->getPosition()) };
        }
        newBtShape->setMargin(0.04f);
        newBtShape->calculateLocalInertia(mass, collisionShape.m_BtInertia); //this is important
        newBtShape->setUserIndex2(std::numeric_limits<int>().min()); //this is important for cleanup
        btCompound->addChildShape(localTransform, newBtShape);

    }
    if (rigidBody) {
        rigidBody->rebuildRigidBody(true);
    }
    transform->setScale(scale);
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    collisionShape.calculateLocalInertia(mass);
    collisionShape.internal_update_ptrs();
}

void Engine::priv::ComponentCollisionShapeDeferredLoading::onEvent(const Event& e) {
    if (e.type == EventType::ResourceLoaded && e.eventResource.resource->type() == ResourceType::Mesh) {
        Mesh* loadedMesh = static_cast<Mesh*>(e.eventResource.resource);

        auto itrSingle = DeferredLoadingCollisionsSingle.begin();
        while (itrSingle != DeferredLoadingCollisionsSingle.end()) {
            auto& data = *itrSingle;
            Handle meshHandle = data.second.mesh;
            Mesh* mesh        = meshHandle.get<Mesh>();
            if (mesh == loadedMesh || mesh->isLoaded()) {
                internal_load_single_mesh(data.first, data.second.collisionType, meshHandle, data.second.mass);
                itrSingle = DeferredLoadingCollisionsSingle.erase(itrSingle);
            } else {
                itrSingle++;
            }
        }


        auto itrMulti = DeferredLoadingCollisionsMulti.begin();
        while (itrMulti != DeferredLoadingCollisionsMulti.end()) {
            uint32_t counter = 0;
            auto& data       = *itrMulti;
            auto& instances  = data.second.instances;
            for (auto& modelInstance : instances) {
                Mesh* modelInstanceMesh = modelInstance->getMesh().get<Mesh>();
                if (modelInstanceMesh == loadedMesh || modelInstanceMesh->isLoaded()) {
                    counter++;
                }
            }
            if (counter == instances.size()) {
                internal_load_multiple_meshes(data.first, instances, data.second.mass, data.second.collisionType);
                itrMulti = DeferredLoadingCollisionsMulti.erase(itrMulti);
            } else {
                itrMulti++;
            }
        }
    }
}
#include <serenity/ecs/components/ComponentCollisionShape.h>
#include <serenity/ecs/components/ComponentRigidBody.h>
#include <serenity/ecs/components/ComponentTransform.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <BulletCollision/CollisionShapes/btCompoundShape.h>
#include <serenity/physics/PhysicsModule.h>

#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/mesh/BuiltInMeshes.h>
#include <serenity/system/Engine.h>

ComponentCollisionShape::ComponentCollisionShape(Entity entity, CollisionType collisionType) 
    : m_Owner{ entity }
{
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    auto model     = m_Owner.getComponent<ComponentModel>();
    if (model) {
        if (model->getNumModels() >= 2) {
            setCollision(collisionType, *model, 0.0f);
        }else{
            setCollision(collisionType, &model->getModel(0), 0.0f);
        }
        if (rigidBody && !rigidBody->getBtBody()) {
            rigidBody->rebuildRigidBody(false);
        }
        internal_update_ptrs();
    }
}
ComponentCollisionShape::~ComponentCollisionShape() {
    internal_free_memory();
}
ComponentCollisionShape::ComponentCollisionShape(ComponentCollisionShape&& other) noexcept
    : m_BtInertia      { std::move(other.m_BtInertia) }
    , m_ParentCompound { std::move(other.m_ParentCompound) }
    , m_CollisionShape { std::move(other.m_CollisionShape) }
    , m_Owner          { std::exchange(other.m_Owner, Entity{}) }
{
    internal_update_ptrs();
}
ComponentCollisionShape& ComponentCollisionShape::operator=(ComponentCollisionShape&& other) noexcept {
    m_BtInertia      = std::move(other.m_BtInertia);
    m_ParentCompound = std::move(other.m_ParentCompound);
    m_CollisionShape = std::move(other.m_CollisionShape);
    m_Owner          = std::exchange(other.m_Owner, Entity{});

    internal_update_ptrs();
    return *this;
}
void ComponentCollisionShape::internal_free_memory() {
    if (m_CollisionShape){
        if (m_CollisionShape->isCompound()) {
            auto compound = static_cast<btCompoundShape*>(m_CollisionShape.get());
            for (int i = 0; i < compound->getNumChildShapes(); ++i) {
                auto child_shape = compound->getChildShape(i);
                if (child_shape->getUserIndex2() == std::numeric_limits<int>().min()) {
                    SAFE_DELETE(child_shape);
                }
            }
        }
        m_CollisionShape.reset(nullptr);
    }
}
void ComponentCollisionShape::internal_update_ptrs() {
    if (!m_Owner) {
        return;
    }
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    if (rigidBody) {
        rigidBody->getBtBody()->setCollisionShape(m_CollisionShape.get());
        //Engine::Physics::cleanProxyFromPairs(rigidBody->getBtBody());
    }
}
void ComponentCollisionShape::calculateLocalInertia(float mass) noexcept {
    setMass(mass);
}
bool ComponentCollisionShape::promoteToCompoundShape(CollisionType collisionType) {
    if (getType() == CollisionType::COMPOUND_SHAPE_PROXYTYPE) {
        return false;
    }
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    auto oldShape  = m_CollisionShape.get();
    m_CollisionShape.release();
    auto compound  = new btCompoundShape{};
    btTransform tr;
    tr.setIdentity();
    compound->addChildShape(tr, oldShape);
    oldShape->setUserIndex2(std::numeric_limits<int>().min());
    m_CollisionShape.reset(compound);
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
    auto compound = static_cast<btCompoundShape*>(m_CollisionShape.get());
    for (int i = 0; i < compound->getNumChildShapes(); ++i) {
        if (compound->getChildShape(i) == other.m_CollisionShape.get()) {
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
    compound->addChildShape(tr, other.m_CollisionShape.get());
    compound->recalculateLocalAabb();
    compound->createAabbTreeFromChildren(); //TODO: is this needed?
    std::get<0>(other.m_ParentCompound) = compound;
    std::get<1>(other.m_ParentCompound) = compound->getNumChildShapes() - 1;
    std::get<2>(other.m_ParentCompound) = m_Owner;
    internal_update_ptrs();
    other.internal_update_ptrs();
    return true;
}
void ComponentCollisionShape::setCollision(btCollisionShape* shape) {
    internal_free_memory();
    m_CollisionShape.reset(shape);

    auto& deferredSystem = Engine::priv::ComponentCollisionShapeDeferredLoading::get();
    {
        auto itrSingle = deferredSystem.m_DeferredLoadingCollisionsSingle.begin();
        while (itrSingle != deferredSystem.m_DeferredLoadingCollisionsSingle.end()) {
            if ((*itrSingle).first == m_Owner) {
                itrSingle = deferredSystem.m_DeferredLoadingCollisionsSingle.erase(itrSingle);
            }else{
                itrSingle++;
            }
        }
    }
    {
        auto itrMulti = deferredSystem.m_DeferredLoadingCollisionsMulti.begin();
        while (itrMulti != deferredSystem.m_DeferredLoadingCollisionsMulti.end()) {
            if ((*itrMulti).first == m_Owner) {
                itrMulti = deferredSystem.m_DeferredLoadingCollisionsMulti.erase(itrMulti);
            }else{
                itrMulti++;
            }
        }
    }
    auto transform = m_Owner.getComponent<ComponentTransform>();
    auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    auto model     = m_Owner.getComponent<ComponentModel>();
    //Engine::Physics::removeRigidBodyThreadSafe(rigidBody->getBtBody());
    if (rigidBody) {
        //rigidBody->rebuildRigidBody(true);
    }
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
    m_CollisionShape.reset(nullptr);
    auto& mesh = *meshHandle.get<Mesh>();
    if (!mesh.isLoaded() /*|| !mesh.m_CPUData.m_CollisionFactory*/) {
        m_CollisionShape = std::unique_ptr<btCollisionShape>(Engine::priv::PublicMesh::BuildCollision(Engine::priv::Core::m_Engine->m_Misc.m_BuiltInMeshes.getCubeMesh(), collisionType));
        Engine::priv::ComponentCollisionShapeDeferredLoading::get().deferredLoadSingle(m_Owner, meshHandle, mass, collisionType);
    }else{
        ComponentCollisionShape::internal_load_single_mesh_impl(*this, collisionType, meshHandle, mass);
    }
}
void ComponentCollisionShape::setCollision(CollisionType collisionType, ModelInstance* modelInstance, float mass) {
    ComponentCollisionShape::setCollision(collisionType, modelInstance->getMesh(), mass);
}
void ComponentCollisionShape::setCollision(CollisionType collisionType, ComponentModel& componentModel, float mass) {
    if (componentModel.getNumModels() >= 2) {
        internal_free_memory();
        m_CollisionShape.reset(new btCompoundShape{});
        std::vector<ModelInstance*> instances = Engine::create_and_reserve<std::vector<ModelInstance*>>((uint32_t)componentModel.getNumModels());
        bool allLoaded = true;
        for (int i = 0; i < componentModel.getNumModels(); ++i) {
            auto& instance = componentModel.getModel(i);
            if (!instance.getMesh().get<Mesh>()->isLoaded()) {
                allLoaded = false;
            }
            instances.push_back(&instance);
        }
        if (!allLoaded) {
            Engine::priv::ComponentCollisionShapeDeferredLoading::get().deferredLoadMultiple(m_Owner, instances, mass, collisionType);
        }else{
            ComponentCollisionShape::internal_load_multiple_meshes_impl(*this, instances, mass, collisionType);
        }
    }else{
        ComponentCollisionShape::setCollision(collisionType, &componentModel.getModel(0), mass);
    }
}
void ComponentCollisionShape::setCollision(CollisionType collisionType, const std::vector<Entity>& entities, float mass) {
    std::vector<ModelInstance*> instances = Engine::create_and_reserve<std::vector<ModelInstance*>>((uint32_t)entities.size());
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
        m_CollisionShape.reset(new btCompoundShape{});
        if (!allLoaded) {
            Engine::priv::ComponentCollisionShapeDeferredLoading::get().deferredLoadMultiple(m_Owner, instances, mass, collisionType);
        }else{
            ComponentCollisionShape::internal_load_multiple_meshes_impl(*this, instances, mass, collisionType);
        }
    }else{
        ComponentCollisionShape::setCollision(collisionType, instances[0], mass);
    }
}


void ComponentCollisionShape::forcePhysicsSync() noexcept {
    auto transform = m_Owner.getComponent<ComponentTransform>();
    if (transform) {
        auto scl = transform->getScale();
        internal_setScale(scl.x, scl.y, scl.z);
    }
}
void ComponentCollisionShape::internal_setScale(float x, float y, float z) {
    auto btColShape = m_CollisionShape.get();
    auto model = m_Owner.getComponent<ComponentModel>();
    //auto rigidBody = m_Owner.getComponent<ComponentRigidBody>();
    if (btColShape) {
        btColShape->setLocalScaling(btVector3{ (btScalar)x, (btScalar)y, (btScalar)z });
    }
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    //if (rigidBody) {
    //    Engine::Physics::cleanProxyFromPairs(rigidBody->getBtBody());
    //}
}

void ComponentCollisionShape::setMass(float mass) {
    auto btShape = getBtShape();
    if (!btShape || isStaticTriangleType() || getType() == CollisionType::INVALID_SHAPE_PROXYTYPE || getType() == CollisionType::EMPTY_SHAPE_PROXYTYPE) {
        return;
    }
    if (btShape->getShapeType() != EMPTY_SHAPE_PROXYTYPE) {
        if (btShape->isCompound()) {
            btCompoundShape* compound = static_cast<btCompoundShape*>(btShape);
            for (int i = 0; i < compound->getNumChildShapes(); ++i) {
                btCollisionShape* child_shape = compound->getChildShape(i);
                if (child_shape) {
                    child_shape->calculateLocalInertia(mass, m_BtInertia);
                }
            }
        }
        btShape->calculateLocalInertia(mass, m_BtInertia);
    }
}



void ComponentCollisionShape::internal_load_single_mesh(Entity entity, CollisionType collisionType, Handle mesh, float mass) {
    auto collision = entity.getComponent<ComponentCollisionShape>();
    if (collision) {
        ComponentCollisionShape::internal_load_single_mesh_impl(*collision, collisionType, mesh, mass);
    }
}
void ComponentCollisionShape::internal_load_multiple_meshes(Entity entity, std::vector<ModelInstance*>& instances, float mass, CollisionType collisionType) {
    auto collision = entity.getComponent<ComponentCollisionShape>();
    if (collision) {
        ComponentCollisionShape::internal_load_multiple_meshes_impl(*collision, instances, mass, collisionType);
    }
}
void ComponentCollisionShape::internal_load_single_mesh_impl(ComponentCollisionShape& collisionShape, CollisionType collisionType, Handle meshHandle, float mass) {
    auto transform = collisionShape.m_Owner.getComponent<ComponentTransform>();
    auto rigidBody = collisionShape.m_Owner.getComponent<ComponentRigidBody>();
    auto model     = collisionShape.m_Owner.getComponent<ComponentModel>();
    //Engine::Physics::removeRigidBodyThreadSafe(rigidBody->getBtBody());
    collisionShape.internal_free_memory();
    collisionShape.m_CollisionShape = std::unique_ptr<btCollisionShape>(Engine::priv::PublicMesh::BuildCollision(meshHandle, collisionType));
    if (rigidBody) {
        //rigidBody->rebuildRigidBody(true);
        rigidBody->getBtBody()->setCollisionShape(collisionShape.m_CollisionShape.get()); //TODO: do we need this? (internal_update_ptrs() does this)
    }
    if (transform) {
        transform->setScale(transform->getScale());
    }
    if (model) {
        Engine::priv::ComponentModel_Functions::CalculateRadius(*model);
    }
    collisionShape.calculateLocalInertia(mass);
    collisionShape.internal_update_ptrs();
}
void ComponentCollisionShape::internal_load_multiple_meshes_impl(ComponentCollisionShape& collisionShape, std::vector<ModelInstance*>& instances, float mass, CollisionType collisionType) {
    auto transform  = collisionShape.m_Owner.getComponent<ComponentTransform>();
    auto rigidBody  = collisionShape.m_Owner.getComponent<ComponentRigidBody>();
    auto model      = collisionShape.m_Owner.getComponent<ComponentModel>();
    auto scale      = transform->getScale();
    auto btCompound = static_cast<btCompoundShape*>(collisionShape.getBtShape());
    for (auto& instance : instances) {
        ASSERT(instance, __FUNCTION__ << "(): instance in instances was null!");
        auto transformChild = instance->getParent().getComponent<ComponentTransform>();
        btCollisionShape* built_collision_shape = Engine::priv::PublicMesh::BuildCollision(instance, collisionType, true);
        btTransform localTransform;
        if (transformChild && &(*transformChild) != &(*transform)) {
            auto rot = transformChild->getRotation() * instance->getRotation();
            auto pos = transformChild->getLocalPosition() * instance->getPosition();
            localTransform = btTransform{ Engine::Math::toBT(rot), Engine::Math::toBT(pos) };
        }else{
            localTransform = btTransform{ Engine::Math::toBT(instance->getRotation()), Engine::Math::toBT(instance->getPosition()) };
        }
        built_collision_shape->setMargin(0.04f);
        built_collision_shape->calculateLocalInertia(mass, collisionShape.m_BtInertia); //this is important
        built_collision_shape->setUserIndex2(std::numeric_limits<int>().min()); //this is important for cleanup
        btCompound->addChildShape(localTransform, built_collision_shape);

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

        auto itrSingle = m_DeferredLoadingCollisionsSingle.begin();
        while (itrSingle != m_DeferredLoadingCollisionsSingle.end()) {
            auto& data = *itrSingle;
            Handle meshHandle = std::get<0>(data.second);
            Mesh* mesh        = meshHandle.get<Mesh>();
            if (mesh == loadedMesh || mesh->isLoaded()) {
                ComponentCollisionShape::internal_load_single_mesh(data.first, std::get<2>(data.second), meshHandle, std::get<1>(data.second));
                itrSingle = m_DeferredLoadingCollisionsSingle.erase(itrSingle);
            }else{
                itrSingle++;
            }
        }


        auto itrMulti = m_DeferredLoadingCollisionsMulti.begin();
        while (itrMulti != m_DeferredLoadingCollisionsMulti.end()) {
            uint32_t counter = 0;
            auto& data       = *itrMulti;
            auto& instances  = std::get<0>(data.second);
            for (auto& modelInstance : instances) {
                Mesh* modelInstanceMesh = modelInstance->getMesh().get<Mesh>();
                if (modelInstanceMesh == loadedMesh || modelInstanceMesh->isLoaded()) {
                    counter++;
                }
            }
            if (counter == instances.size()) {
                ComponentCollisionShape::internal_load_multiple_meshes(data.first, instances, std::get<1>(data.second), std::get<2>(data.second));
                itrMulti = m_DeferredLoadingCollisionsMulti.erase(itrMulti);
            }else{
                itrMulti++;
            }
        }
    }
}
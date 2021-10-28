#include <serenity/ecs/systems/SystemBaseClass.h>
#include <serenity/ecs/ECS.h>
#include <serenity/utils/Utils.h>

struct entity_less final {
    inline bool operator()(Entity lhs, Entity rhs) const noexcept { 
        return lhs.id() < rhs.id(); 
    }
};

void SystemBaseClass::addEntity(Entity entity) noexcept {
    //ASSERT(!hasEntity(entity), __FUNCTION__ << "(): entity was already in m_Entities!");
    if (hasEntity(entity)) {
        return;
    }
    if (m_AssociatedComponents.size() >= 2) {
        for (const auto& [id, sparseSetPool] : m_AssociatedComponents) {
            if (!sparseSetPool->has(entity.id())) {
                return;
            }
        }
    }
    m_Entities.push_back(entity);
    sortEntities();
}
Entity SystemBaseClass::getEntity(uint32_t entityID) const noexcept {
    auto idx = Engine::binary_search(m_Entities, entityID);
    return idx != std::numeric_limits<size_t>().max() ? m_Entities[idx] : Entity{};
}
size_t SystemBaseClass::getEntityIdxInContainer(Entity entity) const noexcept {
    return Engine::binary_search(m_Entities, entity.id());
}
bool SystemBaseClass::eraseEntity(Entity entity) {
    size_t entityIndexInContainer = Engine::binary_search(m_Entities, entity.id());
    if (entityIndexInContainer != std::numeric_limits<size_t>().max()) {
        return Engine::swap_and_pop(m_Entities, entityIndexInContainer);
    }
    return false;
}
void SystemBaseClass::removeEntity(Entity entity) noexcept {
    bool erased = eraseEntity(entity);
    if (erased) {
        sortEntities();
    }
}

void SystemBaseClass::associateComponentImpl(uint32_t typeID) {
    m_AssociatedComponents.emplace_back(typeID, m_ECS.getComponentPool(typeID));
}

bool SystemBaseClass::hasEntity(Entity entity) const noexcept {
    return std::binary_search(std::begin(m_Entities), std::end(m_Entities), entity, entity_less{});
}
bool SystemBaseClass::hasAssociatedComponent(uint32_t typeID) noexcept {
    for (const auto& [id, sparseSetPool] : m_AssociatedComponents) {
        if (id == typeID) {
            return true;
        }
    }
    return false;
}
void SystemBaseClass::sortEntities() noexcept {
    //std::sort(std::execution::par_unseq, std::begin(m_Entities), std::end(m_Entities), entity_less{});
    Engine::insertion_sort(m_Entities);
}

void SystemBaseClass::onUpdate(const float dt, Scene& scene) noexcept { 
    m_UpdateFunction(*this, dt, scene); 
}
void SystemBaseClass::onComponentAddedToEntity(void* component, Entity entity) noexcept { 
    m_ComponentAddedToEntityFunction(*this, component, entity); 
}
void SystemBaseClass::onComponentRemovedFromEntity(Entity entity) noexcept { 
    m_ComponentRemovedFromEntityFunction(*this, entity); 
}
void SystemBaseClass::onEntityAddedToScene(Scene& scene, Entity entity) noexcept { 
    m_EntityToSceneFunction(*this, scene, entity); 
}
void SystemBaseClass::onSceneEntered(Scene& scene) noexcept { 
    m_SceneEnteredFunction(*this, scene); 
}
void SystemBaseClass::onSceneLeft(Scene& scene) noexcept { 
    m_SceneLeftFunction(*this, scene); 
}
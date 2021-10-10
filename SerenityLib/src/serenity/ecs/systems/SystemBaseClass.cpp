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
    auto idx = getEntityIdxInContainer(entityID);
    return idx != std::numeric_limits<size_t>().max() ? m_Entities[idx] : Entity{};
}
size_t SystemBaseClass::getEntityIdxInContainer(uint32_t entityID) const noexcept {
    //binary search
    using signed_size_t = std::make_signed_t<std::size_t>;
    signed_size_t left  = 0;
    signed_size_t right = static_cast<signed_size_t>(m_Entities.size()) - 1;
    while (left <= right) {
        const signed_size_t mid = left + (right - left) / 2;
        const auto itrID = m_Entities[mid].id();
        if (itrID == entityID) {
            return mid;
        } else if (itrID > entityID) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return std::numeric_limits<size_t>().max();
}
size_t SystemBaseClass::getEntityIdxInContainer(Entity entity) const noexcept {
    return getEntityIdxInContainer(entity.id());
}
bool SystemBaseClass::eraseEntity(std::vector<Entity>& entityVector, Entity entity) {
    size_t entityIndexInContainer = getEntityIdxInContainer(entity.id());
    if (entityIndexInContainer != std::numeric_limits<size_t>().max()) {
        auto backIdx = entityVector.size() - 1;
        if (entityIndexInContainer != backIdx) {
            std::swap(entityVector[entityIndexInContainer], entityVector[backIdx]);
        }
        entityVector.pop_back();
        return true;
    }
    return false;
}
void SystemBaseClass::removeEntity(Entity entity) noexcept {
    bool erased = eraseEntity(m_Entities, entity);
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
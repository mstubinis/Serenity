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
    if (m_Components.size() >= 2) {
        for (const auto& [id, pool] : m_Components) {
            if (!pool->has(entity.id())) {
                return;
            }
        }
    }
    m_Entities.push_back(entity);
    sortEntities();
}
Entity SystemBaseClass::getEntity(uint32_t entityID) const noexcept {
    //binary search
    int left = 0;
    int right = static_cast<int>(m_Entities.size()) - 1;
    while (left <= right) {
        const int mid = left + (right - left) / 2;
        if (m_Entities[mid].id() == entityID) {
            return m_Entities[mid];
        } else if (m_Entities[mid].id() > entityID) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return Entity{};
}
void SystemBaseClass::removeEntity(Entity entity) noexcept {
    eraseEntity(m_Entities, entity);
    sortEntities();
}

void SystemBaseClass::associateComponentImpl(uint32_t typeID) {
    m_Components.emplace_back(typeID, m_ECS.getComponentPool(typeID));
}

bool SystemBaseClass::hasEntity(Entity entity) const noexcept {
    return std::binary_search(std::begin(m_Entities), std::end(m_Entities), entity, entity_less{});
}
bool SystemBaseClass::hasAssociatedComponent(uint32_t typeID) noexcept {
    for (const auto& [id, pool] : m_Components) {
        if (id == typeID) {
            return true;
        }
    }
    return false;
}
void SystemBaseClass::eraseEntity(std::vector<Entity>& entityVector, Entity entity) {
    auto boundsIter = std::equal_range(std::begin(entityVector), std::end(entityVector), entity, entity_less{});
    auto lastIter   = entityVector.end() - std::distance(boundsIter.first, boundsIter.second);
    std::swap_ranges(boundsIter.first, boundsIter.second, lastIter);
    entityVector.erase(lastIter, std::end(entityVector));
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
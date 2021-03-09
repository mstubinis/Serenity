#include <serenity/ecs/systems/SystemBaseClass.h>
#include <serenity/ecs/ECS.h>

void SystemBaseClass::addEntity(Entity entity) noexcept {
    //ASSERT(!hasEntity(entity), __FUNCTION__ << "(): entity was already in m_Entities!");
    if (hasEntity(entity)) {
        return;
    }
    m_Entities.push_back(entity);
    sortEntities();
}
void SystemBaseClass::removeEntity(Entity entity) noexcept {
    eraseEntity(m_Entities, entity);
    sortEntities();
}

void SystemBaseClass::associateComponentImpl(uint32_t typeID) {
    m_Components.emplace_back(typeID, m_ECS.getComponentPool(typeID));
}

bool SystemBaseClass::hasEntity(Entity entity) const noexcept {
    return std::binary_search(std::begin(m_Entities), std::end(m_Entities), entity/*, entity_less{}*/);
}
bool SystemBaseClass::hasAssociatedComponent(uint32_t typeID) noexcept {
    for (const auto& [id, pool] : m_Components) {
        if (id == typeID) {
            return true;
        }
    }
    return false;
}
void SystemBaseClass::eraseEntity(std::vector<Entity>& vec, Entity entity) {
    auto bounds = std::equal_range(std::begin(vec), std::end(vec), entity/*, entity_less{}*/);
    auto last = vec.end() - std::distance(bounds.first, bounds.second);
    std::swap_ranges(bounds.first, bounds.second, last);
    vec.erase(last, std::end(vec));
}
void SystemBaseClass::insertionSort(std::vector<Entity>& container) noexcept {
    for (int i = 1; i < container.size(); i++) {
        auto key = container[i];
        int j = i - 1;
        while (j >= 0 && container[j] > key) {
            container[j + 1] = container[j];
            j = j - 1;
        }
        container[j + 1] = key;
    }
}
void SystemBaseClass::sortEntities() noexcept {
    //std::sort(std::execution::par_unseq, std::begin(m_Entities), std::end(m_Entities), entity_less{});
    insertionSort(m_Entities);
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
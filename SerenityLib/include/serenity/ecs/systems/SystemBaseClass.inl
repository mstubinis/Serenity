
template<class FUNC> void SystemBaseClass::setUpdateFunction(FUNC&& func) noexcept { 
    m_UpdateFunction = std::forward<FUNC>(func); 
}
template<class FUNC> void SystemBaseClass::setComponentAddedToEntityFunction(FUNC&& func) noexcept { 
    m_ComponentAddedToEntityFunction = std::forward<FUNC>(func); 
}
template<class FUNC> void SystemBaseClass::setComponentRemovedFromEntityFunction(FUNC&& func) noexcept { 
    m_ComponentRemovedFromEntityFunction = std::forward<FUNC>(func); 
}
template<class FUNC> void SystemBaseClass::setEntityAddedToSceneFunction(FUNC&& func) noexcept { 
    m_EntityToSceneFunction = std::forward<FUNC>(func); 
}
template<class FUNC> void SystemBaseClass::setSceneEnteredFunction(FUNC&& func) noexcept { 
    m_SceneEnteredFunction = std::forward<FUNC>(func); 
}
template<class FUNC> void SystemBaseClass::setSceneLeftFunction(FUNC&& func) noexcept { 
    m_SceneLeftFunction = std::forward<FUNC>(func); 
}

template<class COMPONENT>
void SystemBaseClass::associateComponent() {
    ASSERT(COMPONENT::TYPE_ID != std::numeric_limits<uint32_t>().max(), __FUNCTION__ << "(): COMPONENT::TYPE_ID was null! please register this component (component: " << typeid(COMPONENT).name());
    ASSERT(!hasAssociatedComponent<COMPONENT>(), __FUNCTION__ << "(): already associated component: " << typeid(COMPONENT).name());
    associateComponentImpl(COMPONENT::TYPE_ID);
}
template<class COMPONENT>
[[nodiscard]] bool SystemBaseClass::hasAssociatedComponent() noexcept {
    return hasAssociatedComponent(COMPONENT::TYPE_ID);
}
template<class COMPONENT>
[[nodiscard]] Engine::priv::ECSComponentPool<COMPONENT>& SystemBaseClass::getComponentPool(uint32_t index) noexcept {
    return *static_cast<Engine::priv::ECSComponentPool<COMPONENT>*>(m_AssociatedComponents[index].second);
}
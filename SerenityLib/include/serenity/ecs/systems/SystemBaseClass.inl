
template<class FUNC> void SystemBaseClass::setUpdateFunction(FUNC&& func) noexcept { m_UpdateFunction = std::move(func); }
template<class FUNC> void SystemBaseClass::setUpdateFunction(const FUNC& func) noexcept { m_UpdateFunction = func; }
template<class FUNC> void SystemBaseClass::setComponentAddedToEntityFunction(FUNC&& func) noexcept { m_ComponentAddedToEntityFunction = std::move(func); }
template<class FUNC> void SystemBaseClass::setComponentAddedToEntityFunction(const FUNC& func) noexcept { m_ComponentAddedToEntityFunction = func; }
template<class FUNC> void SystemBaseClass::setComponentRemovedFromEntityFunction(FUNC&& func) noexcept { m_ComponentRemovedFromEntityFunction = std::move(func); }
template<class FUNC> void SystemBaseClass::setComponentRemovedFromEntityFunction(const FUNC& func) noexcept { m_ComponentRemovedFromEntityFunction = func; }
template<class FUNC> void SystemBaseClass::setEntityAddedToSceneFunction(FUNC&& func) noexcept { m_EntityToSceneFunction = std::move(func); }
template<class FUNC> void SystemBaseClass::setEntityAddedToSceneFunction(const FUNC& func) noexcept { m_EntityToSceneFunction = func; }
template<class FUNC> void SystemBaseClass::setSceneEnteredFunction(FUNC&& func) noexcept { m_SceneEnteredFunction = std::move(func); }
template<class FUNC> void SystemBaseClass::setSceneEnteredFunction(const FUNC& func) noexcept { m_SceneEnteredFunction = func; }
template<class FUNC> void SystemBaseClass::setSceneLeftFunction(FUNC&& func) noexcept { m_SceneLeftFunction = std::move(func); }
template<class FUNC> void SystemBaseClass::setSceneLeftFunction(const FUNC& func) noexcept { m_SceneLeftFunction = func; }

template<class COMPONENT>
void SystemBaseClass::associateComponent() {
    ASSERT(COMPONENT::TYPE_ID != 0, __FUNCTION__ << "(): COMPONENT::TYPE_ID was 0! please register this component (component: " << typeid(COMPONENT).name());
    ASSERT(!hasAssociatedComponent<COMPONENT>(), __FUNCTION__ << "(): already associated component: " << typeid(COMPONENT).name());
    associateComponentImpl(COMPONENT::TYPE_ID);
}
template<class COMPONENT>
[[nodiscard]] bool SystemBaseClass::hasAssociatedComponent() noexcept {
    return hasAssociatedComponent(COMPONENT::TYPE_ID);
}
template<class COMPONENT>
[[nodiscard]] Engine::priv::ECSComponentPool<COMPONENT>& SystemBaseClass::getComponentPool(uint32_t index) noexcept {
    return *static_cast<Engine::priv::ECSComponentPool<COMPONENT>*>(m_Components[index].second);
}
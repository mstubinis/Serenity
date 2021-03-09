
template<class T> 
void SystemBaseClass::setUpdateFunction(T&& func) noexcept {
    m_UpdateFunction = std::move(func);
}
template<class T> 
void SystemBaseClass::setUpdateFunction(const T& func) noexcept {
    m_UpdateFunction = func;
}

template<class T> 
void SystemBaseClass::setComponentAddedToEntityFunction(T&& func) noexcept {
    m_ComponentAddedToEntityFunction = std::move(func);
}
template<class T> 
void SystemBaseClass::setComponentAddedToEntityFunction(const T& func) noexcept {
    m_ComponentAddedToEntityFunction = func;
}

template<class T> 
void SystemBaseClass::setComponentRemovedFromEntityFunction(T&& func) noexcept {
    m_ComponentRemovedFromEntityFunction = std::move(func);
}
template<class T> 
void SystemBaseClass::setComponentRemovedFromEntityFunction(const T& func) noexcept {
    m_ComponentRemovedFromEntityFunction = func;
}

template<class T> 
void SystemBaseClass::setEntityAddedToSceneFunction(T&& func) noexcept {
    m_EntityToSceneFunction = std::move(func);
}
template<class T> 
void SystemBaseClass::setEntityAddedToSceneFunction(const T& func) noexcept {
    m_EntityToSceneFunction = func;
}

template<class T> 
void SystemBaseClass::setSceneEnteredFunction(T&& func) noexcept {
    m_SceneEnteredFunction = std::move(func);
}
template<class T> 
void SystemBaseClass::setSceneEnteredFunction(const T& func) noexcept {
    m_SceneEnteredFunction = func;
}

template<class T> 
void SystemBaseClass::setSceneLeftFunction(T&& func) noexcept {
    m_SceneLeftFunction = std::move(func);
}
template<class T> 
void SystemBaseClass::setSceneLeftFunction(const T& func) noexcept {
    m_SceneLeftFunction = func;
}

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
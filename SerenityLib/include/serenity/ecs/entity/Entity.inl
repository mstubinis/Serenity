

template<class COMPONENT>
bool Entity::removeComponent() noexcept {
    return Engine::priv::PublicEntity::GetECS(*this)->removeComponent<COMPONENT>(*this);
}
template<class COMPONENT>
[[nodiscard]] Engine::view_ptr<COMPONENT> Entity::getComponent() const noexcept {
    return Engine::priv::PublicEntity::GetECS(*this)->getComponent<COMPONENT>(*this);
}






template<class COMPONENT_LUA_BINDER> 
[[nodiscard]] luabridge::LuaRef Engine::priv::PublicEntity::GetComponent(lua_State* L, Entity entity, const char* globalName) {
    luabridge::setGlobal(L, COMPONENT_LUA_BINDER( entity ), globalName);
    return luabridge::getGlobal(L, globalName);
}
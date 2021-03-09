

template<class T> 
bool Entity::removeComponent() noexcept {
    return Engine::priv::PublicEntity::GetECS(*this)->removeComponent<T>(*this);
}
template<class T> 
[[nodiscard]] Engine::view_ptr<T> Entity::getComponent() const noexcept {
    return Engine::priv::PublicEntity::GetECS(*this)->getComponent<T>(*this);
}






template<class T> 
[[nodiscard]] luabridge::LuaRef Engine::priv::PublicEntity::GetComponent(lua_State* L, Entity entity, const char* globalName) {
    luabridge::setGlobal(L, entity.getComponent<T>(), globalName);
    return luabridge::getGlobal(L, globalName);
}
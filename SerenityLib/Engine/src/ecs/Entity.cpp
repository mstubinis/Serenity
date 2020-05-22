#include <ecs/Entity.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>

#include <ecs/Components.h>


using namespace Engine::priv;
using namespace std;

Entity Entity::null_ = Entity(0,0,0);

Entity::Entity(Scene& scene) {
    data = scene.createEntity().data;
}
Entity::Entity(const unsigned int entityID, const unsigned int sceneID, const unsigned int versionID) {
    data = versionID << 28 | sceneID << 21 | entityID;
}
Entity::Entity(const Entity& other) {
    data = other.data;
}
Entity& Entity::operator=(const Entity& other) {
    if (&other != this) {
        data = other.data;
    }
    return *this;
}
Entity::Entity(Entity&& other) noexcept {
    data = std::move(other.data);
}
Entity& Entity::operator=(Entity&& other) noexcept {
    data = std::move(other.data);
    return *this;
}


std::uint32_t Entity::id() const {
    const EntityDataRequest dataRequest(*this);
    return dataRequest.ID;
}
std::uint32_t Entity::sceneID() const {
    const EntityDataRequest dataRequest(*this);
    return dataRequest.sceneID;
}
std::uint32_t Entity::versionID() const {
    const EntityDataRequest dataRequest(*this);
    return dataRequest.versionID;
}

void Entity::addChild(const Entity child) const {
    const auto* body = getComponent<ComponentBody>();
    if (body) {
        body->addChild(child);
    }
}
void Entity::removeChild(const Entity child) const {
    const auto* body = getComponent<ComponentBody>();
    if (body) {
        body->removeChild(child);
    }
}
bool Entity::hasParent() const {
    const auto* body = getComponent<ComponentBody>();
    if (body) {
        return body->hasParent();
    }
    return false;
}


Scene& Entity::scene() const {
	const EntityDataRequest dataRequest(*this);
    return Core::m_Engine->m_ResourceManager._getSceneByID(dataRequest.sceneID);
}
void Entity::destroy() {
    Scene& s = scene();
    InternalScenePublicInterface::CleanECS(s, data);
    InternalScenePublicInterface::GetECS(s).removeEntity(*this);
}
bool Entity::operator==(const Entity other) const {
    return (data == other.data);
}
bool Entity::operator!=(const Entity other) const {
    return !(data == other.data);
}
bool Entity::null() const {
    return (data == 0);
}

void Entity::addComponent(const string& componentClassName, luabridge::LuaRef arg1, luabridge::LuaRef arg2, luabridge::LuaRef arg3, luabridge::LuaRef arg4, luabridge::LuaRef arg5, luabridge::LuaRef arg6, luabridge::LuaRef arg7, luabridge::LuaRef arg8) {
    if (componentClassName == "ComponentBody") {
        if (!arg1.isNil()) {
            addComponent<ComponentBody>(arg1.cast<CollisionType::Type>());
        }else{
            addComponent<ComponentBody>();
        }
    }else if (componentClassName == "ComponentModel") {
        if (!arg3.isNil() && !arg4.isNil()) {
            addComponent<ComponentModel>(arg1.cast<Handle>(), arg2.cast<Handle>(), arg3.cast<Handle>(), arg4.cast<RenderStage::Stage>());
        }else if(arg4.isNil()){
            addComponent<ComponentModel>(arg1.cast<Handle>(), arg2.cast<Handle>(), arg3.cast<Handle>());
        }else{
            addComponent<ComponentModel>(arg1.cast<Handle>(), arg2.cast<Handle>());
        }
    }else if (componentClassName == "ComponentCamera") {
        if (!arg5.isNil() || !arg6.isNil()) {
            addComponent<ComponentCamera>(arg1.cast<const float>(), arg2.cast<const float>(), arg3.cast<const float>(), arg4.cast<const float>(), arg5.cast<const float>(), arg6.cast<const float>());
        }else{
            addComponent<ComponentCamera>(arg1.cast<const float>(), arg2.cast<const float>(), arg3.cast<const float>(), arg4.cast<const float>());
        }
    }else if (componentClassName == "ComponentName") {
        addComponent<ComponentName>(arg1.cast<const char*>());
    }else if (componentClassName == "ComponentLogic") {
        if (!arg1.isNil() && arg1.isFunction()) {
            addComponent<ComponentLogic>(arg1.cast<const function<void(const ComponentLogic*, const float)>>());
        }else{
            addComponent<ComponentLogic>();
        }
    }else if (componentClassName == "ComponentLogic1") {
        if (!arg1.isNil() && arg1.isFunction()) {
            addComponent<ComponentLogic1>(arg1.cast<const function<void(const ComponentLogic1*, const float)>>());
        }else{
            addComponent<ComponentLogic1>();
        }
    }else if (componentClassName == "ComponentLogic2") {
        if (!arg1.isNil() && arg1.isFunction()) {
            addComponent<ComponentLogic2>(arg1.cast<const function<void(const ComponentLogic2*, const float)>>());
        }else{
            addComponent<ComponentLogic2>();
        }
    }else if (componentClassName == "ComponentLogic3") {
        if (!arg1.isNil() && arg1.isFunction()) {
            addComponent<ComponentLogic3>(arg1.cast<const function<void(const ComponentLogic3*, const float)>>());
        }else{
            addComponent<ComponentLogic3>();
        }
    }
}
bool Entity::removeComponent(const string& componentClassName) {
    if (componentClassName == "ComponentBody") {
        return removeComponent<ComponentBody>();
    }else if (componentClassName == "ComponentModel") {
        return removeComponent<ComponentModel>();
    }else if (componentClassName == "ComponentCamera") {
        return removeComponent<ComponentCamera>();
    }else if (componentClassName == "ComponentName") {
        return removeComponent<ComponentName>();
    }else if (componentClassName == "ComponentLogic") {
        return removeComponent<ComponentLogic>();
    }else if (componentClassName == "ComponentLogic1") {
        return removeComponent<ComponentLogic1>();
    }else if (componentClassName == "ComponentLogic2") {
        return removeComponent<ComponentLogic2>();
    }else if (componentClassName == "ComponentLogic3") {
        return removeComponent<ComponentLogic3>();
    }
    return false;
}
luabridge::LuaRef Entity::getComponent(const string& componentClassName) {
    lua_State* L = Engine::priv::getLUABinder().getState()->getState();
    string global_name = to_string(data) + "componentClassName";
    auto* global_name_cstr = global_name.c_str();
    if (componentClassName == "ComponentBody") {
        return InternalEntityPublicInterface::GetComponent<ComponentBody>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentModel") {
        return InternalEntityPublicInterface::GetComponent<ComponentModel>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentCamera") {
        return InternalEntityPublicInterface::GetComponent<ComponentCamera>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentName") {
        return InternalEntityPublicInterface::GetComponent<ComponentName>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic") {
        return InternalEntityPublicInterface::GetComponent<ComponentLogic>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic1") {
        return InternalEntityPublicInterface::GetComponent<ComponentLogic1>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic2") {
        return InternalEntityPublicInterface::GetComponent<ComponentLogic2>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic3") {
        return InternalEntityPublicInterface::GetComponent<ComponentLogic3>(L, *this, global_name_cstr);
    }else{
        luabridge::setGlobal(L, nullptr, global_name_cstr); // Prevents errors
        //printError("Component: (" + componentClassName + ") not found.");
    }
    return luabridge::getGlobal(L, global_name_cstr);
}
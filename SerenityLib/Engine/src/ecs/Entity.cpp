#include <ecs/Entity.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>

#include <ecs/Components.h>


using namespace Engine::priv;
using namespace std;

Entity Entity::null_ = Entity(0U,0U,0U);

Entity::Entity(Scene& scene) {
    m_Data = scene.createEntity().m_Data;
}
Entity::Entity(std::uint32_t entityID, std::uint32_t sceneID, std::uint32_t versionID) {
    m_Data = versionID << (ENTITY_SIZE - VERSION_BIT_POSITIONS) | sceneID << (ENTITY_SIZE - VERSION_BIT_POSITIONS - SCENE_BIT_POSITIONS) | entityID;
}
Entity::Entity(const Entity& other) {
    m_Data = other.m_Data;
}
Entity& Entity::operator=(const Entity& other) {
    if (&other != this) {
        m_Data = other.m_Data;
    }
    return *this;
}
Entity::Entity(Entity&& other) noexcept {
    m_Data = std::move(other.m_Data);
}
Entity& Entity::operator=(Entity&& other) noexcept {
    m_Data = std::move(other.m_Data);
    return *this;
}

void Entity::addChild(Entity& child) const {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        body->addChild(child);
    }
}
void Entity::removeChild(Entity& child) const {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        body->removeChild(child);
    }
}
bool Entity::hasParent() const {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        return body->hasParent();
    }
    return false;
}

Scene& Entity::scene() const {
    return Core::m_Engine->m_ResourceManager._getSceneByID(sceneID());
}
void Entity::destroy() {
    if (!null()) {
        Scene& s = scene();
        InternalScenePublicInterface::CleanECS(s, *this);
        InternalScenePublicInterface::GetECS(s).removeEntity(*this);
    }
}

void Entity::addComponent(const string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
    if (componentClassName == "ComponentBody") {
        if (!a1.isNil()) {
            addComponent<ComponentBody>(a1.cast<CollisionType::Type>());
        }else{
            addComponent<ComponentBody>();
        }
    }else if (componentClassName == "ComponentModel") {
        if (!a3.isNil() && !a4.isNil()) {
            addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>(), a4.cast<RenderStage::Stage>());
        }else if(a4.isNil()){
            addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>());
        }else{
            addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>());
        }
    }else if (componentClassName == "ComponentCamera") {
        if (!a5.isNil() || !a6.isNil()) {
            addComponent<ComponentCamera>(a1.cast<const float>(), a2.cast<const float>(), a3.cast<const float>(), a4.cast<const float>(), a5.cast<const float>(), a6.cast<const float>());
        }else{
            addComponent<ComponentCamera>(a1.cast<const float>(), a2.cast<const float>(), a3.cast<const float>(), a4.cast<const float>());
        }
    }else if (componentClassName == "ComponentName") {
        addComponent<ComponentName>(a1.cast<const char*>());
    }else if (componentClassName == "ComponentLogic") {
        if (!a1.isNil() && a1.isFunction()) {
            addComponent<ComponentLogic>(a1.cast<const function<void(const ComponentLogic*, const float)>>());
        }else{
            addComponent<ComponentLogic>();
        }
    }else if (componentClassName == "ComponentLogic1") {
        if (!a1.isNil() && a1.isFunction()) {
            addComponent<ComponentLogic1>(a1.cast<const function<void(const ComponentLogic1*, const float)>>());
        }else{
            addComponent<ComponentLogic1>();
        }
    }else if (componentClassName == "ComponentLogic2") {
        if (!a1.isNil() && a1.isFunction()) {
            addComponent<ComponentLogic2>(a1.cast<const function<void(const ComponentLogic2*, const float)>>());
        }else{
            addComponent<ComponentLogic2>();
        }
    }else if (componentClassName == "ComponentLogic3") {
        if (!a1.isNil() && a1.isFunction()) {
            addComponent<ComponentLogic3>(a1.cast<const function<void(const ComponentLogic3*, const float)>>());
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
    string global_name = to_string(m_Data) + componentClassName;
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
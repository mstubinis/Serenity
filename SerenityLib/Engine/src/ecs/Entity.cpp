#include <core/engine/utils/PrecompiledHeader.h>
#include <ecs/Entity.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/system/Engine.h>
#include <ecs/Components.h>

using namespace Engine::priv;

Entity::Entity(Scene& scene) {
    m_Data = scene.createEntity().m_Data;
}

void Entity::addChild(Entity child) const noexcept {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        body->addChild(child);
    }
}
void Entity::removeChild(Entity child) const noexcept {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        body->removeChild(child);
    }
}
void Entity::removeAllChildren() const noexcept {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        body->removeAllChildren();
    }
}
bool Entity::hasParent() const noexcept {
    auto* body = getComponent<ComponentBody>();
    if (body) {
        return body->hasParent();
    }
    return false;
}
bool Entity::isDestroyed() const noexcept {
    if (!null()) {
        Scene& s = scene();
        return InternalScenePublicInterface::GetECS(s).getEntityPool().isEntityVersionDifferent(*this);
    }
    return false;
}
Scene& Entity::scene() const noexcept {
    return Core::m_Engine->m_ResourceManager._getSceneByID(sceneID());
}
void Entity::destroy() noexcept {
    if (!null()) {
        Scene& scene_ = scene();
        InternalScenePublicInterface::CleanECS(scene_, *this);
        InternalScenePublicInterface::GetECS(scene_).removeEntity(*this);
    }
#ifndef ENGINE_PRODUCTION
    //else {
    //    ENGINE_PRODUCTION_LOG("Entity::destroy() called on a null entity")
    //}
#endif
}

void Entity::addComponent(const std::string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
    if (componentClassName == "ComponentBody") {
        if (!a1.isNil()) {
            addComponent<ComponentBody>(a1.cast<CollisionType>());
        }else{
            addComponent<ComponentBody>();
        }
    }else if (componentClassName == "ComponentModel") {
        if (!a3.isNil() && !a4.isNil()) {
            addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>(), a4.cast<RenderStage>());
        }else if(a4.isNil()){
            addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>());
        }else{
            addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>());
        }
    }else if (componentClassName == "ComponentCamera") {
        if (!a5.isNil() || !a6.isNil()) {
            addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>(), a5.cast<float>(), a6.cast<float>());
        }else{
            addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>());
        }
    }else if (componentClassName == "ComponentName") {
        addComponent<ComponentName>(a1.cast<const char*>());
    }else if (componentClassName == "ComponentLogic") {
        if (!a1.isNil() && a1.isFunction()) {
            using func_type = std::function<void(const ComponentLogic*, const float)>;
            addComponent<ComponentLogic>(std::move(a1.cast<func_type>()));
        }else{
            addComponent<ComponentLogic>();
        }
    }else if (componentClassName == "ComponentLogic1") {
        if (!a1.isNil() && a1.isFunction()) {
            using func_type = std::function<void(const ComponentLogic1*, const float)>;
            addComponent<ComponentLogic1>(std::move(a1.cast<func_type>()));
        }else{
            addComponent<ComponentLogic1>();
        }
    }else if (componentClassName == "ComponentLogic2") {
        if (!a1.isNil() && a1.isFunction()) {
            using func_type = std::function<void(const ComponentLogic2*, const float)>;
            addComponent<ComponentLogic2>(std::move(a1.cast<func_type>()));
        }else{
            addComponent<ComponentLogic2>();
        }
    }else if (componentClassName == "ComponentLogic3") {
        if (!a1.isNil() && a1.isFunction()) {
            using func_type = std::function<void(const ComponentLogic3*, const float)>;
            addComponent<ComponentLogic3>(std::move(a1.cast<func_type>()));
        }else{
            addComponent<ComponentLogic3>();
        }
    }
}
bool Entity::removeComponent(const std::string& componentClassName) {
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
luabridge::LuaRef Entity::getComponent(const std::string& componentClassName) {
    lua_State* L            = Engine::priv::getLUABinder().getState()->getState();
    std::string global_name = std::to_string(m_Data) + componentClassName;
    auto* global_name_cstr  = global_name.c_str();
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
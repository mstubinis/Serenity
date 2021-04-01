#include <serenity/ecs/entity/Entity.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>
#include <serenity/ecs/components/Components.h>

using namespace Engine::priv;

Entity::Entity(Scene& scene) {
    fill(scene.createEntity());
}

void Entity::destroy() noexcept {
    if (!null()) {
        Scene* scene_ptr = scene();
        if (!scene_ptr || (scene_ptr && PublicScene::GetECS(*scene_ptr).getEntityPool().isEntityVersionDifferent(*this))) {
            return;
        }
        PublicScene::CleanECS(*scene_ptr, *this);
        PublicScene::GetECS(*scene_ptr).removeEntity(*this);
    }
}
bool Entity::isDestroyed() const noexcept {
    if (!null()) {
        Scene* s = scene();
        if (s) {
            return PublicScene::GetECS(*s).getEntityPool().isEntityVersionDifferent(*this);
        }
    }
    return false;
}
void Entity::addChild(Entity child) const noexcept {
    auto body = getComponent<ComponentBody>();
    if (body)
        body->addChild(child);
}
void Entity::removeChild(Entity child) const noexcept {
    auto body = getComponent<ComponentBody>();
    if (body)
        body->removeChild(child);
}
/*
void Entity::removeAllChildren() const noexcept {
    auto body = getComponent<ComponentBody>();
    if (body)
        body->removeAllChildren();
}
*/
bool Entity::hasParent() const noexcept {
    auto body = getComponent<ComponentBody>();
    if (body)
        return body->hasParent();
    return false;
}
[[nodiscard]] Entity Entity::getParent() const noexcept {
    auto body = getComponent<ComponentBody>();
    if (body)
        return body->getParent();
    return Entity{};
}

Engine::view_ptr<Scene> Entity::scene() const noexcept {
    return Core::m_Engine->m_ResourceManager.getSceneByID(sceneID());
}
bool Entity::addComponent(std::string_view componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
    if (componentClassName == "ComponentBody") {
        return addComponent<ComponentBody>();
    }else if ("ComponentBodyRigid") {
        return addComponent<ComponentBodyRigid>();
    }else if ("ComponentCollisionShape") {
        if (!a1.isNil()) {
            return addComponent<ComponentCollisionShape>(a1);
        }
    }else if (componentClassName == "ComponentModel") {
        if (!a3.isNil() && !a4.isNil()) {
            return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>(), a4.cast<RenderStage>());
        }else if(a4.isNil()){
            return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>());
        }else{
            return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>());
        }
    }else if (componentClassName == "ComponentCamera") {
        if (!a5.isNil() || !a6.isNil()) {
            return addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>(), a5.cast<float>(), a6.cast<float>());
        }else{
            return addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>());
        }
    }else if (componentClassName == "ComponentName") {
        return addComponent<ComponentName>(a1.cast<const char*>());
    }else if (componentClassName == "ComponentLogic") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic>(a1);
        }else{
            return addComponent<ComponentLogic>();
        }
    }else if (componentClassName == "ComponentLogic1") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic1>(a1);
        }else{
            return addComponent<ComponentLogic1>();
        }
    }else if (componentClassName == "ComponentLogic2") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic2>(a1);
        }else{
            return addComponent<ComponentLogic2>();
        }
    }else if (componentClassName == "ComponentLogic3") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic3>(a1);
        }else{
            return addComponent<ComponentLogic3>();
        }
    }
    return false;
}
bool Entity::removeComponent(std::string_view componentClassName) {
    if (componentClassName == "ComponentBody") {
        return removeComponent<ComponentBody>();
    }else if (componentClassName == "ComponentBodyRigid") {
        return removeComponent<ComponentBodyRigid>();
    }else if (componentClassName == "ComponentCollisionShape") {
        return removeComponent<ComponentCollisionShape>();
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
luabridge::LuaRef Entity::getComponent(std::string_view componentClassName) {
    lua_State* L            = Engine::priv::getLUABinder().getState()->getState();
    std::string global_name = toString() + ":" + std::string{ componentClassName };
    auto* global_name_cstr  = global_name.c_str();
    if (componentClassName == "ComponentBody") {
        return PublicEntity::GetComponent<ComponentBody>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentBodyRigid") {
        return PublicEntity::GetComponent<ComponentBodyRigid>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentCollisionShape") {
        return PublicEntity::GetComponent<ComponentCollisionShape>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentModel") {
        return PublicEntity::GetComponent<ComponentModel>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentCamera") {
        return PublicEntity::GetComponent<ComponentCamera>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentName") {
        return PublicEntity::GetComponent<ComponentName>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic") {
        return PublicEntity::GetComponent<ComponentLogic>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic1") {
        return PublicEntity::GetComponent<ComponentLogic1>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic2") {
        return PublicEntity::GetComponent<ComponentLogic2>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentLogic3") {
        return PublicEntity::GetComponent<ComponentLogic3>(L, *this, global_name_cstr);
    }else{
        luabridge::setGlobal(L, nullptr, global_name_cstr); // Prevents errors
        //printError("Component: (" + componentClassName + ") not found.");
    }
    return luabridge::getGlobal(L, global_name_cstr);
}
Engine::view_ptr<Engine::priv::ECS> Engine::priv::PublicEntity::GetECS(Entity entity) {
    Scene* scene_ptr = entity.scene();
    return (scene_ptr) ? &Engine::priv::PublicScene::GetECS(*scene_ptr) : nullptr;
}
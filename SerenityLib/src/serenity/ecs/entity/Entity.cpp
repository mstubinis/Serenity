#include <serenity/ecs/entity/Entity.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>
#include <serenity/ecs/components/Components.h>

#include <serenity/ecs/systems/SystemTransformParentChild.h>

using namespace Engine::priv;

Entity::Entity(Scene& scene) {
    fill(scene.createEntity());
}
Entity::Entity(Entity&& other) noexcept {
    if (&other != this) {
        m_ID              = (other.m_ID);
        m_SceneID         = (other.m_SceneID);
        m_VersionID       = (other.m_VersionID);
        other.m_ID        = getMaxEntityIDBits(ID_BIT_POSITIONS);
        other.m_SceneID   = getMaxEntityIDBits(SCENE_BIT_POSITIONS);
        other.m_VersionID = getMaxEntityIDBits(VERSION_BIT_POSITIONS);
    }
}
Entity& Entity::operator=(Entity&& other) noexcept {
    if (&other != this) {
        m_ID              = (other.m_ID);
        m_SceneID         = (other.m_SceneID);
        m_VersionID       = (other.m_VersionID);
        other.m_ID        = getMaxEntityIDBits(ID_BIT_POSITIONS);
        other.m_SceneID   = getMaxEntityIDBits(SCENE_BIT_POSITIONS);
        other.m_VersionID = getMaxEntityIDBits(VERSION_BIT_POSITIONS);
    }
    return *this;
}
bool Entity::isValid() const noexcept {
    Scene* scene_ptr = scene();
    return !null() && (scene_ptr && !PublicScene::GetECS(*scene_ptr).getEntityPool().isEntityVersionDifferent(*this));
}
bool Entity::destroy() noexcept {
    if (!null()) {
        Scene* scene_ptr = scene();
        if (!scene_ptr || (scene_ptr && PublicScene::GetECS(*scene_ptr).getEntityPool().isEntityVersionDifferent(*this))) {
            return false;
        }
        PublicScene::CleanECS(*scene_ptr, *this);
        PublicScene::GetECS(*scene_ptr).removeEntity(*this);
        return true;
    }
    return false;
}
bool Entity::isDestroyed() const noexcept {
    if (!null()) {
        Scene* scene_ptr = scene();
        if (scene_ptr) {
            return PublicScene::GetECS(*scene_ptr).getEntityPool().isEntityVersionDifferent(*this);
        }
    }
    return false;
}
void Entity::addChild(Entity child) const noexcept {
    auto transform = getComponent<ComponentTransform>();
    if (transform) {
        transform->addChild(child);
    }
}
void Entity::removeChild(Entity child) const noexcept {
    auto transform = getComponent<ComponentTransform>();
    if (transform) {
        transform->removeChild(child);
    }
}
std::vector<Entity> Entity::getChildren() const noexcept {
    auto transform = getComponent<ComponentTransform>();
    std::vector<Entity> output;
    if (transform) {
        const auto& ecs = *Engine::priv::PublicEntity::GetECS(*this);
        const auto& pcs = ecs.getSystem<SystemTransformParentChild>();
        for (size_t i = 0; i < pcs.m_Parents.size(); ++i) {
            const auto parentID = pcs.m_Parents[i];
            if (parentID == m_ID) {
                output.push_back(ecs.getEntityPool().getEntityFromID(uint32_t(i)));
            }
        }
    }
    return output;
}
bool Entity::hasParent() const noexcept {
    auto transform = getComponent<ComponentTransform>();
    return (transform) ? transform->hasParent() : false;
}
[[nodiscard]] Entity Entity::getParent() const noexcept {
    auto transform = getComponent<ComponentTransform>();
    return (transform) ? transform->getParent() : Entity{};
}

Engine::view_ptr<Scene> Entity::scene() const noexcept {
    return Core::m_Engine->m_ResourceManager.getSceneByID(sceneID());
}
bool Entity::addComponent(std::string_view componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
    if (componentClassName == "ComponentTransform") {
        return addComponent<ComponentTransform>();
    } else if ("ComponentRigidBody") {
        return addComponent<ComponentRigidBody>();
    } else if ("ComponentCollisionShape") {
        if (!a1.isNil()) {
            return addComponent<ComponentCollisionShape>(a1);
        }
    } else if (componentClassName == "ComponentModel") {
        if (!a3.isNil() && !a4.isNil()) {
            return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>(), a4.cast<RenderStage>());
        } else if(a4.isNil()) {
            return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>());
        } else {
            return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>());
        }
    } else if (componentClassName == "ComponentCamera") {
        if (!a5.isNil() || !a6.isNil()) {
            return addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>(), a5.cast<float>(), a6.cast<float>());
        } else {
            return addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>());
        }
    } else if (componentClassName == "ComponentName") {
        return addComponent<ComponentName>(a1.cast<const char*>());
    } else if (componentClassName == "ComponentLogic") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic>(a1);
        } else {
            return addComponent<ComponentLogic>();
        }
    } else if (componentClassName == "ComponentLogic1") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic1>(a1);
        } else {
            return addComponent<ComponentLogic1>();
        }
    } else if (componentClassName == "ComponentLogic2") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic2>(a1);
        } else {
            return addComponent<ComponentLogic2>();
        }
    } else if (componentClassName == "ComponentLogic3") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic3>(a1);
        } else {
            return addComponent<ComponentLogic3>();
        }
    }
    return false;
}
bool Entity::removeComponent(std::string_view componentClassName) {
    if (componentClassName == "ComponentTransform") {
        return removeComponent<ComponentTransform>();
    }else if (componentClassName == "ComponentRigidBody") {
        return removeComponent<ComponentRigidBody>();
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
    lua_State* L            = &Engine::lua::getGlobalState();
    std::string global_name = toString() + ":" + std::string{ componentClassName };
    auto* global_name_cstr  = global_name.c_str();
    if (componentClassName == "ComponentTransform") {
        return PublicEntity::GetComponent<ComponentTransform>(L, *this, global_name_cstr);
    }else if (componentClassName == "ComponentRigidBody") {
        return PublicEntity::GetComponent<ComponentRigidBody>(L, *this, global_name_cstr);
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
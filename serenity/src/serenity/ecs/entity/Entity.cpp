#include <serenity/ecs/entity/Entity.h>
#include <serenity/scene/Scene.h>
#include <serenity/system/Engine.h>
#include <serenity/ecs/components/Components.h>

#include <serenity/ecs/systems/SystemTransformParentChild.h>

using namespace Engine::priv;


Entity::Entity(Scene& scene) {
    *this = scene.createEntity();
}
Entity::Entity(Entity&& other) noexcept 
    : m_ID{ (other.m_ID) }
    , m_SceneID{ (other.m_SceneID) }
    , m_VersionID{ (other.m_VersionID) }
{
    other.m_ID        = getMaxEntityIDBits(ID_BIT_POSITIONS);
    other.m_SceneID   = getMaxEntityIDBits(SCENE_BIT_POSITIONS);
    other.m_VersionID = getMaxEntityIDBits(VERSION_BIT_POSITIONS);
}
Entity& Entity::operator=(Entity&& other) noexcept {
    if (this != &other) {
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
    return !isNull() && (scene_ptr && !PublicScene::GetECS(*scene_ptr).getEntityPool().isEntityVersionDifferent(*this));
}
bool Entity::destroy() noexcept {
    if (!isNull()) {
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
    if (!isNull()) {
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
    return Engine::getResourceManager().getSceneByID(sceneID());
}
bool Entity::addComponent(const std::string& componentClassName, luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3, luabridge::LuaRef a4, luabridge::LuaRef a5, luabridge::LuaRef a6, luabridge::LuaRef a7, luabridge::LuaRef a8) {
    if (componentClassName == "ComponentTransform" || componentClassName == "Transform") {
        if (!a1.isNil()) {
            if (!a2.isNil() && !a3.isNil()) {
                return addComponent<ComponentTransform>(a1.cast<glm_vec3>(), a2.cast<glm::quat>(), a3.cast<glm::vec3>());
            } else if (!a2.isNil() && a3.isNil()) {
                return addComponent<ComponentTransform>(a1.cast<glm_vec3>(), a2.cast<glm::quat>());
            } else {
                return addComponent<ComponentTransform>(a1.cast<glm_vec3>());
            }
        } else {
            return addComponent<ComponentTransform>();
        }
    } else if (componentClassName == "ComponentRigidBody"  || componentClassName == "RigidBody") {
        return addComponent<ComponentRigidBody>();
    } else if (componentClassName == "ComponentCollisionShape" || componentClassName == "CollisionShape") {
        if (!a1.isNil()) {
            return addComponent<ComponentCollisionShape>(a1);
        }
    } else if (componentClassName == "ComponentModel" || componentClassName == "Model") {
        if (!a1.isNil() && !a2.isNil()) {
            if (a1.isString() && a2.isString()) {
                if (!a3.isNil() && !a4.isNil() && a3.isString()) {
                    return addComponent<ComponentModel>(a1.cast<std::string>(), a2.cast<std::string>(), a3.cast<std::string>(), RenderStage(a4.cast<uint32_t>()));
                } else if (a4.isNil() && !a3.isNil()) {
                    return addComponent<ComponentModel>(a1.cast<std::string>(), a2.cast<std::string>(), a3.cast<std::string>());
                } else {
                    return addComponent<ComponentModel>(a1.cast<std::string>(), a2.cast<std::string>());
                }
            } else {
                if (!a3.isNil() && !a4.isNil()) {
                    return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>(), RenderStage(a4.cast<uint32_t>()));
                } else if (a4.isNil() && !a3.isNil()) {
                    return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>(), a3.cast<Handle>());
                } else {
                    return addComponent<ComponentModel>(a1.cast<Handle>(), a2.cast<Handle>());
                }
            }
        }
    } else if (componentClassName == "ComponentCamera" || componentClassName == "Camera") {
        if (!a5.isNil() || !a6.isNil()) {
            return addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>(), a5.cast<float>(), a6.cast<float>());
        } else {
            return addComponent<ComponentCamera>(a1.cast<float>(), a2.cast<float>(), a3.cast<float>(), a4.cast<float>());
        }
    } else if (componentClassName == "ComponentName" || componentClassName == "Name") {
        if (!a1.isNil() && a1.isString()) {
            return addComponent<ComponentName>(a1.cast<const char*>());
        }
    } else if (componentClassName == "ComponentLogic" || componentClassName == "Logic") {
        if (!a1.isNil() && a1.isFunction()) {
            return addComponent<ComponentLogic>(a1);
        } else {
            return addComponent<ComponentLogic>();
        }
    } else if (componentClassName == "ComponentScript" || componentClassName == "Script") {
        if (!a1.isNil() && a1.isString()) {
            return addComponent<ComponentScript>(a1.cast<const char*>());
        } else {
            return addComponent<ComponentScript>();
        }
    }
    return false;
}
bool Entity::removeComponent(const std::string& componentClassName) {
    if (componentClassName == "ComponentTransform" || componentClassName == "Transform") {
        return removeComponent<ComponentTransform>();
    } else if (componentClassName == "ComponentRigidBody" || componentClassName == "RigidBody") {
        return removeComponent<ComponentRigidBody>();
    } else if (componentClassName == "ComponentCollisionShape" || componentClassName == "CollisionShape") {
        return removeComponent<ComponentCollisionShape>();
    } else if (componentClassName == "ComponentModel" || componentClassName == "Model") {
        return removeComponent<ComponentModel>();
    } else if (componentClassName == "ComponentCamera" || componentClassName == "Camera") {
        return removeComponent<ComponentCamera>();
    } else if (componentClassName == "ComponentName" || componentClassName == "Name") {
        return removeComponent<ComponentName>();
    } else if (componentClassName == "ComponentLogic" || componentClassName == "Logic") {
        return removeComponent<ComponentLogic>();
    } else if (componentClassName == "ComponentScript" || componentClassName == "Script") {
        return removeComponent<ComponentScript>();
    }
    return false;
}
luabridge::LuaRef Entity::getComponent(luabridge::LuaRef componentClassName) {
    lua_State* L = &Engine::lua::getGlobalState();
    if (!componentClassName.isNil() && componentClassName.isString()) {
        std::string local_name = componentClassName.cast<std::string>();
        if (local_name == "ComponentTransform" || local_name == "Transform") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentTransformLUABinder{*this} };
        } else if (local_name == "ComponentRigidBody" || local_name == "RigidBody") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentRigidBodyLUABinder{*this} };
        } else if (local_name == "ComponentCollisionShape" || local_name == "CollisionShape") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentCollisionShapeLUABinder{*this} };
        } else if (local_name == "ComponentModel" || local_name == "Model") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentModelLUABinder{*this} };
        } else if (local_name == "ComponentCamera" || local_name == "Camera") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentCameraLUABinder{*this} };
        } else if (local_name == "ComponentName" || local_name == "Name") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentNameLUABinder{*this} };
        } else if (local_name == "ComponentLogic" || local_name == "Logic") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentLogicLUABinder{*this} };
        } else if (local_name == "ComponentScript" || local_name == "Script") {
            return luabridge::LuaRef{ L, Engine::priv::ComponentScriptLUABinder{*this} };
        } else {
            ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): Component: (" + local_name + ") not found.")
        }
    }
    return luabridge::LuaRef{ L };
}
Engine::view_ptr<Engine::priv::ECS> Engine::priv::PublicEntity::GetECS(Entity entity) {
    Scene* scene_ptr = entity.scene();
    return scene_ptr ? &Engine::priv::PublicScene::GetECS(*scene_ptr) : nullptr;
}
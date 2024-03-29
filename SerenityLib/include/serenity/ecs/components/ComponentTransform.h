#pragma once
#ifndef ENGINE_ECS_COMPONENT_TRANSFORM_H
#define ENGINE_ECS_COMPONENT_TRANSFORM_H

struct SceneOptions;
class  ComponentModel;
class  ComponentTransform;
class  SystemComponentTransform;
class  SystemTransformParentChild;
class  SystemSyncRigidToTransform;
class  SystemSyncTransformToRigid;
namespace Engine::priv {
    class  sparse_set_base;
    class  EditorWindowSceneFunctions;
};

#include <serenity/dependencies/glm.h>
#include <serenity/ecs/ECS.h>
#include <serenity/events/Observer.h>
#include <serenity/renderer/RendererIncludes.h>

class ComponentTransform : public ComponentBaseClass<ComponentTransform> {
    friend class  ComponentModel;
    friend class  SystemComponentTransform;
    friend class  SystemTransformParentChild;
    friend class  SystemSyncRigidToTransform;
    friend class  SystemSyncTransformToRigid;
    friend class  Engine::priv::EditorWindowSceneFunctions;
    private:
        glm_vec3    m_Position          = glm_vec3{ 0.0 };
        //glm_vec3    m_LinearVelocity    = glm_vec3{ 0.0 };
        glm::quat   m_Rotation          = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f };
        glm::vec3   m_Scale             = glm::vec3{ 1.0f };

        glm::vec3   m_Forward           = glm::vec3{ 0.0f, 0.0f, -1.0f };
        glm::vec3   m_Right             = glm::vec3{ 1.0f, 0.0f, 0.0f };
        glm::vec3   m_Up                = glm::vec3{ 0.0f, 1.0f, 0.0f };


        void*       m_UserPointer       = nullptr;
        void*       m_UserPointer1      = nullptr;
        void*       m_UserPointer2      = nullptr;
        Entity      m_Owner;
    public:
        static void recalculateAllParentChildMatrices(Scene&);
    public:
        ComponentTransform(Entity);
        ComponentTransform(Entity, const glm_vec3& pos, const glm::quat& rot = glm::quat{ 1.0f, 0.0f, 0.0f, 0.0f }, const glm::vec3& scl = glm::vec3{ 1.0f });
        ComponentTransform(const ComponentTransform&)                  = delete;
        ComponentTransform& operator=(const ComponentTransform&)       = delete;
        ComponentTransform(ComponentTransform&&) noexcept;
        ComponentTransform& operator=(ComponentTransform&&) noexcept;

        [[nodiscard]] inline Entity getOwner() const noexcept { return m_Owner; }

        [[nodiscard]] bool hasParent() const;
        [[nodiscard]] Entity getParent() const;

        void addChild(Entity child) const;
        inline void addChild(const ComponentTransform& child) const noexcept { addChild(child.m_Owner); }

        void removeChild(Entity child) const;
        inline void removeChild(const ComponentTransform& child) const noexcept { removeChild(child.m_Owner); }

        void setUserPointer(void* userPtr) noexcept { m_UserPointer = userPtr; }
        void setUserPointer1(void* userPtr) noexcept { m_UserPointer1 = userPtr; }
        void setUserPointer2(void* userPtr) noexcept { m_UserPointer2 = userPtr; }
        [[nodiscard]] inline void* getUserPointer() const noexcept { return m_UserPointer; }
        [[nodiscard]] inline void* getUserPointer1() const noexcept { return m_UserPointer1; }
        [[nodiscard]] inline void* getUserPointer2() const noexcept { return m_UserPointer2; }

        void alignTo(float dirX, float dirY, float dirZ);
        void alignTo(const glm::vec3& direction);
        void alignToDirection(float dirX, float dirY, float dirZ);
        void alignToDirection(const glm::vec3& direction);
        void alignToPosition(decimal x, decimal y, decimal z);
        void alignToPosition(const glm_vec3& position);

        inline void translate(const glm_vec3& translation, bool local = true) noexcept { translate(translation.x, translation.y, translation.z, local); }
        inline void translate(decimal translation, bool local = true) noexcept { translate(translation, translation, translation, local); }
        void translate(decimal x, decimal y, decimal z, bool local = true);

        inline void rotate(const glm::vec3& rotation_radians, bool local = true) noexcept { rotate(rotation_radians.x, rotation_radians.y, rotation_radians.z, local); }
        void rotate(float pitch_radians, float yaw_radians, float roll_radians, bool local = true);


        void rotatePitch(float pitch_radians, bool local = true);
        void rotateYaw(float yaw_radians, bool local = true);
        void rotateRoll(float roll_radians, bool local = true);

        void scale(float x, float y, float z);
        inline void scale(const glm::vec3& scaleAmount) noexcept { scale(scaleAmount.x, scaleAmount.y, scaleAmount.z); }
        inline void scale(float scaleAmount) noexcept { scale(scaleAmount, scaleAmount, scaleAmount); }

        inline void setPosition(const glm_vec3& newPosition) noexcept { setPosition(newPosition.x, newPosition.y, newPosition.z); }
        inline void setPosition(decimal newPosition) noexcept { setPosition(newPosition, newPosition, newPosition); }
        void setPosition(decimal x, decimal y, decimal z);


        inline void setLocalPosition(const glm_vec3& newPosition) noexcept { setLocalPosition(newPosition.x, newPosition.y, newPosition.z); }
        inline void setLocalPosition(decimal newPosition) noexcept { setLocalPosition(newPosition, newPosition, newPosition); }
        void setLocalPosition(decimal x, decimal y, decimal z);

        inline void setRotation(const glm::quat& newRotation) noexcept { setRotation(newRotation.x, newRotation.y, newRotation.z, newRotation.w); }
        void setRotation(float quat_x, float quat_y, float quat_z, float quat_w);

        void setScale(float x, float y, float z);
        inline void setScale(const glm::vec3& newScale) noexcept { setScale(newScale.x, newScale.y, newScale.z); }
        inline void setScale(float newScale) noexcept { setScale(newScale, newScale, newScale); }

        [[nodiscard]] decimal getDistance(Entity other) const;
        [[nodiscard]] uint64_t getDistanceLL(Entity other) const;
        [[nodiscard]] glm::vec3 getScreenCoordinates(bool clampToEdge = false) const;

        [[nodiscard]] ScreenBoxCoordinates getScreenBoxCoordinates(float minOffset = 10.0f) const;

        [[nodiscard]] inline glm::quat getRotation() const noexcept { return m_Rotation; }
        [[nodiscard]] inline glm::quat getLocalRotation() const noexcept { return getRotation(); }
        [[nodiscard]] glm::quat getWorldRotation() const;

        [[nodiscard]] inline glm::vec3 getScale() const noexcept { return m_Scale; }
        [[nodiscard]] glm_vec3 getPosition() const;
        [[nodiscard]] glm_vec3 getWorldPosition() const;
        [[nodiscard]] inline glm_vec3 getLocalPosition() const noexcept { return m_Position; }
        
        [[nodiscard]] inline const glm::vec3& getForward() const noexcept { return m_Forward; }
        [[nodiscard]] inline const glm::vec3& getRight() const noexcept { return m_Right; }
        [[nodiscard]] inline const glm::vec3& getUp() const noexcept { return m_Up; }
        
        //[[nodiscard]] inline glm_vec3 getLinearVelocity() const noexcept { return m_LinearVelocity; }
        [[nodiscard]] inline glm::mat4 getWorldMatrixRendering() const noexcept { return glm::mat4{ getWorldMatrix() }; }
        [[nodiscard]] const glm_mat4& getWorldMatrix() const noexcept;
        [[nodiscard]] const glm_mat4& getLocalMatrix() const noexcept;

        //void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        //inline void setLinearVelocity(const glm_vec3& velocity, bool local = true) noexcept { setLinearVelocity(velocity.x, velocity.y, velocity.z, local); }
};


namespace Engine::priv {
    class ComponentTransformLUABinder {
        private:
            Entity m_Owner;

            ComponentTransformLUABinder() = delete;
        public:
            ComponentTransformLUABinder(Entity owner) 
                : m_Owner{ owner }
            {}

            void addChild(Entity child) const;
            void removeChild(Entity child) const;
            bool hasParent() const;

            void setPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const;
            void setRotation(float x, float y, float z, float w) const;
            void setScale(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const;

            void setLocalPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const;

            void translate(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z, bool local) const;
            void rotate(float x, float y, float z, bool local) const;
            void scale(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const;
            decimal getDistance(Entity other) const;

            glm_vec3 getPosition() const;
            glm::quat getRotation() const;
            glm::vec3 getScale() const;

            glm_vec3 getLocalPosition() const;

            //TODO: use const references?
            glm::vec3 getForward() const;
            glm::vec3 getRight() const;
            glm::vec3 getUp() const;

            void alignTo(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z);
            void alignToDirection(luabridge::LuaRef dirX, luabridge::LuaRef dirY, luabridge::LuaRef dirZ);
            void alignToPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z);

            const glm_mat4& getWorldMatrix() const;
            glm::mat4 getWorldMatrixRendering() const;
            const glm_mat4& getLocalMatrix() const;


            /*

            .addFunction("getScreenCoordinates", &ComponentTransform::getScreenCoordinates)
            .addFunction("alignTo", static_cast<void(ComponentTransform::*)(const glm::vec3&)>(&ComponentTransform::alignTo))
            .addFunction("alignTo", static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::alignTo))

            //.addFunction("getLinearVelocity", &ComponentTransform::getLinearVelocity)
            */

    };
}

#endif

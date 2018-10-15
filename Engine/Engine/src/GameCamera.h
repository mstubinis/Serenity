#pragma once
#ifndef GAME_GAME_CAMERA_H
#define GAME_GAME_CAMERA_H

#include "core/Camera.h"

//
// OLD
//
#pragma region Old

struct CameraState {enum State {
    Freeform,
    Orbit,
    Follow,
    FollowTarget,
_TOTAL};};
class OLD_GameCameraComponent: public OLD_ComponentCamera{
    public:
        CameraState::State m_State;
        float m_OrbitRadius;
        OLD_Entity* m_Target;
        OLD_Entity* m_Player;
        OLD_ComponentBody* m_Body;
        glm::vec2 m_CameraMouseFactor;

        OLD_GameCameraComponent(float angle,float aspectRatio,float nearPlane,float farPlane);
        OLD_GameCameraComponent(float left,float right,float bottom,float top,float nearPlane,float farPlane);
        ~OLD_GameCameraComponent();

        void update(const float& dt);
};
class OLD_GameCamera: public Camera{
    public:
        OLD_GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);                   // Perspective camera Constructor
        OLD_GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~OLD_GameCamera();

        void follow(OLD_Entity*);
        void followTarget(OLD_Entity* target, OLD_Entity* player);
        void orbit(OLD_Entity*);

        void update(const float& dt);
        void render();

        void setTarget(OLD_Entity* target);
        const OLD_Entity* getTarget() const;
        const CameraState::State getState() const;

        OLD_Entity* getObjectInCenterRay(OLD_Entity* exclusion);
};

#pragma endregion


//
// NEW
//

#pragma region New

#pragma endregion

#endif

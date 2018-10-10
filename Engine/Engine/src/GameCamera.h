#pragma once
#ifndef GAME_GAME_CAMERA_H
#define GAME_GAME_CAMERA_H

#include "core/Camera.h"

enum CAMERA_STATE { 
    CAMERA_STATE_FREEFORM,
    CAMERA_STATE_ORBIT,
    CAMERA_STATE_FOLLOW,
    CAMERA_STATE_FOLLOWTARGET 
};
class GameCameraComponent: public OLD_ComponentCamera{
    public:
        CAMERA_STATE m_State;
        float m_OrbitRadius;
        OLD_Entity* m_Target;
        OLD_Entity* m_Player;
        OLD_ComponentBody* m_Body;
        glm::vec2 m_CameraMouseFactor;

        GameCameraComponent(float angle,float aspectRatio,float nearPlane,float farPlane);
        GameCameraComponent(float left,float right,float bottom,float top,float nearPlane,float farPlane);
        ~GameCameraComponent();

        void update(const float& dt);
};
class GameCamera: public Camera{
    public:
        GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);                   // Perspective camera Constructor
        GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~GameCamera();

        void follow(OLD_Entity*);
        void followTarget(OLD_Entity* target, OLD_Entity* player);
        void orbit(OLD_Entity*);

        void update(const float& dt);
        void render();

        void setTarget(OLD_Entity* target);
        const OLD_Entity* getTarget() const;
        const CAMERA_STATE getState() const;

        OLD_Entity* getObjectInCenterRay(OLD_Entity* exclusion);
};
#endif
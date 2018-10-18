#pragma once
#ifndef GAME_GAME_CAMERA_H
#define GAME_GAME_CAMERA_H

#include "core/Camera.h"

struct GameCameraLogicFunctor;

struct CameraState {enum State {
    Freeform,
    Orbit,
    Follow,
    FollowTarget,
_TOTAL};};

class GameCamera: public Camera{
    friend struct ::GameCameraLogicFunctor;
    public:
        CameraState::State m_State;
        float m_OrbitRadius;
        Entity m_Target;
        Entity m_Player;
        glm::vec2 m_CameraMouseFactor;

        GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);                   // Perspective camera Constructor
        GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~GameCamera();

        void follow(Entity&);
        void followTarget(Entity& target, Entity& player);
        void orbit(Entity&);

        void setTarget(Entity&);
        const Entity& getTarget() const;
        const CameraState::State getState() const;

        Entity getObjectInCenterRay(Entity& exclusion);
};

#endif

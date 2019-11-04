#pragma once
#ifndef GAME_GAME_CAMERA_H
#define GAME_GAME_CAMERA_H

#include <core/engine/scene/Camera.h>

struct GameCameraLogicFunctor;
class Map;

struct CameraState {enum State {
    Freeform,
    Orbit,
    Cockpit,
    FollowTarget,
_TOTAL};};

class GameCamera: public Camera{
    friend struct ::GameCameraLogicFunctor;
    private:
        const bool validateDistanceForOrbit(Map& map);
    public:
        CameraState::State   m_State;
        float                m_OrbitRadius;
        EntityWrapper*       m_Target;
        EntityWrapper*       m_Player;
        glm::dvec2           m_CameraMouseFactor;

        GameCamera(float clipStart, float clipEnd, Scene* = nullptr);                                                  // Perspective camera Constructor
        GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);                   // Perspective camera Constructor
        GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~GameCamera();

        void setState(const CameraState::State&);

        void setPlayer(EntityWrapper*);
        void setTarget(const std::string& targetName);
        void setTarget(EntityWrapper*);
        EntityWrapper* getTarget();
        const CameraState::State getState() const;

        Entity getObjectInCenterRay(Entity& exclusion);
};

#endif

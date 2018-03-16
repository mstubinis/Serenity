#pragma once
#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#include "Camera.h"
class ObjectDisplay;

enum CAMERA_STATE { CAMERA_STATE_FREEFORM,CAMERA_STATE_ORBIT,CAMERA_STATE_FOLLOW,CAMERA_STATE_FOLLOWTARGET };

class GameCamera: public Camera{
    protected:
        CAMERA_STATE m_State;
        Object* m_Target;
        Object* m_Player;
        float m_OrbitRadius;
		ComponentBasicBody* m_Body;
		ComponentCamera* m_Camera;
    public:
        GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);                   // Perspective camera Constructor
        GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
        virtual ~GameCamera();

        void follow(Object*);
        void followTarget(Object*,Object*);
        void orbit(Object*);

        void update(const float& dt);
        void render();

        void setTarget(Object* target) { m_Target = target; }
        const Object* getTarget() const { return m_Target; }
        const CAMERA_STATE getState() const { return m_State; }

        Object* getObjectInCenterRay(Object* exclusion);
};
#endif
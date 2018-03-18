#pragma once
#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#include "Camera.h"
class ObjectDisplay;

enum CAMERA_STATE { CAMERA_STATE_FREEFORM,CAMERA_STATE_ORBIT,CAMERA_STATE_FOLLOW,CAMERA_STATE_FOLLOWTARGET };

class GameCameraComponent: public ComponentCamera{
    public:
		CAMERA_STATE m_State;
		float m_OrbitRadius;
        Object* m_Target;
        Object* m_Player;
		ComponentBasicBody* m_Body;

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

        void follow(Object*);
        void followTarget(Object* target,Object* player);
        void orbit(Object*);

        void update(const float& dt);
        void render();

        void setTarget(Object* target);
        const Object* getTarget() const;
        const CAMERA_STATE getState() const;

        Object* getObjectInCenterRay(Object* exclusion);
};
#endif
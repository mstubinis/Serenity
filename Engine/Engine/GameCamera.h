#pragma once
#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#include "Camera.h"

enum CAMERA_STATE { CAMERA_STATE_FREEFORM,CAMERA_STATE_ORBIT,CAMERA_STATE_FOLLOW,CAMERA_STATE_FOLLOWTARGET };

class GameCameraComponent: public ComponentCamera{
    public:
		CAMERA_STATE m_State;
		float m_OrbitRadius;
        Entity* m_Target;
        Entity* m_Player;
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

        void follow(Entity*);
        void followTarget(Entity* target,Entity* player);
        void orbit(Entity*);

        void update(const float& dt);
        void render();

        void setTarget(Entity* target);
        const Entity* getTarget() const;
        const CAMERA_STATE getState() const;

        Object* getObjectInCenterRay(Object* exclusion);
		Entity* getObjectInCenterRay(Entity* exclusion);
};
#endif
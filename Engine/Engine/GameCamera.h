#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#include "Camera.h"

enum CAMERA_STATE { CAMERA_STATE_FREEFORM,CAMERA_STATE_ORBIT,CAMERA_STATE_FOLLOW };

class GameCamera: public Camera{
	protected:
		CAMERA_STATE m_State;
		Object* m_Target;
		float m_OrbitRadius;
	public:
		GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);				      // Perspective camera Constructor
		GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
		~GameCamera();

		void follow(Object*); 
		void orbit(Object*);

		void update(float);
		void render();

		void setTarget(Object* target) { m_Target = target; }
		const Object* getTarget() const { return m_Target; }
		const CAMERA_STATE getState() const { return m_State; }
};
#endif
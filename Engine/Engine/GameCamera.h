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
		GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd);				      // Perspective camera Constructor
		GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd); // Orthographic camera Constructor
		~GameCamera();

		void Follow(Object*); void Orbit(Object*);

		void Update(float);
		void Render();

		const CAMERA_STATE State() const { return m_State; }
};
#endif
#ifndef GAMECAMERA_H
#define GAMECAMERA_H

#include "Camera.h"
class ObjectDisplay;

enum CAMERA_STATE { CAMERA_STATE_FREEFORM,CAMERA_STATE_ORBIT,CAMERA_STATE_FOLLOW };

class GameCamera: public Camera{
	protected:
		CAMERA_STATE m_State;
		ObjectDisplay* m_Target;
		float m_OrbitRadius;
	public:
		GameCamera(float angle, float aspectRatio, float clipStart, float clipEnd,Scene* = nullptr);				      // Perspective camera Constructor
		GameCamera(float left, float right, float bottom, float top, float clipStart, float clipEnd,Scene* = nullptr); // Orthographic camera Constructor
		~GameCamera();

		void follow(ObjectDisplay*); 
		void orbit(ObjectDisplay*);

		void update(float);
		void render();

		void setTarget(ObjectDisplay* target) { m_Target = target; }
		const ObjectDisplay* getTarget() const { return m_Target; }
		const CAMERA_STATE getState() const { return m_State; }
};
#endif
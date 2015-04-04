#ifndef _CAMERA_H
#define _CAMERA_H

#include "Object.h"

enum CAMERA_TYPE { CAMERA_TYPE_PERSPECTIVE, CAMERA_TYPE_ORTHOGRAPHIC };
enum CAMERA_STATE { CAMERA_STATE_FREEFORM,CAMERA_STATE_ORBIT,CAMERA_STATE_FOLLOW };

class Camera: public Object{
	private:
		CAMERA_TYPE m_Type;
		CAMERA_STATE m_State;

		float m_Angle;
		float m_AspectRatio;

		float m_ClipStart, m_ClipEnd;

		float m_Friction;

		glm::vec3 m_Velocity, m_Velocity_Rotation;
		glm::mat4 m_View, m_Projection;

		void Set_View();

	public:
		Camera(float angle, float aspectRatio, float clipStart, float clipEnd);				      // Perspective camera Constructor
		Camera(float left, float right, float bottom, float top, float clipStart, float clipEnd); // Orthographic camera Constructor
		~Camera();

		CAMERA_TYPE Get_Type() const;
		CAMERA_STATE Get_State() const;

		void Set_Perspective_Projection();
		void Set_Ortho_Projection(float,float,float,float);

		void Update(float);
		void Render(Mesh*,Material*,bool=false);
		void Render(bool=false);

		void LookAt(glm::vec3&);  
		void LookAt(glm::vec3&,glm::vec3&); 
		void LookAt(glm::vec3&,glm::vec3&,glm::vec3&); 
		void LookAt(Object*,bool targetUp = false);

		void Set_Aspect_Ratio(float);

		glm::vec3 Position() const;
		glm::mat4 Calculate_ViewProjInverted();
		glm::mat4 Calculate_Projection(glm::mat4&);
		glm::mat4 Calculate_ModelView(glm::mat4&);
		glm::mat4 Projection() const;
		glm::mat4 View() const;

		float Get_Angle_Between(Object*);

};
#endif
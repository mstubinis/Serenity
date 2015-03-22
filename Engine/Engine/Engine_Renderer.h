#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H
#include "GBuffer.h"

struct PointLight;

//this class will control what rendering techniques to use (forward or deferred)

enum RENDER_TYPE {RENDER_TYPE_FORWARD,RENDER_TYPE_DEFERRED};
class Renderer{
	private:
		GLuint RandomMapSSAO;
		GBuffer* m_gBuffer;

		bool m_EnableLighting;
		bool m_EnableNormalMapping;
		bool m_EnableLightmapping;

		RENDER_TYPE m_Type;

		float CalcPointLightBSphere(const PointLight* Light);

		void Init_Quad();

		void Pass_Light_Point();
		void Pass_Light_Dir();
		void Pass_Light_Spot();

		void Pass_SSAO();
		void Pass_Blur_Horizontal(GLuint texture);
		void Pass_Blur_Vertical(GLuint texture);
		void Pass_AtmosphericScatteringSpace();
		void Pass_AtmosphericScatteringGround();

		void Pass_Final();

	public:
		Renderer(RENDER_TYPE = RENDER_TYPE_FORWARD);
		~Renderer();

		void Render(bool debug = false);
		void Set_Render_Type(RENDER_TYPE);

		void Geometry_Pass(bool debug);
		void Lighting_Pass();
};
extern Renderer* renderer;
#endif
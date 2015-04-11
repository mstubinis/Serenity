#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include <GL/glew.h>
#include <GL/GL.h>

class Object;
class GBuffer;
class Renderer{
	private:
		GLuint RandomMapSSAO;
		GBuffer* m_gBuffer;

		void Init_Quad();

		void Pass_Lighting();
		void Pass_SSAO();
		void Pass_Blur_Horizontal(GLuint texture);
		void Pass_Blur_Vertical(GLuint texture);
		void Pass_Final();

	public:
		Renderer();
		~Renderer();

		void Render(bool debug = false);

		void Geometry_Pass(bool debug);
		void Lighting_Pass();
};
extern Renderer* renderer;
#endif
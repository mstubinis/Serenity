#ifndef ENGINE_RENDER_H
#define ENGINE_RENDER_H

#include <GL/glew.h>
#include <GL/GL.h>

class Object;
class GBuffer;
class Font;
class Renderer{
	private:

		GLuint RandomMapSSAO;
		GBuffer* m_gBuffer;
		Font* m_Font;

		void _initQuad();

		void _geometryPass(bool debug);
		void _lightingPass();

		void _passLighting();
		void _passSSAO();
		void _passBlurHorizontal(GLuint texture);
		void _passBlurVertical(GLuint texture);
		void _passFinal();

	public:
		Renderer();
		~Renderer();

		void render(bool debug = false);
};
extern Renderer* renderer;
#endif
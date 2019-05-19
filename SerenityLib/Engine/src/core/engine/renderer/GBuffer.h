#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <memory>
#include <core/engine/Engine_Utils.h>

class Texture;
namespace Engine{
namespace epriv{
    class FramebufferTexture;
    class FramebufferObject;
    struct GBufferType{enum Type{
        Diffuse, Normal, Misc, Lighting, Bloom, GodRays, Depth,
    _TOTAL};};
    class GBuffer final{
        private:
            class impl; std::unique_ptr<impl> m_i;
        public:
            GBuffer(uint width,uint height);
            ~GBuffer();

            void resize(uint width,uint height);

            void start(uint,std::string = "RGBA",bool = true);
            void start(uint,uint,std::string = "RGBA",bool = true);
            void start(uint,uint,uint,std::string = "RGBA",bool = true);
            void start(uint,uint,uint,uint,std::string = "RGBA",bool = true);
            void start(uint,uint,uint,uint,uint,std::string = "RGBA",bool = true);
            void stop(GLuint fbo = 0,GLuint rbo = 0);

            const std::vector<FramebufferTexture*>& getBuffers() const;
            FramebufferTexture& getBuffer(uint);
            Texture& getTexture(uint);

            FramebufferObject* getMainFBO();
            FramebufferObject* getSmallFBO();
    };
};
};
#endif

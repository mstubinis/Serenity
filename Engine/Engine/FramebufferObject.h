#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include "BindableResource.h"

class Texture;

class FramebufferAttatchment{public: enum Attatchment{
    Color_0, Color_1, Color_2, Color_3, Color_4, Color_5,
    Color_6, Color_7, Color_8, Depth, Stencil, DepthAndStencil
};};

class FramebufferObjectAttatchment{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        FramebufferObjectAttatchment();
        ~FramebufferObjectAttatchment();
};

class RenderbufferObject{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        RenderbufferObject();
        ~RenderbufferObject();
};

class FramebufferObject: public BindableResource{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        FramebufferObject(std::string name,uint width,uint height);
        ~FramebufferObject();

        void resize(uint,uint);
    
        void attatchTexture(Texture*,FramebufferAttatchment::Attatchment);
        void attatchRenderBuffer(RenderbufferObject*,FramebufferAttatchment::Attatchment);
};
#endif

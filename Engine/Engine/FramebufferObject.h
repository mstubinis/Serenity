#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include "BindableResource.h"

class FramebufferObject: public BindableResource{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        FramebufferObject(std::string name,uint width,uint height);
        ~FramebufferObject();

        void resize(uint,uint);
};
#endif

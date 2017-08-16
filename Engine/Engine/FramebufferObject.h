#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include <SFML/OpenGL.hpp>
#include <memory>

class FramebufferObject{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        FramebufferObject();
        ~FramebufferObject();
};
#endif

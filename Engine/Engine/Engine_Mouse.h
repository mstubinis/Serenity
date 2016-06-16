#pragma once
#ifndef ENGINE_MOUSE_H
#define ENGINE_MOUSE_H

#include <memory>

namespace sf{class Mouse;};

typedef unsigned int uint;

class Engine_Mouse final{
    private:
		class impl;
		std::unique_ptr<impl> m_i;
    public:
        Engine_Mouse();
        ~Engine_Mouse();

        void setPosition(uint x, uint y);
};
#endif
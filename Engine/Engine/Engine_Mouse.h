#pragma once
#ifndef ENGINE_MOUSE_H
#define ENGINE_MOUSE_H

#include <string>

namespace sf{class Mouse;};

typedef unsigned int uint;

class Engine_Mouse final{
    private:
        sf::Mouse* m_SFMLMouse;
    public:
        Engine_Mouse();
        ~Engine_Mouse();

        void setPosition(uint x, uint y);
};
#endif
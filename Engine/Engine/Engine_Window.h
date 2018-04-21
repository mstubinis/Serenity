#pragma once
#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

#include <memory>
#include <glm/fwd.hpp>
#include <SFML/Window.hpp>

typedef unsigned int uint;

class Engine_Window final{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        Engine_Window(const char* name,uint width,uint height);
        ~Engine_Window();
        const char* name() const;
        glm::uvec2 getSize();
        void setName(const char* name);
        void setSize(uint w, uint h);
        void setIcon(Texture* texture);
        void setIcon(const char* file);
        void setMouseCursorVisible(bool);
        void setKeyRepeatEnabled(bool);
        void setVerticalSyncEnabled(bool);
        void close();
        void requestFocus();
        void setActive(bool);
        uint getStyle();
        bool hasFocus();
        bool isOpen();
        bool isFullscreen();
        bool isActive();
        void setFullScreen(bool);
        void setStyle(uint style);
        void display();
        void keepMouseInWindow(bool=true);
        void setFramerateLimit(uint limit);
        uint getFramerateLimit() const;
        sf::Window* getSFMLHandle() const;
};
#endif

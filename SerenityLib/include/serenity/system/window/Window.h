#pragma once
#ifndef ENGINE_WINDOW_H
#define ENGINE_WINDOW_H

namespace sf {
    class Window;
};
namespace Engine::priv {
    class  EngineCore;
    class  EventManager;
    class  WindowData;
    class  WindowThread;
    class  ResourceManager;
    class  EngineEventHandler;
};
class  Texture;
struct EngineOptions;
class  Window;
class  Cursor;

#include <serenity/system/window/WindowData.h>
#include <serenity/dependencies/glm.h>

class Window final{
    friend class Engine::priv::EngineCore;
    friend class Engine::priv::EventManager;
    friend class Engine::priv::WindowData;
    friend class Engine::priv::WindowThread;
    friend class Engine::priv::ResourceManager;
    friend class Engine::priv::EngineEventHandler;
    private:
        Engine::priv::WindowData m_Data;

        //Whenever the window's size changes, this function executes. Different from SFML's onResize Event, this fires constantly if you are changing the size of the window via mouse dragging.
        //This currently only performs actions on the windows platform.
        void internal_on_dynamic_resize();

        void internal_restore_state();
        void internal_init() noexcept;
        bool internal_return_window_placement_cmd(uint32_t cmd) const noexcept;
        bool internal_execute_show_window(uint32_t cmd) noexcept;

        Window();
    public:
        ~Window();

        void init(const EngineOptions&) noexcept;

        [[nodiscard]] inline const std::string& name() const noexcept { return m_Data.m_WindowName; }

        [[nodiscard]] glm::uvec2 getSize();
        [[nodiscard]] glm::uvec2 getPosition();

        void setMouseCursor(const Cursor&) noexcept;

        void setJoystickProcessingActive(bool active);
        [[nodiscard]] bool isJoystickProcessingActive() const;

        [[nodiscard]] inline const glm::vec2& getMousePositionDifference() const noexcept { return m_Data.m_MouseDifference; }
        [[nodiscard]] inline const glm::vec2& getMousePositionPrevious() const noexcept { return m_Data.m_MousePosition_Previous; }
        [[nodiscard]] inline const glm::vec2& getMousePosition() const noexcept { return m_Data.m_MousePosition; }
        [[nodiscard]] inline double getMouseWheelDelta() const noexcept { return m_Data.m_MouseDelta; }
        [[nodiscard]] inline std::thread::id getOpenglThreadID() const noexcept { return m_Data.m_OpenGLThreadID; }
        [[nodiscard]] inline sf::WindowHandle getSystemHandle() const noexcept { return m_Data.m_SFMLWindow.getSystemHandle(); }
        [[nodiscard]] inline sf::RenderWindow& getSFMLHandle() noexcept { return m_Data.m_SFMLWindow; }
        [[nodiscard]] inline unsigned int getFramerateLimit() const noexcept { return m_Data.m_FramerateLimit; }

        bool pollEvents(sf::Event&);
        [[nodiscard]] bool hasFocus() const;
        [[nodiscard]] bool isOpen() const;
        [[nodiscard]] bool isVsyncEnabled() const;
        //returns true if the window is in fullscreen OR windowed fullscreen mode
        [[nodiscard]] bool isFullscreen() const;

        [[nodiscard]] bool isWindowOnSeparateThread() const;

        //returns true if the window is in windowed fullscreen mode
        [[nodiscard]] bool isFullscreenWindowed() const;

        //returns true if the window is in regular fullscreen mode
        [[nodiscard]] bool isFullscreenNonWindowed() const;

        //currently specific to windows os only
        [[nodiscard]] bool isMaximized() const noexcept;
        //currently specific to windows os only
        [[nodiscard]] bool isMinimized() const noexcept;

        [[nodiscard]] bool isActive() const;
        [[nodiscard]] bool isMouseKeptInWindow() const;

        void updateMousePosition(float x, float y, bool resetDifference = false, bool resetPreviousPosition = false);
        void updateMousePosition(const glm::vec2& position, bool resetDifference = false, bool resetPreviousPosition = false);

        void setName(const char* name);
        void setSize(uint32_t width, uint32_t height);
        void setIcon(const Texture&);
        void setIcon(const char* file);
        void setIcon(const std::string& file);
        void setMouseCursorVisible(bool);
        void setPosition(uint32_t x, uint32_t y);

        //currently specific to windows os only
        bool maximize() noexcept;
        //currently specific to windows os only
        bool minimize() noexcept;

        //If key repeat is enabled, you will receive repeated KeyPressed events while keeping a key pressed.
        //If it is disabled, you will only get a single event when the key is pressed.
        //Key repeat is enabled by default.
        void setKeyRepeatEnabled(bool);

        //Activating vertical synchronization will limit the number of frames displayed to the refresh rate of the monitor.
        //This can avoid some visual artifacts, and limit the framerate to a good value (but not constant across different computers).
        //Vertical synchronization is disabled by default.
        void setVerticalSyncEnabled(bool);

        //Close the window and destroy all the attached resources. After calling this function, the sf::Window instance
        //remains valid and you can call create() to recreate the window. All other functions such as pollEvent() or
        //display() will still work (i.e. you don't have to test isOpen() every time), and will have no effect on closed windows.
        void close();

        //Request the current window to be made the active foreground window. At any given time, only one window may have the input
        //focus to receive input events such as keystrokes or mouse events. If a window requests focus, it only hints to the
        //operating system, that it would like to be focused. The operating system is free to deny the request.
        //This is not to be confused with setActive().
        void requestFocus();

        //Activate or deactivate the window as the current target for OpenGL rendering. A window is active only on the current thread,
        //if you want to make it active on another thread you have to deactivate it on the previous thread first if it was active.
        //Only one window can be active on a thread at a time, thus the window previously active (if any) automatically gets deactivated.
        //This is not to be confused with requestFocus().
        bool setActive(bool active = true);

        //sets the window to be full screen
        bool setFullscreen(bool isFullscreen = true);

        //sets the window to be windowed fullscreen, this is a window with no style, stretched to the monitor's size
        bool setFullscreenWindowed(bool isFullscreen = true);

        /*
        Display the rendered elements to the screen. This is called automatically during the Engine's render loop. You may want to call this yourself in certain parts of the update loop
        when you want the window to display something and cannot wait for the render loop to begin, like non async loading.
        */
        void display();

        //Grab or release the mouse cursor. If set, grabs the mouse cursor inside this window's client area so it may no
        //longer be moved outside its bounds. Note that grabbing is only active while the window has focus.
        void keepMouseInWindow(bool = true);

        //Limit the framerate to a maximum fixed frequency. If a limit is set, the window will use a small delay after
        //each call to display() to ensure that the current frame lasted long enough to match the framerate limit.
        //SFML will try to match the given limit as much as it can, but since it internally uses sf::sleep,
        //whose precision depends on the underlying OS, the results may be a little unprecise as well
        //(for example, you can get 65 FPS when requesting 60).
        void setFramerateLimit(uint32_t limit);

};
#endif

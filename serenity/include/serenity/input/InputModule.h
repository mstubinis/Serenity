#pragma once
#ifndef ENGINE_INPUT_MODULE_H
#define ENGINE_INPUT_MODULE_H

class Window;
namespace Engine::priv {
    class EngineCore;
}

#include <serenity/input/Keyboard/KeyboardModule.h>
#include <serenity/input/Mouse/MouseModule.h>
#include <serenity/input/Joystick/JoystickModule.h>
#include <serenity/dependencies/glm.h>

namespace Engine::priv {
	class InputModule {
        friend class Engine::priv::EngineCore;
		private:
			KeyboardModule    m_KeyboardModule;
			MouseModule       m_MouseModule;
			JoystickModule    m_JoystickModule;
		public:
			InputModule();
			InputModule(const InputModule&)                = delete;
			InputModule& operator=(const InputModule&)     = delete;
			InputModule(InputModule&&) noexcept            = delete;
			InputModule& operator=(InputModule&&) noexcept = delete;

			void onEventKeyPressed(uint32_t key);
			void onEventKeyReleased(uint32_t key);
			void onEventMouseButtonPressed(uint32_t mouseButton);
			void onEventMouseButtonReleased(uint32_t mouseButton);

			void onWindowLostFocus();
			void onWindowGainedFocus();

            [[nodiscard]] inline const KeyboardModule& getKeyboardModule() const noexcept { return m_KeyboardModule; }
            [[nodiscard]] inline const MouseModule& getMouseModule() const noexcept { return m_MouseModule; }
            [[nodiscard]] inline const JoystickModule& getJoystickModule() const noexcept { return m_JoystickModule; }
	};
}
namespace Engine::input {
    [[nodiscard]] uint32_t getNumPressedKeys();
    [[nodiscard]] uint32_t getNumPressedMouseButtons();

    [[nodiscard]] bool isKeyDown(KeyboardKey);
    [[nodiscard]] bool isKeyDownOnce(KeyboardKey);
    [[nodiscard]] bool isKeyDownOnce(KeyboardKey, KeyboardKey);
    [[nodiscard]] bool isKeyDownOnce(KeyboardKey, KeyboardKey, KeyboardKey);
    [[nodiscard]] bool isKeyUp(KeyboardKey);

    [[nodiscard]] bool isCapsLockOn();
    [[nodiscard]] bool isNumLockOn();
    [[nodiscard]] bool isScrollLockOn();

    [[nodiscard]] bool isMouseButtonDown(MouseButton);
    [[nodiscard]] bool isMouseButtonDownOnce(MouseButton);

    [[nodiscard]] const glm::vec2& getMouseDifference();
    [[nodiscard]] const glm::vec2& getMousePositionPrevious();
    [[nodiscard]] const glm::vec2& getMousePosition();

    [[nodiscard]] const glm::vec2& getMouseDifference(Window&);
    [[nodiscard]] const glm::vec2& getMousePositionPrevious(Window&);
    [[nodiscard]] const glm::vec2& getMousePosition(Window&);

    [[nodiscard]] double getMouseWheelDelta(Window&);
    [[nodiscard]] double getMouseWheelDelta();

    void setMousePosition(float x, float y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(uint32_t x, uint32_t y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(const glm::vec2&, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(const glm::uvec2&, bool resetDifference = false, bool resetPreviousPosition = false);

    void setMousePosition(Window&, float x, float y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window&, uint32_t x, uint32_t y, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window&, const glm::vec2&, bool resetDifference = false, bool resetPreviousPosition = false);
    void setMousePosition(Window&, const glm::uvec2&, bool resetDifference = false, bool resetPreviousPosition = false);
}
namespace Engine {
    //just forward to Engine::input namespace

    [[nodiscard]] inline uint32_t getNumPressedKeys() noexcept { return Engine::input::getNumPressedKeys(); }

    [[nodiscard]] inline uint32_t getNumPressedMouseButtons() noexcept { return Engine::input::getNumPressedMouseButtons(); }

    [[nodiscard]] inline bool isKeyDown(KeyboardKey key) { return Engine::input::isKeyDown(key); }
    [[nodiscard]] inline bool isKeyDownOnce(KeyboardKey key) { return Engine::input::isKeyDownOnce(key); }
    [[nodiscard]] inline bool isKeyDownOnce(KeyboardKey key1, KeyboardKey key2) { return Engine::input::isKeyDownOnce(key1, key2); }
    [[nodiscard]] inline bool isKeyDownOnce(KeyboardKey key1, KeyboardKey key2, KeyboardKey key3) { return Engine::input::isKeyDownOnce(key1, key2, key3); }
    [[nodiscard]] inline bool isKeyUp(KeyboardKey key) { return Engine::input::isKeyUp(key); }

    [[nodiscard]] inline bool isCapsLockOn() { return Engine::input::isCapsLockOn(); }
    [[nodiscard]] inline bool isNumLockOn() { return Engine::input::isNumLockOn(); }
    [[nodiscard]] inline bool isScrollLockOn() { return Engine::input::isScrollLockOn(); }

    [[nodiscard]] inline bool isMouseButtonDown(MouseButton mouseButton) { return Engine::input::isMouseButtonDown(mouseButton); }
    [[nodiscard]] inline bool isMouseButtonDownOnce(MouseButton mouseButton) { return Engine::input::isMouseButtonDownOnce(mouseButton); }

    [[nodiscard]] inline const glm::vec2& getMouseDifference() noexcept { return Engine::input::getMouseDifference(); }
    [[nodiscard]] inline const glm::vec2& getMousePositionPrevious() noexcept { return Engine::input::getMousePositionPrevious(); }
    [[nodiscard]] inline const glm::vec2& getMousePosition() noexcept { return Engine::input::getMousePosition(); }

    [[nodiscard]] inline const glm::vec2& getMouseDifference(Window& window) noexcept { return Engine::input::getMouseDifference(window); }
    [[nodiscard]] inline const glm::vec2& getMousePositionPrevious(Window& window) noexcept { return Engine::input::getMousePositionPrevious(window); }
    [[nodiscard]] inline const glm::vec2& getMousePosition(Window& window) noexcept { return Engine::input::getMousePosition(window); }

    [[nodiscard]] inline double getMouseWheelDelta(Window& window) noexcept { return Engine::input::getMouseWheelDelta(window); }
    [[nodiscard]] inline double getMouseWheelDelta() noexcept { return Engine::input::getMouseWheelDelta(); }

    inline void setMousePosition(float x, float y, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(x, y, resetDifference, resetPreviousPosition); }
    inline void setMousePosition(uint32_t x, uint32_t y, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(x, y, resetDifference, resetPreviousPosition); }
    inline void setMousePosition(const glm::vec2& pos, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(pos, resetDifference, resetPreviousPosition); }
    inline void setMousePosition(const glm::uvec2& pos, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(pos, resetDifference, resetPreviousPosition); }

    inline void setMousePosition(Window& window, float x, float y, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(window, x, y, resetDifference, resetPreviousPosition); }
    inline void setMousePosition(Window& window, uint32_t x, uint32_t y, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(window, x, y, resetDifference, resetPreviousPosition); }
    inline void setMousePosition(Window& window, const glm::vec2& pos, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(window, pos, resetDifference, resetPreviousPosition); }
    inline void setMousePosition(Window& window, const glm::uvec2& pos, bool resetDifference = false, bool resetPreviousPosition = false) noexcept { return Engine::input::setMousePosition(window, pos, resetDifference, resetPreviousPosition); }
}

#endif
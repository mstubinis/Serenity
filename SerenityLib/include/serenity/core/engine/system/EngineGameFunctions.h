#pragma once
#ifndef ENGINE_ENGINE_GAME_FUNCTIONS_H
#define ENGINE_ENGINE_GAME_FUNCTIONS_H

class  Window;

namespace Game {
    void initResources();
    void initLogic();
    void update(const float dt);
    void render();
    void cleanup();

    void onResize(Window& window, uint32_t width, uint32_t height);
    void onWindowRequestedToBeClosed(Window& window);
    void onWindowClosed(Window& window);
    void onGameEnded();
    void onLostFocus(Window& window);
    void onGainedFocus(Window& window);
    void onTextEntered(Window& window, uint32_t unicode);
    void onKeyPressed(Window& window, uint32_t key);
    void onKeyReleased(Window& window, uint32_t key);
    void onMouseWheelScrolled(Window& window, float delta, int x, int y);
    void onMouseButtonPressed(Window& window, uint32_t button);
    void onMouseButtonReleased(Window& window, uint32_t button);
    void onMouseMoved(Window& window, float mouseX, float mouseY);
    void onMouseEntered(Window& window);
    void onMouseLeft(Window& window);
    void onPreUpdate(const float dt);
    void onPostUpdate(const float dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();
};

#endif
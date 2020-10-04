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

    void onResize(Window& window, unsigned int width, unsigned int height);
    void onWindowRequestedToBeClosed(Window& window);
    void onWindowClosed(Window& window);
    void onGameEnded();
    void onLostFocus(Window& window);
    void onGainedFocus(Window& window);
    void onTextEntered(Window& window, unsigned int unicode);
    void onKeyPressed(Window& window, unsigned int key);
    void onKeyReleased(Window& window, unsigned int key);
    void onMouseWheelScrolled(Window& window, float delta, int x, int y);
    void onMouseButtonPressed(Window& window, unsigned int button);
    void onMouseButtonReleased(Window& window, unsigned int button);
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
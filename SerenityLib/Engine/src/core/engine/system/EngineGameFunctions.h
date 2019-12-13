#pragma once
#ifndef ENGINE_ENGINE_GAME_FUNCTIONS_H
#define ENGINE_ENGINE_GAME_FUNCTIONS_H

namespace Game {
    void initResources();
    void initLogic();
    void update(const double& dt);
    void render();
    void cleanup();

    void onResize(const unsigned int& width, const unsigned int& height);
    void onClose();
    void onLostFocus();
    void onGainedFocus();
    void onTextEntered(const unsigned int& unicode);
    void onKeyPressed(const unsigned int& key);
    void onKeyReleased(const unsigned int& key);
    void onMouseWheelMoved(const int& delta);
    void onMouseButtonPressed(const unsigned int& button);
    void onMouseButtonReleased(const unsigned int& button);
    void onMouseMoved(const float& mouseX, const float& mouseY);
    void onMouseEntered();
    void onMouseLeft();
    void onPreUpdate(const double& dt);
    void onPostUpdate(const double& dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();

};
#endif
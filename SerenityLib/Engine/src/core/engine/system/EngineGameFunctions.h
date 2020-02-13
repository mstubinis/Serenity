#pragma once
#ifndef ENGINE_ENGINE_GAME_FUNCTIONS_H
#define ENGINE_ENGINE_GAME_FUNCTIONS_H

class  Window;

namespace Game {
    void initResources();
    void initLogic();
    void update(const float& dt);
    void render();
    void cleanup();

    void onResize(Window&, const unsigned int& width, const unsigned int& height);
    void onWindowRequestedToBeClosed(Window&);
    void onWindowClosed(Window&);
    void onGameEnded();
    void onLostFocus(Window&);
    void onGainedFocus(Window&);
    void onTextEntered(Window&, const unsigned int& unicode);
    void onKeyPressed(Window&, const unsigned int& key);
    void onKeyReleased(Window&, const unsigned int& key);
    void onMouseWheelScrolled(Window&, const float& delta, const int& x, const int& y);
    void onMouseButtonPressed(Window&, const unsigned int& button);
    void onMouseButtonReleased(Window&, const unsigned int& button);
    void onMouseMoved(Window&, const float& mouseX, const float& mouseY);
    void onMouseEntered(Window&);
    void onMouseLeft(Window&);
    void onPreUpdate(const float& dt);
    void onPostUpdate(const float& dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();

};
#endif
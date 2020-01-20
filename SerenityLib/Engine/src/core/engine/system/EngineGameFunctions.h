#pragma once
#ifndef ENGINE_ENGINE_GAME_FUNCTIONS_H
#define ENGINE_ENGINE_GAME_FUNCTIONS_H

class  Engine_Window;

namespace Game {
    void initResources();
    void initLogic();
    void update(const double& dt);
    void render();
    void cleanup();

    void onResize(Engine_Window&, const unsigned int& width, const unsigned int& height);
    void onWindowRequestedToBeClosed(Engine_Window&);
    void onWindowClosed(Engine_Window&);
    void onGameEnded();
    void onLostFocus(Engine_Window&);
    void onGainedFocus(Engine_Window&);
    void onTextEntered(Engine_Window&, const unsigned int& unicode);
    void onKeyPressed(Engine_Window&, const unsigned int& key);
    void onKeyReleased(Engine_Window&, const unsigned int& key);
    void onMouseWheelScrolled(Engine_Window&, const float& delta, const int& x, const int& y);
    void onMouseButtonPressed(Engine_Window&, const unsigned int& button);
    void onMouseButtonReleased(Engine_Window&, const unsigned int& button);
    void onMouseMoved(Engine_Window&, const float& mouseX, const float& mouseY);
    void onMouseEntered(Engine_Window&);
    void onMouseLeft(Engine_Window&);
    void onPreUpdate(const double& dt);
    void onPostUpdate(const double& dt);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();

};
#endif
#pragma once
#ifndef ENGINE_ENGINE_GAME_FUNCTIONS_H
#define ENGINE_ENGINE_GAME_FUNCTIONS_H

class  Window;
struct EngineOptions;
class  GameCore;

#include <serenity/system/TypeDefs.h>

namespace Game {
    void initResources(const EngineOptions&, GameCore&);
    void initLogic(const EngineOptions&, GameCore&);
    void update(const float dt, GameCore&);
    void render(GameCore&);
    void cleanup();

    void onResize(Window&, uint32_t width, uint32_t height, GameCore&);
    void onWindowClosed(Window&);
    void onLostFocus(Window&, GameCore&);
    void onGainedFocus(Window&, GameCore&);
    void onTextEntered(Window&, uint32_t unicode);
    void onKeyPressed(Window&, uint32_t key);
    void onKeyReleased(Window&, uint32_t key);
    void onMouseWheelScrolled(Window&, float delta, int x, int y);
    void onMouseButtonPressed(Window&, uint32_t button);
    void onMouseButtonReleased(Window&, uint32_t button);
    void onMouseMoved(Window&, float mouseX, float mouseY, GameCore&);
    void onMouseEntered(Window&, GameCore&);
    void onMouseLeft(Window&, GameCore&);
    void onPreUpdate(const float dt, GameCore&);
    void onPostUpdate(const float dt, GameCore&);
    void onJoystickButtonPressed();
    void onJoystickButtonReleased();
    void onJoystickMoved();
    void onJoystickConnected();
    void onJoystickDisconnected();
};

#endif
#include <serenity/lua/bindings/InputBindings.h>
#include <serenity/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaBinder.h>

#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <serenity/input/Mouse/MouseButtons.h>
#include <serenity/input/Joystick/JoystickAxises.h>

#include <serenity/input/InputModule.h>

namespace {
    inline void setEnum(lua_State* L, uint32_t enumUInt, const char* key) {
        luabridge::setGlobal(L, enumUInt, key);
    }
    bool isKeyDownOnceBinding(luabridge::LuaRef a1, luabridge::LuaRef a2, luabridge::LuaRef a3) {
        if (!a1.isNil()) {
            if (a2.isNil() && a3.isNil()) {
                return Engine::isKeyDownOnce(a1.cast<uint32_t>());
            } else if (!a2.isNil() && a3.isNil()) {
                return Engine::isKeyDownOnce(a1.cast<uint32_t>(), a2.cast<uint32_t>());
            } else {
                return Engine::isKeyDownOnce(a1.cast<uint32_t>(), a2.cast<uint32_t>(), a3.cast<uint32_t>());
            }
        }
        return false;
    }
}

void Engine::priv::lua::bindings::createBindingsInput(lua_State* L) {
#pragma region KeyboardKeys
    setEnum(L, KeyboardKey::Unknown, "KEY_UNKNOWN");
    setEnum(L, KeyboardKey::A, "KEY_A");
    setEnum(L, KeyboardKey::B, "KEY_B");
    setEnum(L, KeyboardKey::C, "KEY_C");
    setEnum(L, KeyboardKey::D, "KEY_D");
    setEnum(L, KeyboardKey::E, "KEY_E");
    setEnum(L, KeyboardKey::F, "KEY_F");
    setEnum(L, KeyboardKey::G, "KEY_G");
    setEnum(L, KeyboardKey::H, "KEY_H");
    setEnum(L, KeyboardKey::I, "KEY_I");
    setEnum(L, KeyboardKey::J, "KEY_J");
    setEnum(L, KeyboardKey::K, "KEY_K");
    setEnum(L, KeyboardKey::L, "KEY_L");
    setEnum(L, KeyboardKey::M, "KEY_M");
    setEnum(L, KeyboardKey::N, "KEY_N");
    setEnum(L, KeyboardKey::O, "KEY_O");
    setEnum(L, KeyboardKey::P, "KEY_P");
    setEnum(L, KeyboardKey::Q, "KEY_Q");
    setEnum(L, KeyboardKey::R, "KEY_R");
    setEnum(L, KeyboardKey::S, "KEY_S");
    setEnum(L, KeyboardKey::T, "KEY_T");
    setEnum(L, KeyboardKey::U, "KEY_U");
    setEnum(L, KeyboardKey::V, "KEY_V");
    setEnum(L, KeyboardKey::W, "KEY_W");
    setEnum(L, KeyboardKey::X, "KEY_X");
    setEnum(L, KeyboardKey::Y, "KEY_Y");
    setEnum(L, KeyboardKey::Z, "KEY_Z");
    setEnum(L, KeyboardKey::Num0, "KEY_0");
    setEnum(L, KeyboardKey::Num1, "KEY_1");
    setEnum(L, KeyboardKey::Num2, "KEY_2");
    setEnum(L, KeyboardKey::Num3, "KEY_3");
    setEnum(L, KeyboardKey::Num4, "KEY_4");
    setEnum(L, KeyboardKey::Num5, "KEY_5");
    setEnum(L, KeyboardKey::Num6, "KEY_6");
    setEnum(L, KeyboardKey::Num7, "KEY_7");
    setEnum(L, KeyboardKey::Num8, "KEY_8");
    setEnum(L, KeyboardKey::Num9, "KEY_9");
    setEnum(L, KeyboardKey::Escape, "KEY_ESC");
    setEnum(L, KeyboardKey::Escape, "KEY_ESCAPE");
    setEnum(L, KeyboardKey::LeftControl, "KEY_LEFT_CONTROL");
    setEnum(L, KeyboardKey::LeftShift, "KEY_LEFT_SHIFT");
    setEnum(L, KeyboardKey::LeftAlt, "KEY_LEFT_ALT");
    setEnum(L, KeyboardKey::LeftSystem, "KEY_LEFT_SYSTEM");
    setEnum(L, KeyboardKey::LeftControl, "KEY_L_CONTROL");
    setEnum(L, KeyboardKey::LeftShift, "KEY_L_SHIFT");
    setEnum(L, KeyboardKey::LeftAlt, "KEY_L_ALT");
    setEnum(L, KeyboardKey::LeftSystem, "KEY_L_SYSTEM");
    setEnum(L, KeyboardKey::RightControl, "KEY_RIGHT_CONTROL");
    setEnum(L, KeyboardKey::RightShift, "KEY_RIGHT_SHIFT");
    setEnum(L, KeyboardKey::RightAlt, "KEY_RIGHT_ALT");
    setEnum(L, KeyboardKey::RightSystem, "KEY_RIGHT_SYSTEM");
    setEnum(L, KeyboardKey::RightControl, "KEY_R_CONTROL");
    setEnum(L, KeyboardKey::RightShift, "KEY_R_SHIFT");
    setEnum(L, KeyboardKey::RightAlt, "KEY_R_ALT");
    setEnum(L, KeyboardKey::RightSystem, "KEY_R_SYSTEM");
    setEnum(L, KeyboardKey::Menu, "KEY_MENU");
    setEnum(L, KeyboardKey::LeftBracket, "KEY_LEFT_BRACKET");
    setEnum(L, KeyboardKey::LeftBracket, "KEY_L_BRACKET");
    setEnum(L, KeyboardKey::RightBracket, "KEY_RIGHT_BRACKET");
    setEnum(L, KeyboardKey::RightBracket, "KEY_R_BRACKET");
    setEnum(L, KeyboardKey::SemiColon, "KEY_SEMI_COLON");
    setEnum(L, KeyboardKey::Comma, "KEY_COMMA");
    setEnum(L, KeyboardKey::Period, "KEY_PERIOD");
    setEnum(L, KeyboardKey::Quote, "KEY_QUOTE");
    setEnum(L, KeyboardKey::Slash, "KEY_SLASH");
    setEnum(L, KeyboardKey::Slash, "KEY_FORWARD_SLASH");
    setEnum(L, KeyboardKey::BackSlash, "KEY_BACK_SLASH");
    setEnum(L, KeyboardKey::Tilde, "KEY_TILDE");
    setEnum(L, KeyboardKey::Equal, "KEY_EQUAL");
    setEnum(L, KeyboardKey::Equal, "KEY_EQUALS");
    setEnum(L, KeyboardKey::Dash, "KEY_DASH");
    setEnum(L, KeyboardKey::Dash, "KEY_HYPHEN");
    setEnum(L, KeyboardKey::Space, "KEY_SPACE");
    setEnum(L, KeyboardKey::Space, "KEY_SPACEBAR");
    setEnum(L, KeyboardKey::Return, "KEY_RETURN");
    setEnum(L, KeyboardKey::Return, "KEY_ENTER");
    setEnum(L, KeyboardKey::BackSpace, "KEY_BACK_SPACE");
    setEnum(L, KeyboardKey::Tab, "KEY_TAB");
    setEnum(L, KeyboardKey::PageUp, "KEY_PAGE_UP");
    setEnum(L, KeyboardKey::PageDown, "KEY_PAGE_DOWN");
    setEnum(L, KeyboardKey::End, "KEY_END");
    setEnum(L, KeyboardKey::Home, "KEY_HOME");
    setEnum(L, KeyboardKey::Insert, "KEY_INSERT");
    setEnum(L, KeyboardKey::Delete, "KEY_DELETE");
    setEnum(L, KeyboardKey::Add, "KEY_ADD");
    setEnum(L, KeyboardKey::Add, "KEY_PLUS");
    setEnum(L, KeyboardKey::Add, "KEY_PLUS_SIGN");
    setEnum(L, KeyboardKey::Subtract, "KEY_SUBTRACT");
    setEnum(L, KeyboardKey::Subtract, "KEY_MINUS");
    setEnum(L, KeyboardKey::Subtract, "KEY_MINUS_SIGN");
    setEnum(L, KeyboardKey::Multiply, "KEY_MULTIPLY");
    setEnum(L, KeyboardKey::Multiply, "KEY_STAR");
    setEnum(L, KeyboardKey::Divide, "KEY_DIVIDE");
    setEnum(L, KeyboardKey::LeftArrow, "KEY_LEFT_ARROW");
    setEnum(L, KeyboardKey::LeftArrow, "KEY_L_ARROW");
    setEnum(L, KeyboardKey::LeftArrow, "KEY_LEFT");
    setEnum(L, KeyboardKey::RightArrow, "KEY_RIGHT_ARROW");
    setEnum(L, KeyboardKey::RightArrow, "KEY_R_ARROW");
    setEnum(L, KeyboardKey::RightArrow, "KEY_RIGHT");
    setEnum(L, KeyboardKey::UpArrow, "KEY_UP_ARROW");
    setEnum(L, KeyboardKey::UpArrow, "KEY_U_ARROW");
    setEnum(L, KeyboardKey::UpArrow, "KEY_UP");
    setEnum(L, KeyboardKey::DownArrow, "KEY_DOWN_ARROW");
    setEnum(L, KeyboardKey::DownArrow, "KEY_D_ARROW");
    setEnum(L, KeyboardKey::DownArrow, "KEY_DOWN");
    setEnum(L, KeyboardKey::Numpad0, "KEY_NUM_0");
    setEnum(L, KeyboardKey::Numpad0, "KEY_NUMPAD_0");
    setEnum(L, KeyboardKey::Numpad1, "KEY_NUM_1");
    setEnum(L, KeyboardKey::Numpad1, "KEY_NUMPAD_1");
    setEnum(L, KeyboardKey::Numpad2, "KEY_NUM_2");
    setEnum(L, KeyboardKey::Numpad2, "KEY_NUMPAD_2");
    setEnum(L, KeyboardKey::Numpad3, "KEY_NUM_3");
    setEnum(L, KeyboardKey::Numpad3, "KEY_NUMPAD_3");
    setEnum(L, KeyboardKey::Numpad4, "KEY_NUM_4");
    setEnum(L, KeyboardKey::Numpad4, "KEY_NUMPAD_4");
    setEnum(L, KeyboardKey::Numpad5, "KEY_NUM_5");
    setEnum(L, KeyboardKey::Numpad5, "KEY_NUMPAD_5");
    setEnum(L, KeyboardKey::Numpad6, "KEY_NUM_6");
    setEnum(L, KeyboardKey::Numpad6, "KEY_NUMPAD_6");
    setEnum(L, KeyboardKey::Numpad7, "KEY_NUM_7");
    setEnum(L, KeyboardKey::Numpad7, "KEY_NUMPAD_7");
    setEnum(L, KeyboardKey::Numpad8, "KEY_NUM_8");
    setEnum(L, KeyboardKey::Numpad8, "KEY_NUMPAD_8");
    setEnum(L, KeyboardKey::Numpad9, "KEY_NUM_9");
    setEnum(L, KeyboardKey::Numpad9, "KEY_NUMPAD_9");
    setEnum(L, KeyboardKey::F1, "KEY_F1");
    setEnum(L, KeyboardKey::F2, "KEY_F2");
    setEnum(L, KeyboardKey::F3, "KEY_F3");
    setEnum(L, KeyboardKey::F4, "KEY_F4");
    setEnum(L, KeyboardKey::F5, "KEY_F5");
    setEnum(L, KeyboardKey::F6, "KEY_F6");
    setEnum(L, KeyboardKey::F7, "KEY_F7");
    setEnum(L, KeyboardKey::F8, "KEY_F8");
    setEnum(L, KeyboardKey::F9, "KEY_F9");
    setEnum(L, KeyboardKey::F10, "KEY_F10");
    setEnum(L, KeyboardKey::F11, "KEY_F11");
    setEnum(L, KeyboardKey::F12, "KEY_F12");
    setEnum(L, KeyboardKey::F13, "KEY_F13");
    setEnum(L, KeyboardKey::F14, "KEY_F14");
    setEnum(L, KeyboardKey::F15, "KEY_F15");
    setEnum(L, KeyboardKey::Pause, "KEY_PAUSE");
#pragma endregion

#pragma region MouseButtons
    setEnum(L, MouseButton::Unknown, "MOUSE_UNKNOWN");
    setEnum(L, MouseButton::Unknown, "MOUSE_BUTTON_UNKNOWN");
    setEnum(L, MouseButton::Left, "MOUSE_L");
    setEnum(L, MouseButton::Left, "MOUSE_LEFT");
    setEnum(L, MouseButton::Left, "MOUSE_BUTTON_LEFT");
    setEnum(L, MouseButton::Right, "MOUSE_R");
    setEnum(L, MouseButton::Right, "MOUSE_RIGHT");
    setEnum(L, MouseButton::Right, "MOUSE_BUTTON_RIGHT");
    setEnum(L, MouseButton::Middle, "MOUSE_MIDDLE");
    setEnum(L, MouseButton::Middle, "MOUSE_BUTTON_MIDDLE");
    setEnum(L, MouseButton::Middle, "MOUSE_SCROLL");
    setEnum(L, MouseButton::Middle, "MOUSE_BUTTON_SCROLL");
    setEnum(L, MouseButton::XButton1, "MOUSE_X1");
    setEnum(L, MouseButton::XButton1, "MOUSE_BUTTON_X1");
    setEnum(L, MouseButton::XButton1, "MOUSE_X_BUTTON_1");
    setEnum(L, MouseButton::XButton2, "MOUSE_X2");
    setEnum(L, MouseButton::XButton2, "MOUSE_BUTTON_X2");
    setEnum(L, MouseButton::XButton2, "MOUSE_X_BUTTON_2");
#pragma endregion

#pragma region JoystickAxises
    setEnum(L, JoystickAxis::X, "JOYSTICK_X");
    setEnum(L, JoystickAxis::Y, "JOYSTICK_Y");
    setEnum(L, JoystickAxis::Z, "JOYSTICK_Z");
    setEnum(L, JoystickAxis::R, "JOYSTICK_R");
    setEnum(L, JoystickAxis::U, "JOYSTICK_U");
    setEnum(L, JoystickAxis::V, "JOYSTICK_V");
    setEnum(L, JoystickAxis::PovX, "JOYSTICK_POVX");
    setEnum(L, JoystickAxis::PovY, "JOYSTICK_POVY");
    setEnum(L, JoystickAxis::PovX, "JOYSTICK_POV_X");
    setEnum(L, JoystickAxis::PovY, "JOYSTICK_POV_Y");
    setEnum(L, JoystickAxis::Unknown, "JOYSTICK_UNKNOWN");
#pragma endregion

    luabridge::getGlobalNamespace(L)
#pragma region KeyboardFunctions
        .addFunction("isUp", &Engine::isKeyUp)
        .addFunction("isKeyDown", &Engine::isKeyDown)
        .addFunction("isKeyDownOnce", isKeyDownOnceBinding)
        .addFunction("isCapsLockOn", &Engine::isCapsLockOn)
        .addFunction("isNumLockOn", &Engine::isNumLockOn)
        .addFunction("isScrollLockOn", &Engine::isScrollLockOn)
#pragma endregion

#pragma region MouseFunctions
        .addFunction("isMouseButtonDown", &Engine::isMouseButtonDown)
        .addFunction("isMouseButtonDownOnce", &Engine::isMouseButtonDownOnce)
#pragma endregion

#pragma region JoystickFunctions
#pragma endregion          
    ;
}
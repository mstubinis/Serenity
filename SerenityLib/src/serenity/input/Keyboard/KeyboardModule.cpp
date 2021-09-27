#include <serenity/input/Keyboard/KeyboardModule.h>
#include <SFML/Window/Keyboard.hpp>

#if defined(ENGINE_WINDOWS)
    #include <Windows.h>
#elif defined(ENGINE_APPLE)
#elif defined(ENGINE_LINUX)
    #include <X11/XKBlib.h>
#elif defined(ENGINE_FREE_BSD)
    #include <X11/XKBlib.h>
#elif defined(ENGINE_ANDROID)
#else

#endif

namespace {

}

void Engine::priv::KeyboardModule::onWindowLostFocus() noexcept {
    std::fill(std::begin(m_CurrKeyboardKeyStatus), std::end(m_CurrKeyboardKeyStatus), false);
    m_NumPressedKeys = 0;
}
void Engine::priv::KeyboardModule::onWindowGainedFocus() noexcept {
    m_NumPressedKeys = 0;
}
void Engine::priv::KeyboardModule::onKeyPressed(KeyboardKey key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_CurrKeyboardKeyStatus[key] = true;
    ++m_NumPressedKeys;
}
void Engine::priv::KeyboardModule::onKeyReleased(KeyboardKey key) noexcept {
    if (key == KeyboardKey::Unknown) {
        return;
    }
    m_CurrKeyboardKeyStatus[key] = false;
    m_NumPressedKeys = std::max(m_NumPressedKeys - 1, 0);
}
void Engine::priv::KeyboardModule::postUpdate() noexcept {
    m_PrevKeyboardKeyStatus = m_CurrKeyboardKeyStatus;
    m_CurrKeyboardKeyStatus[KeyboardKey::Unknown] = true;
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key) noexcept {
    return isKeyDown(key) && !m_PrevKeyboardKeyStatus[key];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key1, KeyboardKey key2) noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && !m_PrevKeyboardKeyStatus[key1];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key1, KeyboardKey key2, KeyboardKey key3) noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && isKeyDown(key3) && !m_PrevKeyboardKeyStatus[key1];
}

bool Engine::priv::KeyboardModule::isCapsLockOn() const {
#if defined(_WIN32)
    return (GetKeyState(VK_CAPITAL) & 0x0001) != 0;
#else
    Display* d = XOpenDisplay((char*)0);
    bool caps_state = false;
    if (d) {
        unsigned n;
        XkbGetIndicatorState(d, XkbUseCoreKbd, &n);
        caps_state = (n & 0x01) == 1;
    }
    return caps_state;
#endif
    return false;
}
bool Engine::priv::KeyboardModule::isScrollLockOn() const {
#if defined(_WIN32)
    return (GetKeyState(VK_SCROLL) & 0x0001) != 0;
#endif
    return false;
}
bool Engine::priv::KeyboardModule::isNumLockOn() const {
#if defined(_WIN32)
    return (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
#endif
    return false;
}
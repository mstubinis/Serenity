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
    void internal_reset_bits(auto& currKeyboardKeyStatus) {
        if constexpr (std::is_same_v<decltype(currKeyboardKeyStatus), std::vector<bool>>) {
            std::fill(std::begin(currKeyboardKeyStatus), std::end(currKeyboardKeyStatus), false);
        } else {
            currKeyboardKeyStatus.reset();
        }
    }
    void internal_construct(auto& prevKeyboardKeyStatus, auto& currKeyboardKeyStatus) {
        if constexpr (std::is_same_v<decltype(currKeyboardKeyStatus), std::vector<bool>>) {
            currKeyboardKeyStatus = Engine::priv::KeyboardModule::KeyStatusArray(KeyboardKey::_TOTAL, false);
            prevKeyboardKeyStatus = Engine::priv::KeyboardModule::KeyStatusArray(KeyboardKey::_TOTAL, false);
        } else {
            currKeyboardKeyStatus = Engine::priv::KeyboardModule::KeyStatusArray(false);
            prevKeyboardKeyStatus = Engine::priv::KeyboardModule::KeyStatusArray(false);
        }
    }
}

Engine::priv::KeyboardModule::KeyboardModule() {
    internal_construct(m_PrevKeyboardKeyStatus, m_CurrKeyboardKeyStatus);
}
void Engine::priv::KeyboardModule::onWindowLostFocus() noexcept {
    internal_reset_bits(m_CurrKeyboardKeyStatus);
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
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key) const noexcept {
    return isKeyDown(key) && !m_PrevKeyboardKeyStatus[key];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key1, KeyboardKey key2) const noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && !m_PrevKeyboardKeyStatus[key1];
}
bool Engine::priv::KeyboardModule::isKeyDownOnce(KeyboardKey key1, KeyboardKey key2, KeyboardKey key3) const noexcept {
    return isKeyDown(key1) && isKeyDown(key2) && isKeyDown(key3) && !m_PrevKeyboardKeyStatus[key1];
}

bool Engine::priv::KeyboardModule::isCapsLockOn() const noexcept {
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
bool Engine::priv::KeyboardModule::isScrollLockOn() const noexcept {
#if defined(_WIN32)
    return (GetKeyState(VK_SCROLL) & 0x0001) != 0;
#endif
    return false;
}
bool Engine::priv::KeyboardModule::isNumLockOn() const noexcept {
#if defined(_WIN32)
    return (GetKeyState(VK_NUMLOCK) & 0x0001) != 0;
#endif
    return false;
}
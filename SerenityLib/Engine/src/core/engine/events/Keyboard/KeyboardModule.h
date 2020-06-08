#pragma once
#ifndef ENGINE_EVENTS_KEYBOARD_MODULE_H
#define ENGINE_EVENTS_KEYBOARD_MODULE_H

#include <core/engine/events/Keyboard/KeyboardKeys.h>
#include <array>

namespace Engine::priv {
    class KeyboardModule {
        private:
            std::array<bool, KeyboardKey::_TOTAL>  m_KeyboardKeyStatus;
            unsigned int                           m_CurrentKeyboardKey  = static_cast<unsigned int>(KeyboardKey::Unknown);
            unsigned int                           m_PreviousKeyboardKey = static_cast<unsigned int>(KeyboardKey::Unknown);
            unsigned int                           m_NumPressedKeys      = 0U;
        public:
            KeyboardModule();
            virtual ~KeyboardModule();

            void onKeyPressed(unsigned int key);
            void onKeyReleased(unsigned int key);
            void onPostUpdate();
            void onClearEvents();

            KeyboardKey::Key getCurrentPressedKey() const;

            unsigned int getNumPressedKeys() const;

            bool isKeyDown(unsigned int key) const;
            bool isKeyDownOnce() const;

            bool isKeyDownOnce(unsigned int key);
            bool isKeyDownOnce(unsigned int key1, unsigned int key2);
            bool isKeyDownOnce(unsigned int key1, unsigned int key2, unsigned int key3);
    };
}

#endif
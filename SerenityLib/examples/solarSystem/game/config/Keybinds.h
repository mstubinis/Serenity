#pragma once
#ifndef GAME_CONFIG_KEYBINDS_H
#define GAME_CONFIG_KEYBINDS_H

#include <unordered_map>
#include <core/engine/events/Engine_EventIncludes.h>

struct KeybindEnum final { enum Enum {
    TargetCycleEnemy,
    TargetCycleFriendly,
    TargetNearestEnemy,
    TargetNearestCloakedEnemy,
    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    RollLeft,
    RollRight,
    CameraCockpit,
    CameraFollowTarget,
    CameraOrbit,
    CameraFreeform,
    ToggleFireAtWill,
    ToggleAntiCloakScan,
    ToggleCloak,
    ToggleHUD,
    ToggleWarpDrive,
    TogglePerk1,
    TogglePerk2,
    TogglePerk3,
    _TOTAL,
};};

class Keybind final {
    private:
        struct Bind final {
            KeyboardKey::Key m_FirstModifier;
            KeyboardKey::Key m_SecondModifier;
            KeyboardKey::Key m_Key;
            Bind() {
                m_FirstModifier = m_SecondModifier = m_Key = KeyboardKey::Unknown;
            }
            ~Bind() {

            }
        };
        std::vector<Keybind::Bind> binds;

        const bool isModifierPressedDown();
    public:
        Keybind();
        Keybind(const KeyboardKey::Key& key);
        Keybind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod);
        Keybind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod, const KeyboardKey::Key& second_mod);
        ~Keybind();

        void clearBind();

        void addBind(const KeyboardKey::Key& key);
        void addBind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod);
        void addBind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod, const KeyboardKey::Key& second_mod);


        const bool isPressedDown();
        const bool isPressedDownOnce();
};

class Keybinds final {
    private:
        static std::vector<Keybind> keybinds;
    public:
        static void init_from_options();

        static Keybind& getKeybind(const KeybindEnum::Enum& bind);
        static void setKeybind(const KeybindEnum::Enum& bind, Keybind& bind_value);

        static const bool isPressedDown(const KeybindEnum::Enum& bind);
        static const bool isPressedDownOnce(const KeybindEnum::Enum& bind);
};

#endif
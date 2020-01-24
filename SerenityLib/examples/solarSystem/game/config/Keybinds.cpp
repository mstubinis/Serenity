#include "Keybinds.h"

#include <core/engine/events/Engine_Events.h>

using namespace std;
using namespace Engine;

vector<Keybind> Keybinds::keybinds = []() {
    vector<Keybind> ret;
    ret.resize(KeybindEnum::_TOTAL);

    ret[KeybindEnum::TargetCycleEnemy]          = Keybind(KeyboardKey::T);

    ret[KeybindEnum::TargetCycleFriendly]       = Keybind(KeyboardKey::LeftShift, KeyboardKey::T);
    ret[KeybindEnum::TargetCycleFriendly].addBind(KeyboardKey::RightShift, KeyboardKey::T);

    ret[KeybindEnum::TargetNearestEnemy]        = Keybind(KeyboardKey::Y);
    ret[KeybindEnum::TargetNearestCloakedEnemy] = Keybind(KeyboardKey::G);
    ret[KeybindEnum::MoveForward]               = Keybind(KeyboardKey::W);
    ret[KeybindEnum::MoveBackward]              = Keybind(KeyboardKey::S);
    ret[KeybindEnum::MoveLeft]                  = Keybind(KeyboardKey::A);
    ret[KeybindEnum::MoveRight]                 = Keybind(KeyboardKey::D);
    ret[KeybindEnum::MoveUp]                    = Keybind(KeyboardKey::R);
    ret[KeybindEnum::MoveDown]                  = Keybind(KeyboardKey::F);
    ret[KeybindEnum::RollLeft]                  = Keybind(KeyboardKey::Q);
    ret[KeybindEnum::RollRight]                 = Keybind(KeyboardKey::E);
    ret[KeybindEnum::CameraCockpit]             = Keybind(KeyboardKey::F1);
    ret[KeybindEnum::CameraFollowTarget]        = Keybind(KeyboardKey::F2);
    ret[KeybindEnum::CameraOrbit]               = Keybind(KeyboardKey::F3);
    ret[KeybindEnum::CameraFreeform]            = Keybind(KeyboardKey::F4);
    ret[KeybindEnum::ToggleFireAtWill]          = Keybind(KeyboardKey::H);
    ret[KeybindEnum::ToggleAntiCloakScan]       = Keybind(KeyboardKey::I);
    ret[KeybindEnum::ToggleCloak]               = Keybind(KeyboardKey::C);

    ret[KeybindEnum::ToggleHUD]                 = Keybind(KeyboardKey::LeftAlt, KeyboardKey::X);
    ret[KeybindEnum::ToggleHUD].addBind(KeyboardKey::RightAlt, KeyboardKey::X);

    ret[KeybindEnum::ToggleWarpDrive]           = Keybind(KeyboardKey::L);

    ret[KeybindEnum::TogglePerk1]               = Keybind(KeyboardKey::Num1);
    ret[KeybindEnum::TogglePerk2]               = Keybind(KeyboardKey::Num2);
    ret[KeybindEnum::TogglePerk3]               = Keybind(KeyboardKey::Num3);

    return ret;
}();


Keybind::Keybind() {
    Keybind::Bind b;
    b.m_Key            = KeyboardKey::Unknown;
    b.m_FirstModifier  = KeyboardKey::Unknown;
    b.m_SecondModifier = KeyboardKey::Unknown;
    binds.emplace_back(b);
}
Keybind::Keybind(const KeyboardKey::Key& key) {
    addBind(key);
}
Keybind::Keybind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod) {
    addBind(key, first_mod);
}
Keybind::Keybind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod, const KeyboardKey::Key& second_mod) {
    addBind(key, first_mod, second_mod);
}
void Keybind::clearBind() {
    binds.clear();
}
void Keybind::addBind(const KeyboardKey::Key& key) {
    Keybind::Bind b;
    b.m_Key = key;
    b.m_FirstModifier = KeyboardKey::Unknown;
    b.m_SecondModifier = KeyboardKey::Unknown;
    binds.emplace_back(b);
}
void Keybind::addBind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod) {
    Keybind::Bind b;
    b.m_Key = key;
    b.m_FirstModifier = first_mod;
    b.m_SecondModifier = KeyboardKey::Unknown;
    binds.emplace_back(b);
}
void Keybind::addBind(const KeyboardKey::Key& key, const KeyboardKey::Key& first_mod, const KeyboardKey::Key& second_mod) {
    Keybind::Bind b;
    b.m_Key = key;
    b.m_FirstModifier = first_mod;
    b.m_SecondModifier = second_mod;
    binds.emplace_back(b);
}
Keybind::~Keybind() {
}
const bool Keybind::isModifierPressedDown() {
    if (!Engine::isKeyDown(KeyboardKey::LeftShift) && !Engine::isKeyDown(KeyboardKey::RightShift)) {
        if (!Engine::isKeyDown(KeyboardKey::LeftAlt) && !Engine::isKeyDown(KeyboardKey::RightAlt)) {
            if (!Engine::isKeyDown(KeyboardKey::LeftControl) && !Engine::isKeyDown(KeyboardKey::RightControl)) {
                if (!Engine::isKeyDown(KeyboardKey::LeftSystem) && !Engine::isKeyDown(KeyboardKey::RightSystem)) {
                    return false;
                }
            }
        }
    }
    return true;
}
const bool Keybind::isPressedDown() {
    for (auto& b : binds) {
        if (b.m_SecondModifier != KeyboardKey::Unknown && b.m_FirstModifier != KeyboardKey::Unknown) {
            if (Engine::isKeyDown(b.m_Key) && Engine::isKeyDown(b.m_FirstModifier) && Engine::isKeyDown(b.m_SecondModifier))
                return true;
        }else if (b.m_FirstModifier != KeyboardKey::Unknown) {
            if (Engine::isKeyDown(b.m_Key) && Engine::isKeyDown(b.m_FirstModifier))
                return true;
        }else {
            if (Engine::isKeyDown(b.m_Key))
                return true;
        }
    }
    return false;
}
const bool Keybind::isPressedDownOnce() {
    for (auto& b : binds) {
        if (b.m_SecondModifier != KeyboardKey::Unknown && b.m_FirstModifier != KeyboardKey::Unknown) {
            if (Engine::isKeyDownOnce(b.m_SecondModifier, b.m_FirstModifier, b.m_Key))
                return true;
        }else if (b.m_FirstModifier != KeyboardKey::Unknown) {
            if (Engine::isKeyDownOnce(b.m_FirstModifier, b.m_Key))
                return true;
        }else {
            if (Engine::isKeyDownOnce(b.m_Key)) {
                if (!isModifierPressedDown()) { //kinda expensive...
                    return true;
                }
            }
        }
    }
    return false;
}


void Keybinds::init_from_options() {
    //TODO: implement
}


Keybind& Keybinds::getKeybind(const KeybindEnum::Enum& bind) {
    return keybinds[bind];
}
void Keybinds::setKeybind(const KeybindEnum::Enum& bind, Keybind& bind_value) {
    keybinds[bind] = bind_value;
}

const bool Keybinds::isPressedDown(const KeybindEnum::Enum& bind) {
    return keybinds[bind].isPressedDown();
}
const bool Keybinds::isPressedDownOnce(const KeybindEnum::Enum& bind) {
    return keybinds[bind].isPressedDownOnce();
}
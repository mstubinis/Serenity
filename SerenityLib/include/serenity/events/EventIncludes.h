#pragma once
#ifndef ENGINE_EVENT_INCLUDES_H
#define ENGINE_EVENT_INCLUDES_H

#include <SFML/System.hpp>

#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <serenity/input/Mouse/MouseButtons.h>
#include <serenity/input/Joystick/JoystickAxises.h>
#include <serenity/system/Macros.h>

class EventType {
    public:
        enum Type : uint32_t {
            Unknown = 0,
            WindowResized,
            WindowGainedFocus,
            WindowLostFocus,

            EnumChanged0,
            EnumChanged1,
            EnumChanged2,
            EnumChanged3,
            EnumChanged4,
            EnumChanged5,
            EnumChanged6,
            EnumChanged7,
            EnumChanged8,
            EnumChanged9,
            EnumChanged10,
            EnumChanged11,
            EnumChanged12,
            EnumChanged13,
            EnumChanged14,
            EnumChanged15,
            EnumChanged16,
            EnumChanged17,
            EnumChanged18,
            EnumChanged19,
            EnumChanged20,
            EnumChanged21,
            EnumChanged22,
            EnumChanged23,
            EnumChanged24,
            EnumChanged25,
            EnumChanged26,
            EnumChanged27,
            EnumChanged28,
            EnumChanged29,
            EnumChanged30,
            EnumChanged31,
            EnumChanged32,
            EnumChanged33,
            EnumChanged34,
            EnumChanged35,
            EnumChanged36,
            EnumChanged37,
            EnumChanged38,
            EnumChanged39,
            EnumChanged40,
            EnumChanged41,
            EnumChanged42,
            EnumChanged43,
            EnumChanged44,
            EnumChanged45,
            EnumChanged46,
            EnumChanged47,
            EnumChanged48,
            EnumChanged49,
            EnumChanged50,
            EnumChanged51,
            EnumChanged52,
            EnumChanged53,
            EnumChanged54,
            EnumChanged55,
            EnumChanged56,
            EnumChanged57,
            EnumChanged58,
            EnumChanged59,
            EnumChanged60,
            EnumChanged61,
            EnumChanged62,
            EnumChanged63,
            EnumChanged64,
            EnumChanged65,
            EnumChanged66,
            EnumChanged67,
            EnumChanged68,
            EnumChanged69,
            EnumChanged70,
            EnumChanged71,
            EnumChanged72,
            EnumChanged73,
            EnumChanged74,
            EnumChanged75,
            EnumChanged76,
            EnumChanged77,
            EnumChanged78,
            EnumChanged79,
            EnumChanged80,
            EnumChanged81,
            EnumChanged82,
            EnumChanged83,
            EnumChanged84,
            EnumChanged85,
            EnumChanged86,
            EnumChanged87,
            EnumChanged88,
            EnumChanged89,
            EnumChanged90,
            EnumChanged91,
            EnumChanged92,
            EnumChanged93,
            EnumChanged94,
            EnumChanged95,
            EnumChanged96,
            EnumChanged97,
            EnumChanged98,
            EnumChanged99,
            EnumChanged100,
            EnumChanged101,
            EnumChanged102,
            EnumChanged103,
            EnumChanged104,
            EnumChanged105,
            EnumChanged106,
            EnumChanged107,
            EnumChanged108,
            EnumChanged109,
            EnumChanged110,
            EnumChanged111,
            EnumChanged112,
            EnumChanged113,
            EnumChanged114,
            EnumChanged115,
            EnumChanged116,
            EnumChanged117,
            EnumChanged118,
            EnumChanged119,
            EnumChanged120,
            EnumChanged121,
            EnumChanged122,
            EnumChanged123,
            EnumChanged124,
            EnumChanged125,
            EnumChanged126,
            EnumChanged127,
            EnumChanged128,
            EnumChanged129,
            EnumChanged130,
            EnumChanged131,
            EnumChanged132,
            EnumChanged133,
            EnumChanged134,
            EnumChanged135,
            EnumChanged136,
            EnumChanged137,
            EnumChanged138,
            EnumChanged139,
            EnumChanged140,
            EnumChanged141,
            EnumChanged142,
            EnumChanged143,
            EnumChanged144,
            EnumChanged145,
            EnumChanged146,
            EnumChanged147,
            EnumChanged148,
            EnumChanged149,
            EnumChanged150,
            EnumChanged151,
            EnumChanged152,
            EnumChanged153,
            EnumChanged154,
            EnumChanged155,
            EnumChanged156,
            EnumChanged157,
            EnumChanged158,
            EnumChanged159,
            EnumChanged160,
            EnumChanged161,
            EnumChanged162,
            EnumChanged163,
            EnumChanged164,
            EnumChanged165,
            EnumChanged166,
            EnumChanged167,
            EnumChanged168,
            EnumChanged169,
            EnumChanged170,
            EnumChanged171,
            EnumChanged172,
            EnumChanged173,
            EnumChanged174,
            EnumChanged175,
            EnumChanged176,
            EnumChanged177,
            EnumChanged178,
            EnumChanged179,
            EnumChanged180,
            EnumChanged181,
            EnumChanged182,
            EnumChanged183,
            EnumChanged184,
            EnumChanged185,
            EnumChanged186,
            EnumChanged187,
            EnumChanged188,
            EnumChanged189,
            EnumChanged190,
            EnumChanged191,
            EnumChanged192,
            EnumChanged193,
            EnumChanged194,
            EnumChanged195,
            EnumChanged196,
            EnumChanged197,
            EnumChanged198,
            EnumChanged199,
            EnumChanged200,

            SocketConnected,
            SocketDisconnected,
            ServerStarted,
            ServerShutdowned,
            ClientConnected,
            ClientDisconnected,
            /*
            PacketSent,
            PacketSentCustom,
            PacketReceived,
            PacketReceivedCustom,
            */
            WindowRequestedToBeClosed,
            WindowHasClosed,
            GameEnded,

            ResourceLoaded,
            ResourceUnloaded,

            WindowFullscreenChanged,
            SoundStatusChanged,
            KeyPressed,
            KeyReleased,
            MouseButtonPressed,
            MouseButtonReleased,
            MouseWheelMoved,
            MouseMoved,
            MouseEnteredWindow,
            MouseLeftWindow,
            JoystickConnected,
            JoystickDisconnected,
            JoystickButtonPressed,
            JoystickButtonReleased,
            JoystickMoved,
            TextEntered,
            SceneChanged,


            _TOTAL,
        };
        BUILD_ENUM_CLASS_MEMBERS(EventType, Type)
};

#endif
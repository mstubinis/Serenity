#include <serenity/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaBinder.h>

#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/sound/SoundModule.h>

#include <serenity/scene/Scene.h>
#include <serenity/scene/SceneOptions.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Viewport.h>
#include <serenity/scene/Skybox.h>

#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/components/Components.h>

using namespace Engine::priv;

namespace {
    void print(const std::string& s) { std::cout << s << '\n'; }

    void internal_build_event_enum_globals(lua_State* L) {
        luabridge::setGlobal(L, uint32_t(EventType::WindowResized), "Window_Resized");
        luabridge::setGlobal(L, uint32_t(EventType::WindowGainedFocus), "Window_Gained_Focus");
        luabridge::setGlobal(L, uint32_t(EventType::WindowLostFocus), "Window_Lost_Focus");

        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged0), "Enum_Changed_0");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged1), "Enum_Changed_1");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged2), "Enum_Changed_2");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged3), "Enum_Changed_3");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged4), "Enum_Changed_4");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged5), "Enum_Changed_5");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged6), "Enum_Changed_6");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged7), "Enum_Changed_7");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged8), "Enum_Changed_8");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged9), "Enum_Changed_9");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged10), "Enum_Changed_10");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged11), "Enum_Changed_11");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged12), "Enum_Changed_12");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged13), "Enum_Changed_13");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged14), "Enum_Changed_14");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged15), "Enum_Changed_15");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged16), "Enum_Changed_16");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged17), "Enum_Changed_17");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged18), "Enum_Changed_18");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged19), "Enum_Changed_19");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged20), "Enum_Changed_20");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged21), "Enum_Changed_21");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged22), "Enum_Changed_22");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged23), "Enum_Changed_23");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged24), "Enum_Changed_24");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged25), "Enum_Changed_25");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged26), "Enum_Changed_26");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged27), "Enum_Changed_27");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged28), "Enum_Changed_28");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged29), "Enum_Changed_29");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged30), "Enum_Changed_30");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged31), "Enum_Changed_31");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged32), "Enum_Changed_32");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged33), "Enum_Changed_33");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged34), "Enum_Changed_34");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged35), "Enum_Changed_35");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged36), "Enum_Changed_36");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged37), "Enum_Changed_37");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged38), "Enum_Changed_38");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged39), "Enum_Changed_39");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged40), "Enum_Changed_40");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged41), "Enum_Changed_41");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged42), "Enum_Changed_42");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged43), "Enum_Changed_43");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged44), "Enum_Changed_44");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged45), "Enum_Changed_45");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged46), "Enum_Changed_46");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged47), "Enum_Changed_47");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged48), "Enum_Changed_48");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged49), "Enum_Changed_49");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged50), "Enum_Changed_50");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged51), "Enum_Changed_51");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged52), "Enum_Changed_52");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged53), "Enum_Changed_53");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged54), "Enum_Changed_54");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged55), "Enum_Changed_55");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged56), "Enum_Changed_56");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged57), "Enum_Changed_57");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged58), "Enum_Changed_58");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged59), "Enum_Changed_59");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged60), "Enum_Changed_60");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged61), "Enum_Changed_61");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged62), "Enum_Changed_62");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged63), "Enum_Changed_63");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged64), "Enum_Changed_64");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged65), "Enum_Changed_65");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged66), "Enum_Changed_66");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged67), "Enum_Changed_67");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged68), "Enum_Changed_68");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged69), "Enum_Changed_69");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged70), "Enum_Changed_70");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged71), "Enum_Changed_71");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged72), "Enum_Changed_72");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged73), "Enum_Changed_73");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged74), "Enum_Changed_74");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged75), "Enum_Changed_75");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged76), "Enum_Changed_76");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged77), "Enum_Changed_77");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged78), "Enum_Changed_78");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged79), "Enum_Changed_79");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged80), "Enum_Changed_80");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged81), "Enum_Changed_81");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged82), "Enum_Changed_82");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged83), "Enum_Changed_83");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged84), "Enum_Changed_84");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged85), "Enum_Changed_85");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged86), "Enum_Changed_86");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged87), "Enum_Changed_87");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged88), "Enum_Changed_88");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged89), "Enum_Changed_89");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged90), "Enum_Changed_90");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged91), "Enum_Changed_91");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged92), "Enum_Changed_92");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged93), "Enum_Changed_93");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged94), "Enum_Changed_94");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged95), "Enum_Changed_95");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged96), "Enum_Changed_96");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged97), "Enum_Changed_97");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged98), "Enum_Changed_98");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged99), "Enum_Changed_99");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged100), "Enum_Changed_100");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged101), "Enum_Changed_101");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged102), "Enum_Changed_102");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged103), "Enum_Changed_103");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged104), "Enum_Changed_104");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged105), "Enum_Changed_105");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged106), "Enum_Changed_106");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged107), "Enum_Changed_107");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged108), "Enum_Changed_108");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged109), "Enum_Changed_109");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged110), "Enum_Changed_110");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged111), "Enum_Changed_111");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged112), "Enum_Changed_112");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged113), "Enum_Changed_113");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged114), "Enum_Changed_114");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged115), "Enum_Changed_115");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged116), "Enum_Changed_116");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged117), "Enum_Changed_117");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged118), "Enum_Changed_118");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged119), "Enum_Changed_119");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged120), "Enum_Changed_120");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged121), "Enum_Changed_121");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged122), "Enum_Changed_122");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged123), "Enum_Changed_123");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged124), "Enum_Changed_124");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged125), "Enum_Changed_125");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged126), "Enum_Changed_126");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged127), "Enum_Changed_127");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged128), "Enum_Changed_128");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged129), "Enum_Changed_129");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged130), "Enum_Changed_130");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged131), "Enum_Changed_131");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged132), "Enum_Changed_132");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged133), "Enum_Changed_133");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged134), "Enum_Changed_134");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged135), "Enum_Changed_135");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged136), "Enum_Changed_136");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged137), "Enum_Changed_137");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged138), "Enum_Changed_138");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged139), "Enum_Changed_139");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged140), "Enum_Changed_140");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged141), "Enum_Changed_141");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged142), "Enum_Changed_142");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged143), "Enum_Changed_143");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged144), "Enum_Changed_144");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged145), "Enum_Changed_145");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged146), "Enum_Changed_146");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged147), "Enum_Changed_147");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged148), "Enum_Changed_148");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged149), "Enum_Changed_149");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged150), "Enum_Changed_150");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged151), "Enum_Changed_151");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged152), "Enum_Changed_152");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged153), "Enum_Changed_153");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged154), "Enum_Changed_154");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged155), "Enum_Changed_155");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged156), "Enum_Changed_156");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged157), "Enum_Changed_157");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged158), "Enum_Changed_158");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged159), "Enum_Changed_159");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged160), "Enum_Changed_160");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged161), "Enum_Changed_161");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged162), "Enum_Changed_162");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged163), "Enum_Changed_163");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged164), "Enum_Changed_164");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged165), "Enum_Changed_165");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged166), "Enum_Changed_166");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged167), "Enum_Changed_167");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged168), "Enum_Changed_168");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged169), "Enum_Changed_169");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged170), "Enum_Changed_170");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged171), "Enum_Changed_171");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged172), "Enum_Changed_172");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged173), "Enum_Changed_173");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged174), "Enum_Changed_174");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged175), "Enum_Changed_175");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged176), "Enum_Changed_176");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged177), "Enum_Changed_177");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged178), "Enum_Changed_178");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged179), "Enum_Changed_179");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged180), "Enum_Changed_180");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged181), "Enum_Changed_181");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged182), "Enum_Changed_182");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged183), "Enum_Changed_183");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged184), "Enum_Changed_184");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged185), "Enum_Changed_185");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged186), "Enum_Changed_186");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged187), "Enum_Changed_187");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged188), "Enum_Changed_188");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged189), "Enum_Changed_189");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged190), "Enum_Changed_190");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged191), "Enum_Changed_191");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged192), "Enum_Changed_192");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged193), "Enum_Changed_193");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged194), "Enum_Changed_194");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged195), "Enum_Changed_195");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged196), "Enum_Changed_196");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged197), "Enum_Changed_197");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged198), "Enum_Changed_198");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged199), "Enum_Changed_199");
        luabridge::setGlobal(L, uint32_t(EventType::EnumChanged200), "Enum_Changed_200");

        luabridge::setGlobal(L, uint32_t(EventType::SocketConnected), "Socket_Connected");
        luabridge::setGlobal(L, uint32_t(EventType::SocketDisconnected), "Socket_Disconnected");
        luabridge::setGlobal(L, uint32_t(EventType::ServerStarted), "Server_Started");
        luabridge::setGlobal(L, uint32_t(EventType::ServerShutdowned), "Server_Shutdowned");
        luabridge::setGlobal(L, uint32_t(EventType::ClientConnected), "Client_Connected");
        luabridge::setGlobal(L, uint32_t(EventType::ClientDisconnected), "Client_Disconnected");
        //luabridge::setGlobal(L, uint32_t(EventType::PacketSent), "Packet_Sent");
        //luabridge::setGlobal(L, uint32_t(EventType::PacketSentCustom), "Packet_Sent_Custom");
        //luabridge::setGlobal(L, uint32_t(EventType::PacketReceived), "Packet_Received"); 
        //luabridge::setGlobal(L, uint32_t(EventType::PacketReceivedCustom), "Packet_Received_Custom");
        luabridge::setGlobal(L, uint32_t(EventType::WindowRequestedToBeClosed), "Window_Requested_To_Be_Closed");
        luabridge::setGlobal(L, uint32_t(EventType::WindowHasClosed), "Window_Has_Closed");
        luabridge::setGlobal(L, uint32_t(EventType::GameEnded), "Game_Ended");
        luabridge::setGlobal(L, uint32_t(EventType::ResourceLoaded), "Resource_Loaded");
        luabridge::setGlobal(L, uint32_t(EventType::ResourceUnloaded), "Resource_Unloaded");
        luabridge::setGlobal(L, uint32_t(EventType::WindowFullscreenChanged), "Window_Fullscreen_Changed");
        luabridge::setGlobal(L, uint32_t(EventType::SoundStatusChanged), "Sound_Status_Changed");
        luabridge::setGlobal(L, uint32_t(EventType::KeyPressed), "Key_Pressed");
        luabridge::setGlobal(L, uint32_t(EventType::KeyReleased), "Key_Released");
        luabridge::setGlobal(L, uint32_t(EventType::MouseButtonPressed), "Mouse_Button_Pressed");
        luabridge::setGlobal(L, uint32_t(EventType::MouseButtonReleased), "Mouse_Button_Released");
        luabridge::setGlobal(L, uint32_t(EventType::MouseWheelMoved), "Mouse_Wheel_Moved");
        luabridge::setGlobal(L, uint32_t(EventType::MouseMoved), "Mouse_Moved");
        luabridge::setGlobal(L, uint32_t(EventType::MouseEnteredWindow), "Mouse_Entered_Window");
        luabridge::setGlobal(L, uint32_t(EventType::MouseLeftWindow), "Mouse_Left_Window");
        luabridge::setGlobal(L, uint32_t(EventType::JoystickConnected), "Joystick_Connected");
        luabridge::setGlobal(L, uint32_t(EventType::JoystickDisconnected), "Joystick_Disconnected");
        luabridge::setGlobal(L, uint32_t(EventType::JoystickButtonPressed), "Joystick_Button_Pressed");
        luabridge::setGlobal(L, uint32_t(EventType::JoystickButtonReleased), "Joystick_Button_Released");
        luabridge::setGlobal(L, uint32_t(EventType::JoystickMoved), "Joystick_Moved");
        luabridge::setGlobal(L, uint32_t(EventType::TextEntered), "Text_Entered");
        luabridge::setGlobal(L, uint32_t(EventType::SceneChanged), "Scene_Changed");
    }
}
LUABinder::LUABinder()
    : m_LUA_STATE{ std::make_unique<LUAState>() }
{
    lua_State* L{ m_LUA_STATE->getState() };

    internal_build_event_enum_globals(L);

    luabridge::getGlobalNamespace(L)
        .addFunction("print", &print)
        .addFunction("getDeltaTime", &Engine::Resources::dt)
        .addFunction("dt", &Engine::Resources::dt)
        .addFunction("addOnUpdateFunction", &Engine::lua::addOnUpdateFunction)
        .addFunction("addOnEventFunction", &Engine::lua::addOnEventFunction)
        .addFunction("playSoundEffect", &Engine::Sound::playEffect)
        .addFunction("playSoundMusic", &Engine::Sound::playMusic)
        .addFunction("stopAllSoundEffects", &Engine::Sound::stop_all_effects)
        .addFunction("stopAllSoundMusic", &Engine::Sound::stop_all_music)

#pragma region Matrices
        //glm mat4 TODO: add more to this
        .beginClass<glm::mat4>("mat4")

        .endClass()
        //glm mat3 TODO: add more to this
        .beginClass<glm::mat3>("mat3")

        .endClass()
        //glm mat2 TODO: add more to this
        .beginClass<glm::mat2>("mat2")

        .endClass()
#pragma endregion

#pragma region Vectors



        //glm ivec2 TODO: add more to this
        .beginClass<glm::ivec2>("ivec2")
            .addConstructor<void(*)(int, int)>()
            .addData("x", &glm::ivec2::x)
            .addData("y", &glm::ivec2::y)
        .endClass()
        //glm ivec3 TODO: add more to this
        .beginClass<glm::ivec3>("ivec3")
            .addConstructor<void(*)(int, int, int)>()
            .addData("x", &glm::ivec3::x)
            .addData("y", &glm::ivec3::y)
            .addData("z", &glm::ivec3::z)
        .endClass()
        //glm ivec4 TODO: add more to this
        .beginClass<glm::ivec4>("ivec4")
            .addConstructor<void(*)(int, int, int, int)>()
            .addData("x", &glm::ivec4::x)
            .addData("y", &glm::ivec4::y)
            .addData("z", &glm::ivec4::z)
            .addData("w", &glm::ivec4::w)
        .endClass()




        //glm dvec2 TODO: add more to this
        .beginClass<glm::dvec2>("dvec2")
            .addConstructor<void(*)(double, double)>()
            .addData("x", &glm::dvec2::x)
            .addData("y", &glm::dvec2::y)
        .endClass()
        //glm dvec3 TODO: add more to this
        .beginClass<glm::dvec3>("dvec3")
            .addConstructor<void(*)(double, double, double)>()
            .addData("x", &glm::dvec3::x)
            .addData("y", &glm::dvec3::y)
            .addData("z", &glm::dvec3::z)
        .endClass()
        //glm dvec4 TODO: add more to this
        .beginClass<glm::dvec4>("dvec4")
            .addConstructor<void(*)(double, double, double, double)>()
            .addData("x", &glm::dvec4::x)
            .addData("y", &glm::dvec4::y)
            .addData("z", &glm::dvec4::z)
            .addData("w", &glm::dvec4::w)
        .endClass()




        //glm vec2 TODO: add more to this
        .beginClass<glm::vec2>("fvec2")
            .addConstructor<void(*)(float, float)>()
            .addData("x", &glm::vec2::x)
            .addData("y", &glm::vec2::y)
        .endClass()
        //glm vec3 TODO: add more to this
        .beginClass<glm::vec3>("fvec3")
            .addConstructor<void(*)(float, float, float)>()
            .addData("x", &glm::vec3::x)
            .addData("y", &glm::vec3::y)
            .addData("z", &glm::vec3::z)
        .endClass()
        //glm vec4 TODO: add more to this
        .beginClass<glm::vec4>("fvec4")
            .addConstructor<void(*)(float, float, float, float)>()
            .addData("x", &glm::vec4::x)
            .addData("y", &glm::vec4::y)
            .addData("z", &glm::vec4::z)
            .addData("w", &glm::vec4::w)
        .endClass()

        //glm vec2 TODO: add more to this
        .beginClass<glm_vec2>("vec2")
            .addConstructor<void(*)(decimal, decimal)>()
            .addData("x", &glm_vec2::x)
            .addData("y", &glm_vec2::y)
        .endClass()
        //glm vec3 TODO: add more to this
        .beginClass<glm_vec3>("vec3")
            .addConstructor<void(*)(decimal, decimal, decimal)>()
            .addData("x", &glm_vec3::x)
            .addData("y", &glm_vec3::y)
            .addData("z", &glm_vec3::z)
        .endClass()
        //glm vec4 TODO: add more to this
        .beginClass<glm_vec4>("vec4")
            .addConstructor<void(*)(decimal, decimal, decimal, decimal)>()
            .addData("x", &glm_vec4::x)
            .addData("y", &glm_vec4::y)
            .addData("z", &glm_vec4::z)
            .addData("w", &glm_vec4::w)
        .endClass()
        //bullet vector3 TODO: add more to this
        .beginClass<btVector3>("btVector3")
            .addConstructor<void(*)(const btScalar&, const btScalar&, const btScalar&)>()
            .addProperty("x", &btVector3::x, &btVector3::setX)
            .addProperty("y", &btVector3::y, &btVector3::setY)
            .addProperty("z", &btVector3::z, &btVector3::setZ)
            .addProperty("w", &btVector3::w, &btVector3::setW)
            .addFunction("angle", &btVector3::angle)
            .addFunction("cross", &btVector3::cross)
            .addFunction("absolute", &btVector3::absolute)
            .addFunction("closestAxis", &btVector3::closestAxis)
            .addFunction("dot", &btVector3::dot)
            .addFunction("distance", &btVector3::distance)
            .addFunction("distance2", &btVector3::distance2)
            .addFunction("isZero", &btVector3::isZero)
            .addFunction("rotate", &btVector3::rotate)
            .addFunction("triple", &btVector3::triple)
            .addFunction("dot3", &btVector3::dot3)
            .addFunction("lerp", &btVector3::lerp)
            .addFunction("minAxis", &btVector3::minAxis)
            .addFunction("maxAxis", &btVector3::maxAxis)
            .addFunction("normalize",&btVector3::normalize)
            .addFunction("norm", &btVector3::norm)
            .addFunction("normalized", &btVector3::normalized)
            .addFunction("furthestAxis",&btVector3::furthestAxis)
        .endClass()
#pragma endregion

#pragma region Event
        //event stuff
        .beginClass<Event>("Event")
            .addData("type", &Event::type, false)
            .addData("eventWindowResized", &Event::eventWindowResized, false)
            .addData("eventWindowFullscreenChanged", &Event::eventWindowFullscreenChanged, false)
            .addData("eventKeyboard", &Event::eventKeyboard, false)
            .addData("eventTextEntered", &Event::eventTextEntered, false)
            .addData("eventMouseButton", &Event::eventMouseButton, false)
            .addData("eventMouseMoved", &Event::eventMouseMoved, false)
            .addData("eventMouseWheel", &Event::eventMouseWheel, false)
            .addData("eventJoystickMoved", &Event::eventJoystickMoved, false)
            .addData("eventJoystickButton", &Event::eventJoystickButton, false)
            .addData("eventJoystickConnection", &Event::eventJoystickConnection, false)
            .addData("eventSoundStatusChanged", &Event::eventSoundStatusChanged, false)
            .addData("eventSceneChanged", &Event::eventSceneChanged, false)
            .addData("eventResource", &Event::eventResource, false)
            .addData("eventSocket", &Event::eventSocket, false)
            .addData("eventServer", &Event::eventServer, false)
            .addData("eventClient", &Event::eventClient, false)
        .endClass()
        .beginClass<Engine::priv::EventEnum>("EventEnum")
            .addData("enum", &Engine::priv::EventEnum::enumValue, false)
            //.addData("ptr", &Engine::priv::EventEnum::enumPtr, false)
        .endClass()
        .beginClass<Engine::priv::EventSocket>("EventSocket")
            .addData("localPort", &Engine::priv::EventSocket::localPort, false)
            .addData("remotePort", &Engine::priv::EventSocket::remotePort, false)
            .addData("type", &Engine::priv::EventSocket::type, false)
            //.addProperty("remoteIP", &Engine::priv::EventSocket::ipToString)
        .endClass()
        .beginClass<Engine::priv::EventServer>("EventServer")
        .endClass()
        .beginClass<Engine::priv::EventClient>("EventClient")
        .endClass()
        .beginClass<Engine::priv::EventResource>("EventResource")
        .endClass()
        .beginClass<Engine::priv::EventWindowResized>("EventWindowResized")
            .addData("width", &Engine::priv::EventWindowResized::width, false)
            .addData("height", &Engine::priv::EventWindowResized::height, false)
        .endClass()
        .beginClass<Engine::priv::EventWindowFullscreenChanged>("EventWindowFullscreenChanged")
            .addData("isFullscreen", &Engine::priv::EventWindowFullscreenChanged::isFullscreen, false)
        .endClass()
        .beginClass<Engine::priv::EventKeyboard>("EventKeyboard")
            .addData("key", &Engine::priv::EventKeyboard::key, false)
            .addData("alt", &Engine::priv::EventKeyboard::alt, false)
            .addData("shift", &Engine::priv::EventKeyboard::shift, false)
            .addData("system", &Engine::priv::EventKeyboard::system, false)
            .addData("control", &Engine::priv::EventKeyboard::control, false)
        .endClass()
        .beginClass<Engine::priv::EventTextEntered>("EventTextEntered")
            .addData("unicode", &Engine::priv::EventTextEntered::unicode, false)
            //.addProperty("text", &Engine::priv::EventTextEntered::convert, false)
        .endClass()
        .beginClass<Engine::priv::EventMouseButton>("EventMouseButton")
            .addData("button", &Engine::priv::EventMouseButton::button, false)
            .addData("x", &Engine::priv::EventMouseButton::x, false)
            .addData("y", &Engine::priv::EventMouseButton::y, false)
        .endClass()
        .beginClass<Engine::priv::EventMouseMove>("EventMouseMove")
            .addData("x", &Engine::priv::EventMouseMove::x, false)
            .addData("y", &Engine::priv::EventMouseMove::y, false)
        .endClass()
        .beginClass<Engine::priv::EventMouseWheel>("EventMouseWheel")
            .addData("delta", &Engine::priv::EventMouseWheel::delta, false)
            .addData("x", &Engine::priv::EventMouseWheel::x, false)
            .addData("y", &Engine::priv::EventMouseWheel::y, false)
        .endClass()
        .beginClass<Engine::priv::EventJoystickMoved>("EventJoystickMoved")
             .addData("id", &Engine::priv::EventJoystickMoved::joystickID, false)
             .addData("position", &Engine::priv::EventJoystickMoved::position, false)
             .addData("axis", &Engine::priv::EventJoystickMoved::axis, false)
        .endClass()
        .beginClass<Engine::priv::EventJoystickButton>("EventJoystickButton")
            .addData("id", &Engine::priv::EventJoystickButton::joystickID, false)
            .addData("button", &Engine::priv::EventJoystickButton::button, false)
        .endClass()
        .beginClass<Engine::priv::EventJoystickConnection>("EventJoystickConnection")
            .addData("id", &Engine::priv::EventJoystickConnection::joystickID, false)
        .endClass()
        .beginClass<Engine::priv::EventSoundStatusChanged>("EventSoundStatusChanged")
            .addData("status", &Engine::priv::EventSoundStatusChanged::status, false)
        .endClass()
        .beginClass<Engine::priv::EventSceneChanged>("EventSceneChanged")
        .endClass()
#pragma endregion

#pragma region Camera
        //camera stuff
        .beginClass<Camera>("Camera")
            .addFunction("getAngle", &Camera::getAngle)
            .addFunction("setAngle", &Camera::setAngle)
            .addFunction("getAspectRatio", &Camera::getAspectRatio)
            .addFunction("setAspectRatio", &Camera::setAspectRatio)
            .addFunction("getNear", &Camera::getNear)
            .addFunction("setNear", &Camera::setNear)
            .addFunction("getFar", &Camera::getFar)
            .addFunction("setFar", &Camera::setFar)
            .addFunction("getView", &Camera::getView)
            .addFunction("setView", &Camera::setViewMatrix)
            .addFunction("getProjection", &Camera::getProjection)
            .addFunction("setProjection", &Camera::setProjectionMatrix)
        .endClass()
#pragma endregion

        //skybox stuff
        .beginClass<Skybox>("Skybox")

        .endClass()
        //viewport stuff
        .beginClass<Viewport>("Viewport")
            .addFunction("getId", &Viewport::getId)
            .addFunction("getScene", &Viewport::getScene)
            .addFunction("getCamera", &Viewport::getCamera)
            .addFunction("setCamera", &Viewport::setCamera)
            //.addFunction("setID", &Viewport::setID)
            .addFunction("activate", &Viewport::activate)
            .addFunction("activateDepthMask", &Viewport::activateDepthMask)
            .addFunction("setDepthMaskValue", &Viewport::setDepthMaskValue)
            .addFunction("setAspectRatioSynced", &Viewport::setAspectRatioSynced)
            .addFunction("isAspectRatioSynced", &Viewport::isAspectRatioSynced)
            .addFunction("isDepthMaskActive", &Viewport::isDepthMaskActive)
            .addFunction("getBackgroundColor", &Viewport::getBackgroundColor)
            .addFunction("setBackgroundColor", static_cast<void(Viewport::*)(float, float, float, float)>(&Viewport::setBackgroundColor))
            .addFunction("setViewportDimensions", &Viewport::setViewportDimensions)
            .addFunction("getViewportDimensions", &Viewport::getViewportDimensions)
        .endClass()
        //scene stuff
        .beginClass<Scene>("Scene")
            //.addProperty("name", &Scene::name, &Scene::setName)
            .addFunction("setName", static_cast<void(Scene::*)(std::string_view)>(&Scene::setName))
            .addFunction("createEntity", &Scene::createEntity)
            .addFunction("id", &Scene::id)
            .addFunction("setActiveCamera", &Scene::setActiveCamera)
            .addFunction("getActiveCamera", &Scene::getActiveCamera)
            .addFunction("centerSceneToObject", &Scene::centerSceneToObject)
            .addFunction("getBackgroundColor", &Scene::getBackgroundColor)
            .addFunction("setBackgroundColor", static_cast<void(Scene::*)(float, float, float, float)>(&Scene::setBackgroundColor))
            .addFunction("setGlobalIllumination", static_cast<void(Scene::*)(float, float, float)>(&Scene::setGlobalIllumination))
            .addFunction("getGlobalIllumination", &Scene::getGlobalIllumination)
        .endClass()
        .addFunction("getCurrentScene", &Engine::Resources::getCurrentScene)
        .addFunction("setCurrentScene", static_cast<bool(*)(Scene*)>(&Engine::Resources::setCurrentScene))
        .addFunction("setCurrentSceneByName", static_cast<bool(*)(std::string_view)>(&Engine::Resources::setCurrentScene))
        //entity stuff
        .beginClass<Entity>("Entity")
            .addFunction("scene", &Entity::scene)
            .addFunction("sceneID", &Entity::sceneID)
            .addFunction("id", &Entity::id)
            .addFunction("null", &Entity::null)
            .addFunction("destroy", &Entity::destroy)
            .addFunction("hasParent", &Entity::hasParent)
            .addFunction("versionID", &Entity::versionID)
            .addFunction("addChild", &Entity::addChild)
            .addFunction("removeChild", &Entity::removeChild)
            .addFunction("removeComponent", static_cast<bool(Entity::*)(std::string_view)>(&Entity::removeComponent))
            .addFunction("getComponent", static_cast<luabridge::LuaRef(Entity::*)(std::string_view)>(&Entity::getComponent))
            .addFunction("addComponent", static_cast<bool(Entity::*)(std::string_view, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef, luabridge::LuaRef)>(&Entity::addComponent))
        .endClass()
        //component name
        .beginClass<ComponentName>("ComponentName")
            .addFunction("name", &ComponentName::name)
            .addFunction("size", &ComponentName::size)
            .addFunction("empty", &ComponentName::empty)
        .endClass()
        //component body
        //TODO: add component rigid body
        .beginClass<ComponentTransform>("ComponentTransform")
            .addFunction("addChild", static_cast<void(ComponentTransform::*)(Entity)const>(&ComponentTransform::addChild))
            .addFunction("removeChild", static_cast<void(ComponentTransform::*)(Entity)const>(&ComponentTransform::removeChild))
            .addFunction("hasParent", &ComponentTransform::hasParent)

            .addFunction("setPosition", static_cast<void(ComponentTransform::*)(decimal, decimal, decimal)>(&ComponentTransform::setPosition))
            .addFunction("setScale",    static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::setScale))
            .addFunction("setRotation", static_cast<void(ComponentTransform::*)(float, float, float, float)>(&ComponentTransform::setRotation))

            .addFunction("translate", static_cast<void(ComponentTransform::*)(decimal, decimal, decimal, bool)>(&ComponentTransform::translate))
            .addFunction("scale", static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::scale))
            .addFunction("rotate", static_cast<void(ComponentTransform::*)(float, float, float, bool)>(&ComponentTransform::rotate))
            .addFunction("getDistance", &ComponentTransform::getDistance)
            .addFunction("getScreenCoordinates", &ComponentTransform::getScreenCoordinates)
            .addFunction("alignTo", static_cast<void(ComponentTransform::*)(const glm::vec3&)>(&ComponentTransform::alignTo))
            .addFunction("alignTo", static_cast<void(ComponentTransform::*)(float, float, float)>(&ComponentTransform::alignTo))

            .addFunction("getRotation", &ComponentTransform::getRotation)
            .addFunction("getPosition", &ComponentTransform::getPosition)
            .addFunction("getScale", &ComponentTransform::getScale)
            .addFunction("getLocalPosition", &ComponentTransform::getLocalPosition)
            .addFunction("getForward", &ComponentTransform::getForward)
            .addFunction("getRight", &ComponentTransform::getRight)
            .addFunction("getUp", &ComponentTransform::getUp)
            //.addFunction("getLinearVelocity", &ComponentTransform::getLinearVelocity)
            .addFunction("getWorldMatrix", &ComponentTransform::getWorldMatrix)
            .addFunction("getWorldMatrixRendering", &ComponentTransform::getWorldMatrixRendering)
            .addFunction("getLocalMatrix", &ComponentTransform::getLocalMatrix)

            //.addFunction("setLinearVelocity", static_cast<void(ComponentTransform::*)(decimal, decimal, decimal, bool)>(&ComponentTransform::setLinearVelocity))

        .endClass()
        //component camera
        .beginClass<ComponentCamera>("ComponentCamera")
            .addFunction("getRight", &ComponentCamera::getRight)
            //.addFunction("getUp", &ComponentCamera::getUp)

            .addFunction("getAngle", &ComponentCamera::getAngle)
            .addFunction("getAspectRatio", &ComponentCamera::getAspectRatio)
            .addFunction("getNear", &ComponentCamera::getNear)
            .addFunction("getFar", &ComponentCamera::getFar)
/*
            .addFunction("getView", &ComponentCamera::getView)
            .addFunction("getProjection", &ComponentCamera::getProjection)
            .addFunction("getViewInverse", &ComponentCamera::getViewInverse)
            .addFunction("getProjectionInverse", &ComponentCamera::getProjectionInverse)

            .addFunction("getViewProjection", &ComponentCamera::getViewProjection)
            .addFunction("getViewProjectionInverse", &ComponentCamera::getViewProjectionInverse)
            .addFunction("getViewVector", &ComponentCamera::getViewVector)
*/


        .endClass()
        //component logics
        .beginClass<ComponentLogic>("ComponentLogic")
            .addFunction("call", &ComponentLogic::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic::*)(luabridge::LuaRef)>(&ComponentLogic::setFunctor))
        .endClass()
        .beginClass<ComponentLogic1>("ComponentLogic1")
            .addFunction("call", &ComponentLogic1::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic1::*)(luabridge::LuaRef)>(&ComponentLogic1::setFunctor))
        .endClass()
        .beginClass<ComponentLogic2>("ComponentLogic2")
            .addFunction("call", &ComponentLogic2::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic2::*)(luabridge::LuaRef)>(&ComponentLogic2::setFunctor))
        .endClass()
        .beginClass<ComponentLogic3>("ComponentLogic3")
            .addFunction("call", &ComponentLogic3::call)
            .addFunction("setFunctor", static_cast<void(ComponentLogic3::*)(luabridge::LuaRef)>(&ComponentLogic3::setFunctor))
        .endClass()
        //component model TODO add more
        .beginClass<ComponentModel>("ComponentModel")
            .addFunction("getBoundingBox", &ComponentModel::getBoundingBox)
            .addFunction("getRadius", &ComponentModel::getRadius)
            .addFunction("getNumModels", &ComponentModel::getNumModels)
            .addFunction("setStage", &ComponentModel::setStage)
            .addFunction("getOwner", &ComponentModel::getOwner)
            .addFunction("removeModel", &ComponentModel::removeModel)
            .addFunction("hide", &ComponentModel::hide)
            .addFunction("show", &ComponentModel::show)
            .addFunction("getModel", &ComponentModel::getModel)
        .endClass()
        //model instance
        .beginClass<ModelInstance>("ModelInstance")
            .addFunction("getColor", &ModelInstance::getColor)
            .addFunction("setColor", static_cast<void(ModelInstance::*)(const float, const float, const float, const float)>(&ModelInstance::setColor))
            .addFunction("forceRender", &ModelInstance::forceRender)
            .addFunction("getScale", &ModelInstance::getScale)
            .addFunction("setScale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setScale))
            .addFunction("rotate", static_cast<void(ModelInstance::*)(const float, const float, const float, bool)>(&ModelInstance::rotate))
            .addFunction("translate", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::translate))
            .addFunction("scale", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::scale))
            .addFunction("getPosition", &ModelInstance::getPosition)
            .addFunction("getRotation", &ModelInstance::getRotation)
            .addFunction("getGodRaysColor", &ModelInstance::getGodRaysColor)
            .addFunction("setGodRaysColor", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setGodRaysColor))
            .addFunction("getParent", &ModelInstance::getParent)
            .addFunction("hasPassedRenderCheck", &ModelInstance::hasPassedRenderCheck)
            .addFunction("hide", &ModelInstance::hide)
            .addFunction("show", &ModelInstance::show)
            .addFunction("isVisible", &ModelInstance::isVisible)
            .addFunction("getIndex", &ModelInstance::getIndex)
            .addFunction("isForceRendered", &ModelInstance::isForceRendered)
            .addFunction("playAnimation", static_cast<void(ModelInstance::*)(std::string_view, float, float, uint32_t)>(&ModelInstance::playAnimation))
            .addFunction("setOrientation", static_cast<void(ModelInstance::*)(const float, const float, const float)>(&ModelInstance::setOrientation))
        .endClass()
    ;
}

#pragma once
#ifndef GAME_STATE_H
#define GAME_STATE_H

struct GameState {enum State {
    Main_Menu,

    Host_Screen_Setup_0,

    Host_Screen_Setup_1,
    Host_Screen_Setup_1_Persistent,

    Host_Screen_Setup_FFA_2,
    Host_Screen_Setup_TeamDeathMatch_2,
    Host_Screen_Setup_HomelandSecurity_2,

    Host_Screen_Lobby_FFA_3,
    Host_Screen_Lobby_TeamDeathMatch_3,
    Host_Screen_Lobby_HomelandSecurity_3,

    Join_Screen_Setup_1,
    Join_Screen_Lobby_2,

    Encyclopedia_Main,
    Encyclopedia_Ships,
    Encyclopedia_Factions,
    Encyclopedia_Technology,

    Options_Main,
    Options_Sounds,
    Options_Keybinds,
    Options_Graphics,

    LoadingScreen,

    Game,
};};

#endif
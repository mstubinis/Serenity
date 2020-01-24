#pragma once
#ifndef GAME_CONFIG_DATA_STRUCT_H
#define GAME_CONFIG_DATA_STRUCT_H

#include <string>
#include <SFML/Window.hpp>

struct ConfigDataStruct final {

    //basic info
    unsigned int    window_width;
    unsigned int    window_height;
    std::string     window_mode; //"fullscreen" or "windowed" or "windowed_fullscreen"
    bool            window_maximized;

    //sound options
    unsigned int    volume_music;
    unsigned int    volume_effects;
    unsigned int    volume_speech;

    //host / join info
    unsigned short  join_server_port;
    std::string     join_server_ip;
    std::string     join_server_player_name;

    unsigned short  host_server_port;
    std::string     host_server_player_name;

    ConfigDataStruct() {
        volume_music            = 45;
        volume_effects          = 100;
        volume_speech           = 100;

        join_server_port        = 55000;
        join_server_ip          = "127.0.0.1";
        join_server_player_name = "Redshirt";

        host_server_port        = 55000;
        host_server_player_name = "Goldshirt";

        window_width            = sf::VideoMode::getDesktopMode().width - 30;
        window_height           = sf::VideoMode::getDesktopMode().height - 120;
        window_mode             = "fullscreen";
        window_maximized        = false;
    }
    ~ConfigDataStruct() = default;

    //0 = normal, 1 = fullscreen, 2 = windowed_fullscreen
    const int getWindowModeInt() const{
        if (window_mode == "fullscreen")
            return 1;
        else if (window_mode == "windowed")
            return 0;
        else if (window_mode == "windowed_fullscreen")
            return 2;
        else
            return 0;
        return 0;
    }
};

#endif
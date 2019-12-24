#pragma once
#ifndef GAME_MAP_ENTRY_DATA_H
#define GAME_MAP_ENTRY_DATA_H

#include <vector>
#include <string>

#include <core/engine/resources/Handle.h>

struct MapEntryData final {
    std::string               map_file_path;
    std::string               map_folder_path;
    std::string               map_desc;
    std::string               map_skybox;
    std::string               map_name;
    std::vector<unsigned int> map_valid_game_modes;
    Handle                    map_screenshot_handle;
    MapEntryData() {
        map_file_path = map_folder_path = map_desc = map_skybox = map_name = "";
    }
};

#endif
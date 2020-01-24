#pragma once
#ifndef GAME_MAP_H
#define GAME_MAP_H

#include <unordered_map>
#include <string>

#include "MapEntry.h"

class MapDatabase final {
    public:
        static std::unordered_map<std::string, MapEntryData> DATABASE;
    private:

    public:
        static void init();
};

#endif
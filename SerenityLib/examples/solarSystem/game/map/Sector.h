#pragma once
#ifndef GAME_MAP_SECTOR_H
#define GAME_MAP_SECTOR_H

#include <unordered_map>

//a sector should be a collection of celestial object anchor points, and their midpoint

class Sector final {
    private:

    public:
        Sector();
        ~Sector();

        void update(const float& dt);
};


#endif

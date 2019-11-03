#pragma once
#ifndef GAME_MODE_FFA_H
#define GAME_MODE_FFA_H

#include "GameplayMode.h"

class FFA final: public GameplayMode {
    private:
    public:
        FFA(const unsigned int maxPlayers);
        ~FFA();
};

#endif
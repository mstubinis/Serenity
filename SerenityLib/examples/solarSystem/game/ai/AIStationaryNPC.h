#pragma once
#ifndef GAME_AI_STATIONARY_NPC_H
#define GAME_AI_STATIONARY_NPC_H

#include "AINPC.h"

class AIStationaryNPC: public AINPC {
    public:
        AIStationaryNPC(Ship& ship);
        ~AIStationaryNPC();

        void update(const double& dt);
};

#endif
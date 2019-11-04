#pragma once
#ifndef GAME_AI_INCLUDES_H
#define GAME_AI_INCLUDES_H

struct AIType final { enum Type {
    Player_You,
    Player_Other,
    AI_Easy,
    AI_Medium,
    AI_Hard,
    AI_None,
};};


#endif
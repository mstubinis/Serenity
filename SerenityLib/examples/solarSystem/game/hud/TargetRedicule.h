#pragma once
#ifndef GAME_HUD_TARGET_REDICULE_H
#define GAME_HUD_TARGET_REDICULE_H

class Map;
class Font;
class TargetRedicule {
    private:
        Map& m_Map;
        Font& m_Font;
    public:
        TargetRedicule(Map&, Font&);
        ~TargetRedicule();

        void update(const double& dt);
        void render();
};

#endif
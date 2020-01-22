#pragma once
#ifndef GAME_GUI_LOADING_SCREEN_H
#define GAME_GUI_LOADING_SCREEN_H

#include <string>

class Menu;
class Font;
class Map;
class LoadingScreen final {
    friend class Map;
    private:
        Menu&         m_Menu;
        Font&         m_Font;
        std::string   m_ShipClass;
        bool          m_SentPacket;

        float         m_Progress;
    public:
        LoadingScreen(Menu& menu, Font& font);
        ~LoadingScreen();

        void setProgress(const float& progress);

        const bool setShipClass(const std::string& ship_class);
        const bool startLoadingProcess();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};

#endif
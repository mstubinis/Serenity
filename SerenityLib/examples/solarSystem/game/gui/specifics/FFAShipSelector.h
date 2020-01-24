#pragma once
#ifndef GAME_GUI_FFA_SHIP_SELECTOR_H
#define GAME_GUI_FFA_SHIP_SELECTOR_H

class  Font;
class  Client;
class  HostScreenFFA2;
class  ScrollFrame;
class  Text;
class  Button;

struct ShipTokenOnClickTotal;
struct ShipTokenOnClickAllowed;

#include "../RoundedWindow.h"
#include <string>

#include "../../map/MapEntry.h"
#include "../../modes/GameplayMode.h"

class FFAShipSelector final : public RoundedWindow {
    friend struct ShipTokenOnClickTotal;
    friend struct ShipTokenOnClickAllowed;
    friend class  Client;
    friend class  HostScreenFFA2;
    private:
        Button*           m_MiddleDivide;
        HostScreenFFA2&   m_HostScreen;
        ScrollFrame*      m_ShipsTotalFrame;
        ScrollFrame*      m_ShipsAllowedFrame;
        Font&             m_Font;

        Text*             m_ShipsTotalLabel;
        Text*             m_ShipsAllowedLabel;

        void init_window_contents();
    public:
        FFAShipSelector(HostScreenFFA2&, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const std::string& labelText);
        ~FFAShipSelector();

        void resetWindow();

        std::vector<std::string> getAllowedShips();

        void onResize(const unsigned int& newWidth, const unsigned int& newHeight);

        void update(const double& dt);
        void render();
};

#endif
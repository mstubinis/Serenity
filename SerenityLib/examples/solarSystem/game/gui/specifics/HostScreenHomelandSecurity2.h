#pragma once
#ifndef GAME_GUI_HOST_SCREEN_2_HOMELAND_SECURITY_H
#define GAME_GUI_HOST_SCREEN_2_HOMELAND_SECURITY_H

class  Button;
class  Font;
class  Menu;
class  TextBox;
class  Text;
class  ServerHostingMapSelectorWindow;
class  MapDescriptionWindow;
struct Host2HS_ButtonBack_OnClick;
struct Host2HS_ButtonNext_OnClick;
class  HostScreen1;

class HostScreenHomelandSecurity2 final {
    friend class  HostScreen1;
    friend struct Host2HS_ButtonBack_OnClick;
    friend struct Host2HS_ButtonNext_OnClick;
    private:
        HostScreen1& m_HostScreen1;
        Menu& m_Menu;

        Button* m_BackgroundEdgeGraphicTop;
        Text* m_TopLabel;

        Button* m_BackgroundEdgeGraphicBottom;
        Button* m_BackButton;
        Button* m_ForwardButton;

        bool m_IsPersistent;
    public:
        HostScreenHomelandSecurity2(HostScreen1& hostScreen1, Menu&, Font& font);
        ~HostScreenHomelandSecurity2();

        Menu& getMenu();

        void setPersistent(const bool& persistent);
        const bool isPersistent() const;

        void setTopText(const std::string& text);

        void onResize(const unsigned int newWidth, const unsigned int newHeight);

        void update(const double& dt);
        void render();
};


#endif
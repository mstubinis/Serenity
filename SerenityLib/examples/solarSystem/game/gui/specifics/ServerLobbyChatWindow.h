#pragma once
#ifndef GAME_SERVER_LOBBY_CHAT_WINDOW_H
#define GAME_SERVER_LOBBY_CHAT_WINDOW_H

#include "../ScrollWindow.h"
#include "../TextBox.h"


class ServerLobbyChatWindow final {
    private:
        ScrollFrame* m_ChatWindow;
        TextBox*     m_ChatInput;
        Font&        m_Font;
        float        m_Width;
        float        m_Height;
        void*        m_UserPointer;
    public:
        ServerLobbyChatWindow(const Font& font, const float& x, const float& y);
        ~ServerLobbyChatWindow();

        void setColor(const float& r, const float& g, const float& b, const float& a);
        void setPosition(const float& x, const float& y);

        void setUserPointer(void*);
        void* getUserPointer();

        void addContent(const std::string& key, Widget* widget);

        Font& getFont();
        ScrollFrame& getWindowFrame();
        TextBox& getInputFrame();

        void update(const double& dt);
        void render();

};


#endif
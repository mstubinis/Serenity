#pragma once
#ifndef GAME_GUI_SHIP_TOKEN_H
#define GAME_GUI_SHIP_TOKEN_H

class  Font;

#include "../Button.h"
#include "../../ships/Ships.h"
#include <string>

//inherited button is the "border"
class ShipToken final : public Button {
    private:
        std::string     m_ShipClass;
        Button*         m_TokenCenter; //this button is the "center"
        bool            m_LightedUp;
    public:
        ShipToken(const std::string& shipClass, Font& font, const float& x, const float& y, const float& depth, const bool& lit);
        ShipToken(const ShipInformation& shipInfo, Font& font, const float& x, const float& y, const float& depth, const bool& lit);
        ~ShipToken();

        template<class T> void setOnClickFunctor(const T& functor) {
            m_FunctorOnClick = std::bind<void>(functor, this);
            m_TokenCenter->setOnClickFunctor(functor);
        }

        void lightUp();
        void lightOff();
        const std::string& getShipClass() const;
        const bool& isLit() const;

        void setColor(const float& r, const float& g, const float& b, const float& a) override;
        void setColor(const glm::vec4& color) override;

        void update(const double& dt) override;
        void render(const glm::vec4& scissor) override;
        void render() override;
};


#endif
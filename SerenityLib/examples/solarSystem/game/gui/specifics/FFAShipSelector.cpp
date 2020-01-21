#include "FFAShipSelector.h"
#include "HostScreenFFA2.h"
#include "ShipToken.h"
#include "../../factions/Faction.h"
#include "../../map/Map.h"
#include "../Text.h"
#include "../ScrollFrame.h"
#include "../../ships/Ships.h"
#include "../../networking/server/Server.h"

#include <core/engine/fonts/Font.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>

using namespace std;
using namespace Engine;

constexpr auto dividor_width          = 25.0f;
constexpr auto scroll_frame_padding_2 = 20.0f;
constexpr auto scroll_frame_padding   = 30.0f;
constexpr auto padding_y              = 110.0f;

struct ShipTokenOnClickTotal final { void operator()(Button* button) const {
    ShipToken* token_ptr = dynamic_cast<ShipToken*>(button);
    if (token_ptr) {
        ShipToken& token             = *token_ptr;
        FFAShipSelector& ffaSelector = *static_cast<FFAShipSelector*>(token.getUserPointer());

        //ship pool
        if (token.isLit()) {
            for (auto& widget_row : ffaSelector.m_ShipsAllowedFrame->content()) {
                bool done = false;
                for (auto& widget : widget_row.widgets) {
                    ShipToken& token_1 = *static_cast<ShipToken*>(widget.widget);
                    if (token_1.getShipClass() == token.getShipClass()) {
                        token_1.lightUp();
                        done = true;
                        break;
                    }
                }
                if (done)
                    break;
            }
            token.lightOff();
        }
        Server::SERVER_HOST_DATA.setAllowedShips(ffaSelector.getAllowedShips());
    }
}};
struct ShipTokenOnClickAllowed final { void operator()(Button* button) const {
    ShipToken* token_ptr = dynamic_cast<ShipToken*>(button);
    if (token_ptr) {
        ShipToken& token             = *token_ptr;
        FFAShipSelector& ffaSelector = *static_cast<FFAShipSelector*>(token.getUserPointer());

        //ships allowed
        if (token.isLit()) {
            for (auto& widget_row : ffaSelector.m_ShipsTotalFrame->content()) {
                bool done = false;
                for (auto& widget : widget_row.widgets) {
                    ShipToken& token_1 = *static_cast<ShipToken*>(widget.widget);
                    if (token_1.getShipClass() == token.getShipClass()) {
                        token_1.lightUp();
                        done = true;
                        break;
                    }
                }
                if (done)
                    break;
            }
            token.lightOff();
        }
        Server::SERVER_HOST_DATA.setAllowedShips(ffaSelector.getAllowedShips());
    }
}};

FFAShipSelector::FFAShipSelector(HostScreenFFA2& hostScreen, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font,x,y,width,height,depth,borderSize,labelText), m_HostScreen(hostScreen), m_Font(font){

    m_ShipsTotalFrame = NEW ScrollFrame(font,
        x - (dividor_width / 2.0f) - scroll_frame_padding_2, 
        y - (height / 2.0f) + scroll_frame_padding,
        ((width - (scroll_frame_padding * 2.0f)) / 2.0f) - (scroll_frame_padding_2 * 2.0f),
        (height - padding_y),
    depth - 0.001f);
    m_ShipsTotalFrame->setAlignment(Alignment::BottomRight);

    m_ShipsAllowedFrame = NEW ScrollFrame(font,
        x + (dividor_width / 2.0f) + scroll_frame_padding_2,
        y - (height / 2.0f) + scroll_frame_padding,
        ((width - (scroll_frame_padding * 2.0f)) / 2.0f) - (scroll_frame_padding_2 * 2.0f),
        (height - padding_y),
    depth - 0.001f);
    m_ShipsAllowedFrame->setAlignment(Alignment::BottomLeft);

    m_ShipsTotalLabel = NEW Text(x - (width / 4.0f), y + (height / 2.0f) - 9.0f, font, "Ship Database");
    m_ShipsTotalLabel->setAlignment(Alignment::Center);
    m_ShipsTotalLabel->setTextAlignment(TextAlignment::Center);
    m_ShipsTotalLabel->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);

    m_ShipsAllowedLabel = NEW Text(x + (width / 4.0f), y + (height / 2.0f) - 9.0f, font, "Ships Allowed");
    m_ShipsAllowedLabel->setAlignment(Alignment::Center);
    m_ShipsAllowedLabel->setTextAlignment(TextAlignment::Center);
    m_ShipsAllowedLabel->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);

    m_MiddleDivide = NEW Button(font, x, y - (height / 2.0f), dividor_width, height - 51.0f);
    m_MiddleDivide->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    m_MiddleDivide->setText("");
    m_MiddleDivide->disableMouseover();
    m_MiddleDivide->disable();
    m_MiddleDivide->setDepth(depth - 0.003f);
    m_MiddleDivide->setAlignment(Alignment::BottomCenter);
    m_MiddleDivide->setTextureCorner(nullptr);
    m_MiddleDivide->setTextureCornerHighlight(nullptr);
    m_MiddleDivide->setTextureEdge(nullptr);
    m_MiddleDivide->setTextureEdgeHighlight(nullptr);
    m_MiddleDivide->enableTextureCorner(false);
    m_MiddleDivide->enableTextureEdge(false);

    init_window_contents();
}
FFAShipSelector::~FFAShipSelector() {
    SAFE_DELETE(m_ShipsTotalFrame);
    SAFE_DELETE(m_ShipsAllowedFrame);
    SAFE_DELETE(m_ShipsTotalLabel);
    SAFE_DELETE(m_ShipsAllowedLabel);
    SAFE_DELETE(m_MiddleDivide);
}
vector<string> FFAShipSelector::getAllowedShips() {
    vector<string> ret;
    for (auto& itr : m_ShipsAllowedFrame->content()) {
        for (auto& widget_entry : itr.widgets) {
            auto& token = *static_cast<ShipToken*>(widget_entry.widget);

            if (token.isLit()) {
                ret.push_back(token.getShipClass());
            }
        }
    }
    return ret;
}
void FFAShipSelector::init_window_contents() {
    for (size_t i = 0; i < ShipTier::_TOTAL; ++i) {
        ShipTier::Tier tier = static_cast<ShipTier::Tier>(i);
        auto classes = Ships::getShipClassesSortedByFaction(tier);
        for (auto& ship_class : classes) {
            auto& ship_info = Ships::Database.at(ship_class);
            if (ship_info.Faction == FactionEnum::Borg || ship_info.Tier == ShipTier::Station) { //no borg or stations
                continue;
            }

            ShipToken* ship_pool_button = NEW ShipToken(ship_info, m_Font, 0, 0, getDepth() - 0.004f, true, this);
            ship_pool_button->setOnClickFunctor(ShipTokenOnClickTotal());
            

            ShipToken* ship_allowed_button = NEW ShipToken(ship_info, m_Font, 0, 0, getDepth() - 0.004f, false, this);
            ship_allowed_button->setOnClickFunctor(ShipTokenOnClickAllowed());

            m_ShipsTotalFrame->addContent(ship_pool_button, i);
            m_ShipsAllowedFrame->addContent(ship_allowed_button, i);
        }
    }
}
void FFAShipSelector::resetWindow() {
    m_ShipsTotalFrame->clear();
    m_ShipsAllowedFrame->clear();

    init_window_contents();
}

void FFAShipSelector::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);

    const auto pos = positionWorld();
    const auto frame_size = glm::vec2(width(), height());

    m_ShipsTotalFrame->setPosition(pos.x - (dividor_width / 2.0f) - scroll_frame_padding_2, pos.y - (frame_size.y / 2.0f) + scroll_frame_padding);
    m_ShipsTotalFrame->setSize(((frame_size.x - (scroll_frame_padding * 2.0f)) / 2.0f) - (scroll_frame_padding_2 * 2.0f), frame_size.y - padding_y);

    m_ShipsAllowedFrame->setPosition(pos.x + (dividor_width / 2.0f) + scroll_frame_padding_2, pos.y - (frame_size.y / 2.0f) + scroll_frame_padding);
    m_ShipsAllowedFrame->setSize(((frame_size.x - (scroll_frame_padding * 2.0f)) / 2.0f) - (scroll_frame_padding_2 * 2.0f), frame_size.y - padding_y);

    m_ShipsTotalLabel->setPosition(pos.x - (frame_size.x / 4.0f), pos.y + (frame_size.y / 2.0f) - 9.0f);
    m_ShipsAllowedLabel->setPosition(pos.x + (frame_size.x / 4.0f), pos.y + (frame_size.y / 2.0f) - 9.0f);

    m_MiddleDivide->setPosition(pos.x, pos.y - (frame_size.y / 2.0f));
    m_MiddleDivide->setSize(dividor_width, frame_size.y - 51.0f);
}

void FFAShipSelector::update(const double& dt) {
    RoundedWindow::update(dt);

    m_ShipsTotalFrame->update(dt);
    m_ShipsAllowedFrame->update(dt);

    m_ShipsTotalLabel->update(dt);
    m_ShipsAllowedLabel->update(dt);

    m_MiddleDivide->update(dt);
}
void FFAShipSelector::render() {
    RoundedWindow::render();

    m_ShipsTotalFrame->render();
    m_ShipsAllowedFrame->render();

    m_ShipsTotalLabel->render();
    m_ShipsAllowedLabel->render();

    m_MiddleDivide->render();
}
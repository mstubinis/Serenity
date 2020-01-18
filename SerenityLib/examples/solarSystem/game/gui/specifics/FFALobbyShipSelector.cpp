#include "FFALobbyShipSelector.h"
#include "LobbyFFA.h"
#include "ShipToken.h"
#include "Ship3DViewer.h"
#include "ShipDescription.h"
#include "MessageWithArrow.h"

#include "../ScrollFrame.h"
#include "../Text.h"

#include "../../Helper.h"
#include "../../networking/packets/PacketConnectionAccepted.h"
#include "../../factions/Faction.h"

using namespace std;

struct ShipTokenOnClick final { void operator()(Button* button) const {
    ShipToken* token_ptr = dynamic_cast<ShipToken*>(button);
    if (token_ptr) {
        
        ShipToken& token = *token_ptr;
        auto& ffaShipSelector = *static_cast<FFALobbyShipSelector*>(token.getUserPointer());

        auto& ship_class = token.getShipClass();
        ffaShipSelector.clearChosenShipClass();
        ffaShipSelector.m_FFALobby.setChosenShipClass(ship_class);
        ffaShipSelector.m_ChosenShipClass = ship_class;
        token.lightUpFully();
    }
}};

FFALobbyShipSelector::FFALobbyShipSelector(LobbyScreenFFA& lobbyFFA, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
: m_Font(font), m_FFALobby(lobbyFFA){
    m_ChosenShipClass = "";
    m_ShipsFrame = new ScrollFrame(font, x, y, width, height, depth - 0.001f);

    //setLabelText("Ships");
    //m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
}

FFALobbyShipSelector::~FFALobbyShipSelector() {
    SAFE_DELETE(m_ShipsFrame);
}
void FFALobbyShipSelector::clearChosenShipClass() {
    for (auto& widget_row : m_ShipsFrame->content()) {
        for (auto& widget : widget_row.widgets) {
            ShipToken& token_1 = *static_cast<ShipToken*>(widget.widget);
            token_1.lightOff();
            token_1.lightUp();
        }
    }
    m_ChosenShipClass = "";
}
void FFALobbyShipSelector::clear() {
    m_ChosenShipClass = "";
    m_ShipsFrame->clear();
}
void FFALobbyShipSelector::setPosition(const float x, const float y) {
    m_ShipsFrame->setPosition(x, y);
}
void FFALobbyShipSelector::setSize(const float width, const float height) {
    m_ShipsFrame->setSize(width, height);
}
const string& FFALobbyShipSelector::getChosenShipClass() const {
    return m_ChosenShipClass;
}
void FFALobbyShipSelector::init_window_contents(const PacketConnectionAccepted& packet) {
    clear();

    const auto list = Helper::SeparateStringByCharacterIntoSet(packet.allowed_ships, ',');
    for (size_t i = 0; i < ShipTier::_TOTAL; ++i) {
        ShipTier::Tier tier = static_cast<ShipTier::Tier>(i);
        auto classes = Ships::getShipClassesSortedByFaction(tier);
        for (auto& ship_class : classes) {
            if (list.count(ship_class)) {
                auto& ship_info = Ships::Database.at(ship_class);
                if (ship_info.Faction == FactionEnum::Borg || ship_info.Tier == ShipTier::Station) { //no borg or stations
                    continue;
                }

                ShipToken* ship_button = new ShipToken(ship_info, m_Font, 0, 0,  0.004f, true, this);
                ship_button->setOnClickFunctor(ShipTokenOnClick());

                m_ShipsFrame->addContent(ship_button, i);
            }
        }
    }
}
void FFALobbyShipSelector::update(const double& dt) {
    m_ShipsFrame->update(dt);
}
void FFALobbyShipSelector::render() {
    m_ShipsFrame->render();
}
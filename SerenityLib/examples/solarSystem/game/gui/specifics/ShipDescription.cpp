#include "ShipDescription.h"
#include "../Text.h"
#include "../ScrollFrame.h"
#include "../../factions/Faction.h"

#include "../../ships/Ships.h"

constexpr auto text_to_scroll_frame_break_height = 20.0f;

using namespace std;

ShipDescription::ShipDescription(Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth) : m_Font(font){
    m_Text = NEW Text(x, y - (text_to_scroll_frame_break_height / 2.0f), font, " ");
    m_Text->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
    m_ScrollFrame = NEW ScrollFrame(font, x, y - m_Text->singleLineHeight() - text_to_scroll_frame_break_height, width, height - m_Text->singleLineHeight() - text_to_scroll_frame_break_height, depth);

    setShipClass("");
}
ShipDescription::~ShipDescription() {
    SAFE_DELETE(m_Text);
    SAFE_DELETE(m_ScrollFrame);
}

const bool ShipDescription::setShipClass(const string& shipClass) {
    if (m_ShipClass == shipClass) {
        return false;
    }
    if (shipClass.empty()){
        m_ShipClass = "";
        m_Text->setText(" ");
        m_ScrollFrame->clear();
        return true;
    }
    assert(Ships::Database.count(shipClass));

    auto& data = Ships::Database[shipClass];

    m_ShipClass = shipClass;

    m_Text->setText(data.ClassVerbose);

    m_ScrollFrame->clear();
    Text* text = NEW Text(0, 0, m_Font, data.Description);
    text->setColor(Factions::Database[FactionEnum::Federation].GUIColor);
    text->setTextScale(0.61f, 0.61f);
    m_ScrollFrame->addContent(text);

    return true;
}
void ShipDescription::setPosition(const float& x, const float& y) {
    m_ScrollFrame->setPosition(x, y - m_Text->height() - text_to_scroll_frame_break_height);
    m_Text->setPosition(x, y - (text_to_scroll_frame_break_height / 2.0f));
}
void ShipDescription::setPosition(const glm::vec2& position) {
    ShipDescription::setPosition(position.x, position.y);
}

void ShipDescription::setSize(const float& width, const float& height) {
    m_ScrollFrame->setSize(width, height - m_Text->height() - text_to_scroll_frame_break_height);
}
void ShipDescription::setSize(const glm::vec2& size) {
    ShipDescription::setSize(size.x, size.y);
}

void ShipDescription::update(const double& dt) {
    m_Text->update(dt);
    m_ScrollFrame->update(dt);
}
void ShipDescription::render() {
    m_Text->render();
    m_ScrollFrame->render();
}
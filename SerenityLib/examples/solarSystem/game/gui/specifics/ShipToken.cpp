#include "ShipToken.h"
#include "../../ships/Ships.h"
#include "../../Menu.h"
#include <core/engine/fonts/Font.h>
#include <core/engine/textures/Texture.h>

using namespace std;

const auto token_padding = 12.0f;

ShipToken::ShipToken(const string& shipClass, Font& font, const float& x, const float& y, const float& depth, const bool& lit) : Button(font, x, y, 1, 1){
    m_ShipClass = shipClass;
    m_LightedUp = lit;

    auto& info = Ships::Database[shipClass];

    Texture* texture_border = (Texture*)(info.IconBorderTextureHandle.get());
    Texture* texture = (Texture*)(info.IconTextureHandle.get());

    setSize(texture_border->width(), texture_border->height());
    setAlignment(Alignment::TopLeft);
    setText("");
    setTexture(texture_border);
    setTextureHighlight(texture_border);

    setTextureCorner(nullptr);
    setTextureEdge(nullptr);
    setTextureCornerHighlight(nullptr);
    setTextureEdgeHighlight(nullptr);
    enableTextureCorner(false);
    enableTextureEdge(false);
    setPaddingSize(token_padding);
    setDepth(depth);

    m_TokenCenter = new Button(font, 0, 0, 1, 1);
    m_TokenCenter->setAlignment(Alignment::TopLeft);
    m_TokenCenter->setSize(texture_border->width(), texture_border->height());
    m_TokenCenter->setText("");
    m_TokenCenter->setTexture(nullptr);
    m_TokenCenter->setTextureHighlight(texture);
    m_TokenCenter->enableTexture(false);

    m_TokenCenter->setTextureCorner(nullptr);
    m_TokenCenter->setTextureEdge(nullptr);
    m_TokenCenter->setTextureCornerHighlight(nullptr);
    m_TokenCenter->setTextureEdgeHighlight(nullptr);
    m_TokenCenter->enableTextureCorner(false);
    m_TokenCenter->enableTextureEdge(false);
    m_TokenCenter->setPaddingSize(token_padding);
    m_TokenCenter->setDepth(depth - 0.001f);
    if (!lit) {
        lightOff();
    }else{
        lightUp();
    }
    addChild(m_TokenCenter);
}
ShipToken::ShipToken(const ShipInformation& shipInfo, Font& font, const float& x, const float& y, const float& depth, const bool& lit) : ShipToken(shipInfo.Class,font, x, y, depth, lit) {
}
ShipToken::~ShipToken() {
    
}
const string& ShipToken::getShipClass() const {
    return m_ShipClass;
}
const bool& ShipToken::isLit() const {
    return m_LightedUp;
}
void ShipToken::lightUp() {
    m_LightedUp = true;
    m_TokenCenter->show();
    m_TokenCenter->enableMouseover();
    m_TokenCenter->enable();

    enableMouseover();
    enable();
    auto& info = Ships::Database[m_ShipClass];
    setColor(info.FactionInformation.ColorText);
}
void ShipToken::lightOff() {
    m_LightedUp = false;
    m_TokenCenter->hide();
    m_TokenCenter->disable();
    m_TokenCenter->disableMouseover();

    disable();
    disableMouseover();
    setColor(glm::vec4(0.25f, 0.25f, 0.25f, 1.0f));
}

void ShipToken::setColor(const float& r, const float& g, const float& b, const float& a) {
    Button::setColor(r, g, b, a);
    m_TokenCenter->setColor(r, g, b, a);
}
void ShipToken::setColor(const glm::vec4& color) {
    ShipToken::setColor(color.r, color.g, color.b, color.a);
}
void ShipToken::update(const double& dt) {
    Button::update(dt);
}
void ShipToken::render(const glm::vec4& scissor) {
    Button::render(scissor);
}
void ShipToken::render() {
    Button::render();
}
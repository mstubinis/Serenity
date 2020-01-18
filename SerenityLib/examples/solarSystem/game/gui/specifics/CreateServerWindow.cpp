#include "CreateServerWindow.h"
#include "../../factions/Faction.h"
#include "../Text.h"

using namespace std;



CreateServerWindow::CreateServerWindow(HostScreen1Persistent& hostScreen1Persistent, Font& font, const float& x, const float& y, const float& width, const float& height, const float& depth, const unsigned int& borderSize, const string& labelText)
:RoundedWindow(font, x, y, width, height, depth, borderSize, labelText), m_Font(font), m_HostScreen1Persistent(hostScreen1Persistent) {

    m_Label->setColor(Factions::Database[FactionEnum::Federation].GUIColorText1);
}
CreateServerWindow::~CreateServerWindow() {

}

void CreateServerWindow::resetWindow() {

}

void CreateServerWindow::onResize(const unsigned int& newWidth, const unsigned int& newHeight) {
    RoundedWindow::onResize(newWidth, newHeight);
}

void CreateServerWindow::update(const double& dt) {
    RoundedWindow::update(dt);
}
void CreateServerWindow::render() {
    RoundedWindow::render();
}
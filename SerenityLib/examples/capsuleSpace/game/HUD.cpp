#include "HUD.h"
#include "core/engine/resources/Engine_Resources.h"
#include "core/engine/events/Engine_Events.h"
#include "core/engine/renderer/Engine_Renderer.h"
#include "core/engine/Engine_Window.h"
#include "core/engine/Engine_Math.h"
#include "core/Font.h"
#include "core/engine/textures/Texture.h"
#include "core/Scene.h"
#include "core/Camera.h"
#include "core/Material.h"
#include "Ship.h"

#include "ResourceManifest.h"

#include <glm/vec4.hpp>

using namespace Engine;
using namespace std;

HUD::HUD(){
    m_Font = Resources::addFont(ResourceManifest::BasePath + "data/Fonts/consolas.fnt");
    Engine::Math::setColor(m_Color,255,255,0);
    m_Active = true;
}
HUD::~HUD(){
}

uint _count = 0;
void HUD::update(const float& dt){
    if (Engine::isKeyDownOnce(KeyboardKey::LeftAlt, KeyboardKey::Z)) {
        m_Active = !m_Active;
    }
}
void HUD::render(){
    if (!m_Active) return;
    glm::vec2 winSize = glm::vec2(Resources::getWindowSize().x, Resources::getWindowSize().y);

    Font& font = *Resources::getFont(m_Font);

    #pragma region DrawDebugStuff

    font.renderText(Engine::Data::reportTime() + 
        epriv::Core::m_Engine->m_DebugManager.reportDebug(),
        glm::vec2(10, Resources::getWindowSize().y - 10), glm::vec4(m_Color.x, m_Color.y, m_Color.z, 1), 0, glm::vec2(0.8f, 0.8f), 0.1f);

    #pragma endregion
}

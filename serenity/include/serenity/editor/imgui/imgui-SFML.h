#ifndef IMGUI_SFML_H
#define IMGUI_SFML_H

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Joystick.hpp>

#include <serenity/editor/imgui/imgui-SFML_export.h>

namespace sf {
    class Event;
    class RenderTarget;
    class RenderTexture;
    class RenderWindow;
    class Sprite;
    class Texture;
    class Window;
    class WindowBase;
}

namespace ImGui {
    namespace SFML {
        IMGUI_SFML_API void Init(sf::WindowBase& window, const sf::Vector2f& displaySize,
            bool loadDefaultFont = true);

        IMGUI_SFML_API void ProcessEvent(const sf::Event& event);
        IMGUI_SFML_API void Update(sf::WindowBase& window, sf::Time dt);
        IMGUI_SFML_API void Update(const sf::Vector2i& mousePos, const sf::Vector2f& displaySize,
            sf::Time dt);

        IMGUI_SFML_API void Shutdown();

        // joystick functions
        IMGUI_SFML_API void SetActiveJoystickId(unsigned int joystickId);
        IMGUI_SFML_API void SetJoytickDPadThreshold(float threshold);
        IMGUI_SFML_API void SetJoytickLStickThreshold(float threshold);

        IMGUI_SFML_API void SetJoystickMapping(int action, unsigned int joystickButton);
        IMGUI_SFML_API void SetDPadXAxis(sf::Joystick::Axis dPadXAxis, bool inverted = false);
        IMGUI_SFML_API void SetDPadYAxis(sf::Joystick::Axis dPadYAxis, bool inverted = false);
        IMGUI_SFML_API void SetLStickXAxis(sf::Joystick::Axis lStickXAxis, bool inverted = false);
        IMGUI_SFML_API void SetLStickYAxis(sf::Joystick::Axis lStickYAxis, bool inverted = false);
    } // end of namespace SFML

} // end of namespace ImGui

#endif //# IMGUI_SFML_H
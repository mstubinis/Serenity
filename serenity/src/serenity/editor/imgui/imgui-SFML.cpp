#include <serenity/editor/imgui/imgui-SFML.h>
#include <serenity/editor/imgui/imgui.h>

#include <SFML/Config.hpp>
#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Window/Clipboard.hpp>
#include <SFML/Window/Cursor.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Touch.hpp>
#include <SFML/Window/Window.hpp>

#include <cassert>
#include <cmath> // abs
#include <cstddef> // offsetof, NULL
#include <cstring> // memcpy

#if __cplusplus >= 201103L // C++11 and above
    static_assert(sizeof(GLuint) <= sizeof(ImTextureID), "ImTextureID is not large enough to fit GLuint.");
#endif

struct SFEditorFontImage {
    sf::Image m_FontTexture;
    GLuint m_GLTextureHandle = 0;

    ~SFEditorFontImage() {
        if (m_GLTextureHandle != 0) {
            glDeleteTextures(1, &m_GLTextureHandle);
        }
    }

    void createTexture() {
        if (m_GLTextureHandle != 0) {
            glDeleteTextures(1, &m_GLTextureHandle);
        }
        glGenTextures(1, &m_GLTextureHandle);
        glBindTexture(GL_TEXTURE_2D, m_GLTextureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_FontTexture.getSize().x, m_FontTexture.getSize().y, 0, GL_RGB, GL_UNSIGNED_BYTE, m_FontTexture.getPixelsPtr());
        //glGenerateMipmap(GL_TEXTURE_2D);
    }

};

namespace {
    static SFEditorFontImage* s_fontTexture = NULL; // owning pointer to internal font atlas which is used if user doesn't set custom sf::Texture.

    // data
    static bool s_windowHasFocus = false;
    static bool s_mousePressed[3] = { false, false, false };
    static bool s_touchDown[3] = { false, false, false };
    static bool s_mouseMoved = false;
    static sf::Vector2i s_touchPos;

    static const unsigned int NULL_JOYSTICK_ID = sf::Joystick::Count;
    static unsigned int s_joystickId = NULL_JOYSTICK_ID;

    static const unsigned int NULL_JOYSTICK_BUTTON = sf::Joystick::ButtonCount;
    static unsigned int s_joystickMapping[ImGuiNavInput_COUNT];

    struct StickInfo {
        sf::Joystick::Axis xAxis;
        sf::Joystick::Axis yAxis;

        bool xInverted;
        bool yInverted;

        float threshold;
    };

    StickInfo s_dPadInfo;
    StickInfo s_lStickInfo;

    // various helper functions
    ImColor toImColor(sf::Color c);
    ImVec2 getTopLeftAbsolute(const sf::FloatRect& rect);
    ImVec2 getDownRightAbsolute(const sf::FloatRect& rect);

    // Default mapping is XInput gamepad mapping
    void initDefaultJoystickMapping();

    // Returns first id of connected joystick
    unsigned int getConnectedJoystickId();

    void updateJoystickActionState(ImGuiIO& io, ImGuiNavInput_ action);
    void updateJoystickDPadState(ImGuiIO& io);
    void updateJoystickLStickState(ImGuiIO& io);

    // clipboard functions
    void setClipboardText(void* userData, const char* text);
    const char* getClipboadText(void* userData);
    std::string s_clipboardText;

    // mouse cursors
    void loadMouseCursor(ImGuiMouseCursor imguiCursorType, sf::Cursor::Type sfmlCursorType);
    void updateMouseCursor(sf::WindowBase& window);

    sf::Cursor* s_mouseCursors[ImGuiMouseCursor_COUNT];
    bool s_mouseCursorLoaded[ImGuiMouseCursor_COUNT];

    ImTextureID convertGLTextureHandleToImTextureID(GLuint glTextureHandle) {
        ImTextureID textureID = (ImTextureID)NULL;
        std::memcpy(&textureID, &glTextureHandle, sizeof(GLuint));
        return textureID;
    }
    void UpdateFontTexture() {
        ImGuiIO& io = ImGui::GetIO();
        unsigned char* pixels;
        int width, height;

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        auto& texture = *s_fontTexture;
        texture.m_FontTexture.create(width, height, pixels);
        texture.createTexture();
        //texture.update(pixels);
        io.Fonts->TexID = convertGLTextureHandleToImTextureID(texture.m_GLTextureHandle);
    }
    SFEditorFontImage& GetFontTexture() {
        return *s_fontTexture;
    }


} // end of anonymous namespace

namespace ImGui {
    namespace SFML {

        void Init(sf::WindowBase& window, const sf::Vector2f& displaySize, bool loadDefaultFont) {
#if __cplusplus < 201103L // runtime assert when using earlier than C++11 as no
            // static_assert support
            assert(sizeof(GLuint) <= sizeof(ImTextureID)); // ImTextureID is not large enough to fit
                                                           // GLuint.
#endif
            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            // tell ImGui which features we support
            io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
            io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
            io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
            io.BackendPlatformName = "imgui_impl_sfml";

            // init keyboard mapping
            io.KeyMap[ImGuiKey_Tab] = sf::Keyboard::Tab;
            io.KeyMap[ImGuiKey_LeftArrow] = sf::Keyboard::Left;
            io.KeyMap[ImGuiKey_RightArrow] = sf::Keyboard::Right;
            io.KeyMap[ImGuiKey_UpArrow] = sf::Keyboard::Up;
            io.KeyMap[ImGuiKey_DownArrow] = sf::Keyboard::Down;
            io.KeyMap[ImGuiKey_PageUp] = sf::Keyboard::PageUp;
            io.KeyMap[ImGuiKey_PageDown] = sf::Keyboard::PageDown;
            io.KeyMap[ImGuiKey_Home] = sf::Keyboard::Home;
            io.KeyMap[ImGuiKey_End] = sf::Keyboard::End;
            io.KeyMap[ImGuiKey_Insert] = sf::Keyboard::Insert;
#ifdef ANDROID
            io.KeyMap[ImGuiKey_Backspace] = sf::Keyboard::Delete;
#else
            io.KeyMap[ImGuiKey_Delete] = sf::Keyboard::Delete;
            io.KeyMap[ImGuiKey_Backspace] = sf::Keyboard::BackSpace;
#endif
            io.KeyMap[ImGuiKey_Space] = sf::Keyboard::Space;
            io.KeyMap[ImGuiKey_Enter] = sf::Keyboard::Return;
            io.KeyMap[ImGuiKey_Escape] = sf::Keyboard::Escape;
            io.KeyMap[ImGuiKey_A] = sf::Keyboard::A;
            io.KeyMap[ImGuiKey_C] = sf::Keyboard::C;
            io.KeyMap[ImGuiKey_V] = sf::Keyboard::V;
            io.KeyMap[ImGuiKey_X] = sf::Keyboard::X;
            io.KeyMap[ImGuiKey_Y] = sf::Keyboard::Y;
            io.KeyMap[ImGuiKey_Z] = sf::Keyboard::Z;

            s_joystickId = getConnectedJoystickId();

            for (unsigned int i = 0; i < ImGuiNavInput_COUNT; i++) {
                s_joystickMapping[i] = NULL_JOYSTICK_BUTTON;
            }

            initDefaultJoystickMapping();

            // clipboard
            io.SetClipboardTextFn = setClipboardText;
            io.GetClipboardTextFn = getClipboadText;

            // load mouse cursors
            for (int i = 0; i < ImGuiMouseCursor_COUNT; ++i) {
                s_mouseCursorLoaded[i] = false;
            }
            loadMouseCursor(ImGuiMouseCursor_Arrow, sf::Cursor::Arrow);
            loadMouseCursor(ImGuiMouseCursor_TextInput, sf::Cursor::Text);
            loadMouseCursor(ImGuiMouseCursor_ResizeAll, sf::Cursor::SizeAll);
            loadMouseCursor(ImGuiMouseCursor_ResizeNS, sf::Cursor::SizeVertical);
            loadMouseCursor(ImGuiMouseCursor_ResizeEW, sf::Cursor::SizeHorizontal);
            loadMouseCursor(ImGuiMouseCursor_ResizeNESW, sf::Cursor::SizeBottomLeftTopRight);
            loadMouseCursor(ImGuiMouseCursor_ResizeNWSE, sf::Cursor::SizeTopLeftBottomRight);
            loadMouseCursor(ImGuiMouseCursor_Hand, sf::Cursor::Hand);
            if (s_fontTexture) { // delete previously created texture
                delete s_fontTexture;
            }
            s_fontTexture = new SFEditorFontImage{};

            if (loadDefaultFont) {
                // this will load default font automatically
                // No need to call AddDefaultFont
                UpdateFontTexture();
            }
            s_windowHasFocus = window.hasFocus();
            io.DisplaySize = ImVec2(displaySize.x, displaySize.y);
        }

        void ProcessEvent(const sf::Event& event) {
            if (s_windowHasFocus) {
                ImGuiIO& io = ImGui::GetIO();

                switch (event.type) {
                case sf::Event::MouseMoved:
                    s_mouseMoved = true;
                    break;
                case sf::Event::MouseButtonPressed: // fall-through
                case sf::Event::MouseButtonReleased: {
                    int button = event.mouseButton.button;
                    if (event.type == sf::Event::MouseButtonPressed && button >= 0 && button < 3) {
                        s_mousePressed[event.mouseButton.button] = true;
                    }
                } break;
                case sf::Event::TouchBegan: // fall-through
                case sf::Event::TouchEnded: {
                    s_mouseMoved = false;
                    int button = event.touch.finger;
                    if (event.type == sf::Event::TouchBegan && button >= 0 && button < 3) {
                        s_touchDown[event.touch.finger] = true;
                    }
                } break;
                case sf::Event::MouseWheelScrolled:
                    if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel ||
                        (event.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel && io.KeyShift)) {
                        io.MouseWheel += event.mouseWheelScroll.delta;
                    }
                    else if (event.mouseWheelScroll.wheel == sf::Mouse::HorizontalWheel) {
                        io.MouseWheelH += event.mouseWheelScroll.delta;
                    }
                    break;
                case sf::Event::KeyPressed: // fall-through
                case sf::Event::KeyReleased:
                    io.KeysDown[event.key.code] = (event.type == sf::Event::KeyPressed);
                    break;
                case sf::Event::TextEntered:
                    // Don't handle the event for unprintable characters
                    if (event.text.unicode < ' ' || event.text.unicode == 127) {
                        break;
                    }
                    io.AddInputCharacter(event.text.unicode);
                    break;
                case sf::Event::JoystickConnected:
                    if (s_joystickId == NULL_JOYSTICK_ID) {
                        s_joystickId = event.joystickConnect.joystickId;
                    }
                    break;
                case sf::Event::JoystickDisconnected:
                    if (s_joystickId == event.joystickConnect.joystickId) { // used gamepad was
                                                                            // disconnected
                        s_joystickId = getConnectedJoystickId();
                    }
                    break;
                default:
                    break;
                }
            }

            switch (event.type) {
            case sf::Event::LostFocus:
                s_windowHasFocus = false;
                break;
            case sf::Event::GainedFocus:
                s_windowHasFocus = true;
                break;
            default:
                break;
            }
        }

        void Update(sf::WindowBase& window, sf::Time dt) {
            // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
            updateMouseCursor(window);

            if (!s_mouseMoved) {
                if (sf::Touch::isDown(0)) s_touchPos = sf::Touch::getPosition(0, window);

                Update(s_touchPos, static_cast<sf::Vector2f>(window.getSize()), dt);
            }
            else {
                Update(sf::Mouse::getPosition(window), static_cast<sf::Vector2f>(window.getSize()), dt);
            }

            if (ImGui::GetIO().MouseDrawCursor) {
                // Hide OS mouse cursor if imgui is drawing it
                window.setMouseCursorVisible(false);
            }
        }

        void Update(const sf::Vector2i& mousePos, const sf::Vector2f& displaySize, sf::Time dt) {
            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(displaySize.x, displaySize.y);

            io.DeltaTime = dt.asSeconds();

            if (s_windowHasFocus) {
                if (io.WantSetMousePos) {
                    sf::Vector2i newMousePos(static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
                    sf::Mouse::setPosition(newMousePos);
                }
                else {
                    io.MousePos = ImVec2(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y));
                }
                for (unsigned int i = 0; i < 3; i++) {
                    io.MouseDown[i] = s_touchDown[i] || sf::Touch::isDown(i) || s_mousePressed[i] || sf::Mouse::isButtonPressed((sf::Mouse::Button)i);
                    s_mousePressed[i] = false;
                    s_touchDown[i] = false;
                }
            }

            // Update Ctrl, Shift, Alt, Super state
            io.KeyCtrl = io.KeysDown[sf::Keyboard::LControl] || io.KeysDown[sf::Keyboard::RControl];
            io.KeyAlt = io.KeysDown[sf::Keyboard::LAlt] || io.KeysDown[sf::Keyboard::RAlt];
            io.KeyShift = io.KeysDown[sf::Keyboard::LShift] || io.KeysDown[sf::Keyboard::RShift];
            io.KeySuper = io.KeysDown[sf::Keyboard::LSystem] || io.KeysDown[sf::Keyboard::RSystem];

            // gamepad navigation
            if ((io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) && s_joystickId != NULL_JOYSTICK_ID) {
                updateJoystickActionState(io, ImGuiNavInput_Activate);
                updateJoystickActionState(io, ImGuiNavInput_Cancel);
                updateJoystickActionState(io, ImGuiNavInput_Input);
                updateJoystickActionState(io, ImGuiNavInput_Menu);

                updateJoystickActionState(io, ImGuiNavInput_FocusPrev);
                updateJoystickActionState(io, ImGuiNavInput_FocusNext);

                updateJoystickActionState(io, ImGuiNavInput_TweakSlow);
                updateJoystickActionState(io, ImGuiNavInput_TweakFast);

                updateJoystickDPadState(io);
                updateJoystickLStickState(io);
            }
        }

        void Shutdown() {
            ImGui::GetIO().Fonts->TexID = (ImTextureID)NULL;
            if (s_fontTexture) { // if internal texture was created, we delete it
                delete s_fontTexture;
                s_fontTexture = nullptr;
            }
            for (int i = 0; i < ImGuiMouseCursor_COUNT; ++i) {
                if (s_mouseCursorLoaded[i]) {
                    delete s_mouseCursors[i];
                    s_mouseCursors[i] = nullptr;

                    s_mouseCursorLoaded[i] = false;
                }
            }

            ImGui::DestroyContext();
        }

        void SetActiveJoystickId(unsigned int joystickId) {
            assert(joystickId < sf::Joystick::Count);
            s_joystickId = joystickId;
        }

        void SetJoytickDPadThreshold(float threshold) {
            assert(threshold >= 0.f && threshold <= 100.f);
            s_dPadInfo.threshold = threshold;
        }

        void SetJoytickLStickThreshold(float threshold) {
            assert(threshold >= 0.f && threshold <= 100.f);
            s_lStickInfo.threshold = threshold;
        }

        void SetJoystickMapping(int action, unsigned int joystickButton) {
            assert(action < ImGuiNavInput_COUNT);
            assert(joystickButton < sf::Joystick::ButtonCount);
            s_joystickMapping[action] = joystickButton;
        }

        void SetDPadXAxis(sf::Joystick::Axis dPadXAxis, bool inverted) {
            s_dPadInfo.xAxis = dPadXAxis;
            s_dPadInfo.xInverted = inverted;
        }

        void SetDPadYAxis(sf::Joystick::Axis dPadYAxis, bool inverted) {
            s_dPadInfo.yAxis = dPadYAxis;
            s_dPadInfo.yInverted = inverted;
        }

        void SetLStickXAxis(sf::Joystick::Axis lStickXAxis, bool inverted) {
            s_lStickInfo.xAxis = lStickXAxis;
            s_lStickInfo.xInverted = inverted;
        }

        void SetLStickYAxis(sf::Joystick::Axis lStickYAxis, bool inverted) {
            s_lStickInfo.yAxis = lStickYAxis;
            s_lStickInfo.yInverted = inverted;
        }

    } // end of namespace SFML

} // end of namespace ImGui

namespace {
    ImColor toImColor(sf::Color c) {
        return ImColor(static_cast<int>(c.r), static_cast<int>(c.g), static_cast<int>(c.b), static_cast<int>(c.a));
    }
    ImVec2 getTopLeftAbsolute(const sf::FloatRect& rect) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        return ImVec2(rect.left + pos.x, rect.top + pos.y);
    }
    ImVec2 getDownRightAbsolute(const sf::FloatRect& rect) {
        ImVec2 pos = ImGui::GetCursorScreenPos();
        return ImVec2(rect.left + rect.width + pos.x, rect.top + rect.height + pos.y);
    }

    unsigned int getConnectedJoystickId() {
        for (unsigned int i = 0; i < (unsigned int)sf::Joystick::Count; ++i) {
            if (sf::Joystick::isConnected(i)) return i;
        }

        return NULL_JOYSTICK_ID;
    }

    void initDefaultJoystickMapping() {
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_Activate, 0);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_Cancel, 1);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_Input, 3);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_Menu, 2);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_FocusPrev, 4);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_FocusNext, 5);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_TweakSlow, 4);
        ImGui::SFML::SetJoystickMapping(ImGuiNavInput_TweakFast, 5);

        ImGui::SFML::SetDPadXAxis(sf::Joystick::PovX);
        // D-pad Y axis is inverted on Windows
#ifdef _WIN32
        ImGui::SFML::SetDPadYAxis(sf::Joystick::PovY, true);
#else
        ImGui::SFML::SetDPadYAxis(sf::Joystick::PovY);
#endif

        ImGui::SFML::SetLStickXAxis(sf::Joystick::X);
        ImGui::SFML::SetLStickYAxis(sf::Joystick::Y);

        ImGui::SFML::SetJoytickDPadThreshold(5.f);
        ImGui::SFML::SetJoytickLStickThreshold(5.f);
    }

    void updateJoystickActionState(ImGuiIO& io, ImGuiNavInput_ action) {
        bool isPressed = sf::Joystick::isButtonPressed(s_joystickId, s_joystickMapping[action]);
        io.NavInputs[action] = isPressed ? 1.0f : 0.0f;
    }

    void updateJoystickDPadState(ImGuiIO& io) {
        float dpadXPos = sf::Joystick::getAxisPosition(s_joystickId, s_dPadInfo.xAxis);
        if (s_dPadInfo.xInverted) dpadXPos = -dpadXPos;

        float dpadYPos = sf::Joystick::getAxisPosition(s_joystickId, s_dPadInfo.yAxis);
        if (s_dPadInfo.yInverted) dpadYPos = -dpadYPos;

        io.NavInputs[ImGuiNavInput_DpadLeft] = dpadXPos < -s_dPadInfo.threshold ? 1.0f : 0.0f;
        io.NavInputs[ImGuiNavInput_DpadRight] = dpadXPos > s_dPadInfo.threshold ? 1.0f : 0.0f;

        io.NavInputs[ImGuiNavInput_DpadUp] = dpadYPos < -s_dPadInfo.threshold ? 1.0f : 0.0f;
        io.NavInputs[ImGuiNavInput_DpadDown] = dpadYPos > s_dPadInfo.threshold ? 1.0f : 0.0f;
    }

    void updateJoystickLStickState(ImGuiIO& io) {
        float lStickXPos = sf::Joystick::getAxisPosition(s_joystickId, s_lStickInfo.xAxis);
        if (s_lStickInfo.xInverted) lStickXPos = -lStickXPos;

        float lStickYPos = sf::Joystick::getAxisPosition(s_joystickId, s_lStickInfo.yAxis);
        if (s_lStickInfo.yInverted) lStickYPos = -lStickYPos;

        if (lStickXPos < -s_lStickInfo.threshold) {
            io.NavInputs[ImGuiNavInput_LStickLeft] = std::abs(lStickXPos / 100.f);
        }

        if (lStickXPos > s_lStickInfo.threshold) {
            io.NavInputs[ImGuiNavInput_LStickRight] = lStickXPos / 100.f;
        }

        if (lStickYPos < -s_lStickInfo.threshold) {
            io.NavInputs[ImGuiNavInput_LStickUp] = std::abs(lStickYPos / 100.f);
        }

        if (lStickYPos > s_lStickInfo.threshold) {
            io.NavInputs[ImGuiNavInput_LStickDown] = lStickYPos / 100.f;
        }
    }

    void setClipboardText(void* /*userData*/, const char* text) {
        sf::Clipboard::setString(sf::String::fromUtf8(text, text + std::strlen(text)));
    }

    const char* getClipboadText(void* /*userData*/) {
        std::basic_string<sf::Uint8> tmp = sf::Clipboard::getString().toUtf8();
        s_clipboardText = std::string(tmp.begin(), tmp.end());
        return s_clipboardText.c_str();
    }

    void loadMouseCursor(ImGuiMouseCursor imguiCursorType, sf::Cursor::Type sfmlCursorType) {
        s_mouseCursors[imguiCursorType] = new sf::Cursor();
        s_mouseCursorLoaded[imguiCursorType] = s_mouseCursors[imguiCursorType]->loadFromSystem(sfmlCursorType);
    }

    void updateMouseCursor(sf::WindowBase& window) {
        ImGuiIO& io = ImGui::GetIO();
        if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0) {
            ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
            if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None) {
                window.setMouseCursorVisible(false);
            } else {
                window.setMouseCursorVisible(true);

                sf::Cursor& c = s_mouseCursorLoaded[cursor] ? *s_mouseCursors[cursor] : *s_mouseCursors[ImGuiMouseCursor_Arrow];
                window.setMouseCursor(c);
            }
        }
    }

} // end of anonymous namespace
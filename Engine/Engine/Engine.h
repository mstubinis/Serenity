#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <GL/glew.h>
#include <GL/GL.h>
#include <SFML/Graphics.hpp>
#include <glm/glm.hpp>

#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Engine_Events.h"
#include "Engine_Physics.h"
#include "Engine_Sounds.h"

#include "GBuffer.h"

namespace Engine{
	namespace Detail{
		 class EngineClass final{
			public:
				static void initWindow(std::string name, unsigned int width, unsigned int height);
				static void initGame();

				#pragma region Event Handlers
				static void EVENT_RESIZE(unsigned int width, unsigned int height);
				static void EVENT_CLOSE();
				static void EVENT_LOST_FOCUS();
				static void EVENT_GAINED_FOCUS();
				static void EVENT_TEXT_ENTERED(sf::Event::TextEvent);
				static void EVENT_KEY_PRESSED(sf::Event::KeyEvent);
				static void EVENT_KEY_RELEASED(sf::Event::KeyEvent);
				static void EVENT_MOUSE_WHEEL_MOVED(sf::Event::MouseWheelEvent);
				static void EVENT_MOUSE_BUTTON_PRESSED(sf::Event::MouseButtonEvent);
				static void EVENT_MOUSE_BUTTON_RELEASED(sf::Event::MouseButtonEvent);
				static void EVENT_MOUSE_MOVED(sf::Event::MouseMoveEvent);
				static void EVENT_MOUSE_ENTERED();
				static void EVENT_MOUSE_LEFT();
				/*
				static void EVENT_JOYSTICK_BUTTON_PRESSED();
				static void EVENT_JOYSTICK_BUTTON_RELEASED();
				static void EVENT_JOYSTICK_MOVED();
				static void EVENT_JOYSTICK_CONNECTED();
				static void EVENT_JOYSTICK_DISCONNECTED();
				*/
				#pragma endregion

				static void RESET_EVENTS();

				static GLuint m_vao;
				static sf::Clock clock;

				static void update();
				static void render();

				static void init(std::string app_name, unsigned int width=0, unsigned int height=0);
				static void destruct();

				static void run();
		};
	};
	static float getAngleBetweenTwoVectors(glm::vec3 a, glm::vec3 b, bool degrees = true){
		if(degrees == true)
			return glm::acos(glm::dot(a,b)) * 57.2958f;
		return glm::acos(glm::dot(a,b));
	}
	static void alignTo(glm::quat& o, glm::vec3 direction,float speed=0, bool overTime=false){

		glm::vec3 r = glm::normalize(glm::vec3(1-2*(o.y*o.y+o.z*o.z),2*(o.x*o.y+o.w*o.z),2*(o.x*o.z-o.w*o.y)));
		glm::vec3 u = glm::normalize(glm::vec3(2*(o.x*o.y-o.w*o.z),1-2*(o.x*o.x+o.z*o.z),2*(o.y*o.z+o.w*o.x)));

		glm::vec3 forward = glm::normalize(glm::cross(r,u));
		direction = glm::normalize(direction);
		if(speed == 0){
			glm::vec3 xaxis = glm::normalize(glm::cross(glm::vec3(0,1,0), direction));
			glm::vec3 yaxis = glm::normalize(glm::cross(direction, xaxis));

			glm::mat3 rot;
			rot[0][0] = static_cast<float>(xaxis.x);
			rot[1][0] = static_cast<float>(yaxis.x);
			rot[2][0] = static_cast<float>(direction.x);
			rot[0][1] = static_cast<float>(xaxis.y);
			rot[1][1] = static_cast<float>(yaxis.y);
			rot[2][1] = static_cast<float>(direction.y);
			rot[0][2] = static_cast<float>(xaxis.z);
			rot[1][2] = static_cast<float>(yaxis.z);
			rot[2][2] = static_cast<float>(direction.z);
			o = glm::quat_cast(rot);
		}
		else{
			glm::vec3 cross = glm::normalize(glm::cross(direction,forward));
			if(overTime){
				float angle = getAngleBetweenTwoVectors(direction,forward) * (speed*Resources::dt());
				o = o * (glm::angleAxis(angle, -glm::vec3(cross)));
			}
			else{
				o = o * (glm::angleAxis(speed, -glm::vec3(cross)));
			}
		}
	}

	static sf::Window* getWindow(){ return Resources::Detail::ResourceManagement::m_Window; }
	static std::string getWindowName(){ return Resources::Detail::ResourceManagement::m_WindowName; }
	static sf::Vector2u getWindowSize(){ return Resources::Detail::ResourceManagement::m_Window->getSize(); }
	static sf::Mouse* getMouse(){ return Resources::Detail::ResourceManagement::m_Mouse; }
	static void setWindowIcon(Texture* texture){texture->generatePixelPointer();Resources::getWindow()->setIcon(texture->getWidth(),texture->getHeight(),texture->getPixelsPtr()); }
	static void showMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(true); }
	static void hideMouseCursor(){ Resources::getWindow()->setMouseCursorVisible(false); }
	static void stop(){ Resources::getWindow()->close(); }

	static void setFullScreen(bool b){
		sf::VideoMode videoMode = sf::VideoMode::getDesktopMode();
		unsigned int style = sf::Style::Fullscreen;
		if(!b){
			style = sf::Style::Default;
			videoMode.width = Resources::getWindowSize().x;
			videoMode.height = Resources::getWindowSize().y;
		}
		SAFE_DELETE(Renderer::Detail::RenderManagement::m_gBuffer);
		Resources::Detail::ResourceManagement::m_Window->create(videoMode,Resources::Detail::ResourceManagement::m_WindowName,style,Resources::getWindow()->getSettings());

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);

		Engine::Renderer::Detail::RenderManagement::m_gBuffer = new GBuffer(Resources::getWindowSize().x,Resources::getWindowSize().y);
		Engine::Detail::EngineClass::EVENT_RESIZE(Resources::getWindowSize().x,Resources::getWindowSize().y);
	}
};

namespace Game{
	void initResources();
	void initLogic();
	void update(float);
	void render();
	void cleanup();
};
#endif
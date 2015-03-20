#include "Engine.h"

Renderer* renderer;
Bullet* bullet;
ResourceManager* Resources;
glm::vec2 Mouse_Position, Mouse_Position_Previous,Mouse_Difference;
sf::Window* Window;
sf::Mouse* Mouse;

int main(){
	Engine::EngineClass* program = new Engine::EngineClass("Engine",900,600);

	program->Run();

	delete program;
    return 0;
}
#include "Engine.h"

class Renderer;
class PhysicsEngine;
class ResourceManager;

Renderer* renderer;
PhysicsEngine* physicsEngine;
ResourceManager* Resources;
sf::Window* Window;
sf::Mouse* Mouse;

int main(){
	Engine::EngineClass* program = new Engine::EngineClass("Engine",1024,768);

	program->Run();

	delete program;
    return 0;
}
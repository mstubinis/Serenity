#include "Engine.h"

class Renderer;
class PhysicsEngine;

Renderer* renderer;
PhysicsEngine* physicsEngine;

int main(){
	Engine::EngineClass* program = new Engine::EngineClass("Engine",1024,768);

	program->Run();

	delete program;
    return 0;
}
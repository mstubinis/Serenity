#include "Engine.h"

class PhysicsEngine;
PhysicsEngine* physicsEngine;

int main(){
	Engine::EngineClass* program = new Engine::EngineClass("Engine",1024,680);

	program->run();

	delete program;
    return 0;
}
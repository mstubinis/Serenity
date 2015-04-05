#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "ObjectDynamic.h"
#include "Planet.h"
#include "GameCamera.h"

class Game{
	private:
		ObjectDynamic* player;
		GameCamera* playerCamera;
	public:
		Game();
		~Game();

		void Init_Resources();
		void Init_Logic();

		void Update(float);
};
#endif
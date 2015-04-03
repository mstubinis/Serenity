#ifndef GAME_H
#define GAME_H

#include "Object.h"
#include "ObjectDynamic.h"
#include "Planet.h"

class Game{
	private:
		ObjectDynamic* player;
	public:
		Game();
		~Game();

		void Init_Resources();
		void Init_Logic();

		void Update(float);
};
#endif
#ifndef GAME_H
#define GAME_H

class Game{
	public:
		Game();
		~Game();

		void Init_Resources();
		void Init_Logic();

		void Update(float);
};
#endif
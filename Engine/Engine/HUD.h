#ifndef HUD_H
#define HUD_H

class Font;
class HUD{
	private:
		Font* m_Font;
	public:
		HUD();
		~HUD();

		void update(float);
		void render();
};
#endif
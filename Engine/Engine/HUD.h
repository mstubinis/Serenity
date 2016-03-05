#ifndef HUD_H
#define HUD_H

#include <glm/glm.hpp>

class Font;
class Object;
class Ship;

class HUD final{
	private:
		unsigned int m_TargetIterator;
		glm::vec3 m_Color;
		Font* m_Font;
		Ship* m_Player;
		void _renderCrosshair(Object*);


		glm::vec2 m_WarpIndicatorSize;


	public:
		HUD(Ship*);
		~HUD();

		void update(float);
		void render(bool=false);

		const glm::vec3 getColor() const { return m_Color; }
		void setColor(glm::vec3 c){ m_Color = c; }
};
#endif
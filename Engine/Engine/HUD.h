#ifndef HUD_H
#define HUD_H

#include <glm/glm.hpp>

class Font;
class Object;
class PlayerShip;

class HUD{
	private:
		unsigned int m_TargetIterator;
		glm::vec3 m_Color;
		Font* m_Font;
		PlayerShip* m_Player;
		void _renderCrosshair(Object*);
	public:
		HUD(PlayerShip*);
		~HUD();

		void update(float);
		void render(bool debug=false);

		const glm::vec3 getColor() const { return m_Color; }
		void setColor(glm::vec3 c){ m_Color = c; }
};
#endif
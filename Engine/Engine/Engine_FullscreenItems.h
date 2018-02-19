#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <memory>

namespace Engine{
	namespace Renderer{
		namespace Detail{
			class FullscreenTriangle final{
				private:
					class impl; std::unique_ptr<impl> m_i;
				public:
					FullscreenTriangle();
					~FullscreenTriangle();

					void render();
			};
			class FullscreenQuad final{
				private:
					class impl; std::unique_ptr<impl> m_i;
				public:
					FullscreenQuad();
					~FullscreenQuad();

					void render();
			};
		};
	};
};
#endif
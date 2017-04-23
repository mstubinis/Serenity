#pragma once
#ifndef ENGINE_PHYSICSLOADER_H
#define ENGINE_PHYSICSLOADER_H

class Collision;

namespace Engine{
    namespace Resources{
        namespace Load{
			namespace Physics{
				void load(const Collision*);
				void unload(const Collision*);
			};
        };
    };
};

#endif
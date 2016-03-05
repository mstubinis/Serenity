#ifndef MATH_H
#define MATH_H

#include "Object.h"

#include <vector>
#include <map>
#include <list>
#include <unordered_map>
#include <glm/glm.hpp>

namespace Game{

	namespace Pathfinding{

		static std::list<glm::v3> getPathFrom(Object* start, Object* finish);

	};

};
#endif
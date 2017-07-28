#pragma once
#ifndef MATH_H
#define MATH_H

#include "Engine_Math.h"
#include <list>

class Object;

namespace Game{
    namespace Pathfinding{
        static std::list<glm::vec3> getPathFrom(Object* start, Object* finish);
    };
};
#endif
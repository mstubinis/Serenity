#pragma once
#ifndef ENGINE_TYPES_TYPES_H
#define ENGINE_TYPES_TYPES_H

#include <serenity/types/ColorVector.h>
#include <serenity/types/Flag.h>
#include <serenity/types/ViewPointer.h>
//#include <serenity/types/SmallMap.h>
//#include <serenity/types/StaticString.h>
#include <serenity/resources/Handle.h>

namespace Engine {
    using ResourceCallback = void(*)(Handle);
}

#endif
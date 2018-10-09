#pragma once
#ifndef ENGINE_BUILT_IN_RESOURCES_H_INCLUDE_GUARD
#define ENGINE_BUILT_IN_RESOURCES_H_INCLUDE_GUARD

class Mesh;
class ShaderP;
namespace Engine{
    namespace epriv{
        struct InternalMeshes{
            static Mesh* PointLightBounds;
            static Mesh* RodLightBounds;
            static Mesh* SpotLightBounds;
        };
        struct InternalShaderPrograms{
            static ShaderP* Deferred;
            static ShaderP* Forward;
        };
    };
};


#endif
#pragma once
#ifndef ENGINE_BUILT_IN_RESOURCES_H
#define ENGINE_BUILT_IN_RESOURCES_H

class Mesh;
class ShaderProgram;
namespace Engine{
namespace epriv{
    struct InternalMeshes{
        static Mesh* PointLightBounds;
        static Mesh* RodLightBounds;
        static Mesh* SpotLightBounds;
    };
    struct InternalShaderPrograms{
        static ShaderProgram* Deferred;
        static ShaderProgram* Forward;
    };
};
};


#endif
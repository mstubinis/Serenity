#ifndef LIGHT_H
#define LIGHT_H
#include "Object.h"

enum LIGHT_TYPE {LIGHT_TYPE_POINT,LIGHT_TYPE_DIRECTIONAL, LIGHT_TYPE_SPOT};

#pragma region Light Structs
struct BaseLight{
    glm::vec3 Color;
    float AmbientIntensity;
    float DiffuseIntensity;
    BaseLight(){
        Color = glm::vec3(0,0,0);
        AmbientIntensity = 0;
        DiffuseIntensity = 0;
    }
};
struct DirectionalLight : public BaseLight{        
    glm::vec3 Direction;
    DirectionalLight(){
        Direction = glm::vec3(0,0,0);
    }
};
struct PointLight : public BaseLight{
    glm::vec3 Position;
    struct{
        float Constant;
        float Linear;
        float Exp;
    } Attenuation;
    PointLight(){
        Position = glm::vec3(0,0,0);
        Attenuation.Constant = 0;
        Attenuation.Linear = 0;
        Attenuation.Exp = 0;
    }
};
struct SpotLight : public PointLight{
    glm::vec3 Direction;
    float Cutoff;
    SpotLight(){
        Direction = glm::vec3(0,0,0);
        Cutoff = 0;
    }
};
#pragma endregion

#endif
#pragma once
#ifndef GAME_REUBIX_CUBE_OBJECT_INCLUDE_GUARD
#define GAME_REUBIX_CUBE_OBJECT_INCLUDE_GUARD

#include <vector>
#include <memory>
#include <glm/glm.hpp>
typedef unsigned int uint;

class ReubixCubeObject{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        ReubixCubeObject();
        ~ReubixCubeObject();

        void rotateUp1(bool left);
        void rotateUp2(bool left);
        void rotateUp3(bool left);

        void rotateLeft1(bool left);
        void rotateLeft2(bool left);
        void rotateLeft3(bool left);

        void rotateFront1(bool left);
        void rotateFront2(bool left);
        void rotateFront3(bool left);

};

#endif
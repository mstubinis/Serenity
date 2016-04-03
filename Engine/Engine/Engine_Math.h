#ifndef ENGINE_ENGINE_MATH_H
#define ENGINE_ENGINE_MATH_H

#include "Engine_Resources.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>

namespace glm{

    #ifdef ENGINE_PRECISION_NORMAL
    typedef float nType;
    typedef glm::detail::tvec3<float> v3;
    typedef glm::detail::tmat4x4<float> m4;
    #endif
    #ifdef ENGINE_PRECISION_HIGH
    typedef double nType;
    typedef glm::detail::tvec3<double> v3;
    typedef glm::detail::tmat4x4<double> m4;
    #endif
    #ifdef ENGINE_PRECISION_VERY_HIGH
    typedef long double nType;
    typedef glm::detail::tvec3<long double> v3;
    typedef glm::detail::tmat4x4<long double> m4;
    #endif

    typedef glm::detail::tvec3<float> v3_f;
    typedef glm::detail::tmat4x4<float> m4_f;
    typedef glm::detail::tvec3<double> v3_d;
    typedef glm::detail::tmat4x4<double> m4_d;
    typedef glm::detail::tvec3<long double> v3_ld;
    typedef glm::detail::tmat4x4<long double> m4_ld;
};

namespace Engine{
    namespace Math{
        glm::vec3 getScreenCoordinates(glm::vec3,bool clampToEdge = true);

        glm::vec3 midpoint(glm::v3,glm::v3);
        glm::vec3 midpoint(glm::vec3,glm::vec3);

        glm::v3 getForward(glm::quat& q);
        glm::v3 getRight(glm::quat& q);
        glm::v3 getUp(glm::quat& q);
        glm::v3 getColumnVector(const btRigidBody* b, unsigned int column);
        glm::v3 getForward(const btRigidBody* b);
        glm::v3 getRight(const btRigidBody* b);
        glm::v3 getUp(const btRigidBody* b);

        float getAngleBetweenTwoVectors(glm::vec3 a, glm::vec3 b, bool degrees = true);
        void alignTo(glm::quat& o, glm::vec3 direction,float speed=0, bool overTime=false);

        void setColor(glm::vec3&,float,float,float);
        void setColor(glm::vec4&,float,float,float,float);
    };
};
#endif
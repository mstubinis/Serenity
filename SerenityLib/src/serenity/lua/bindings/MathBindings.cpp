#include <serenity/lua/bindings/MathBindings.h>
#include <serenity/lua/LuaScript.h>
#include <LuaBridge/LuaBridge.h>

#include <serenity/dependencies/glm.h>

#define ADD_OPS(CLASS) .addFunction("__eq", std::function<bool(CLASS*,const CLASS&)>([](CLASS* a,const CLASS& b)->bool { return (*a) == b; })) \
.addFunction("__mul", std::function<CLASS(CLASS*, const CLASS&)>([](CLASS* vec, const CLASS& v) {(*vec) *= v; return *vec; })) \
.addFunction("__div", std::function<CLASS(CLASS*, const CLASS&)>([](CLASS* vec, const CLASS& v) {(*vec) /= v; return *vec; })) \
.addFunction("__add", std::function<CLASS(CLASS*, const CLASS&)>([](CLASS* vec, const CLASS& v) {(*vec) += v; return *vec; })) \
.addFunction("__sub", std::function<CLASS(CLASS*, const CLASS&)>([](CLASS* vec, const CLASS& v) {(*vec) -= v; return *vec; })) \
.addStaticFunction("length", &CLASS::length)

#define ADD_OPS_VEC(CLASS) ADD_OPS(CLASS) \

#define ADD_OPS_MAT(CLASS) ADD_OPS(CLASS) \

void Engine::priv::lua::bindings::createBindingsMath(lua_State* L) {
    luabridge::getGlobalNamespace(L)

#pragma region Quaternions
        //glm mat4 TODO: add more to this
        .beginClass<glm::fquat>("fquat")
            .addConstructor<void(*)(float, float, float, float)>()
            .addData("x", &glm::fquat::x)
            .addData("y", &glm::fquat::y)
            .addData("z", &glm::fquat::z)
            .addData("w", &glm::fquat::w)
        .endClass()

        //glm mat4 TODO: add more to this
        .beginClass<glm::dquat>("dquat")
            .addConstructor<void(*)(double, double, double, double)>()
            .addData("x", &glm::dquat::x)
            .addData("y", &glm::dquat::y)
            .addData("z", &glm::dquat::z)
            .addData("w", &glm::dquat::w)
        .endClass()

        .beginClass<glm_quat>("quat")
            .addConstructor<void(*)(decimal, decimal, decimal, decimal)>()
            .addData("x", &glm_quat::x)
            .addData("y", &glm_quat::y)
            .addData("z", &glm_quat::z)
            .addData("w", &glm_quat::w)
        .endClass()
#pragma endregion

#pragma region Matrices
        .beginClass<glm::fmat4>("fmat4")
            .addConstructor<void(*)(float)>()
            ADD_OPS_MAT(glm::fmat4)
        .endClass()
        .beginClass<glm::fmat3>("fmat3")
            .addConstructor<void(*)(float)>()
            ADD_OPS_MAT(glm::fmat3)
        .endClass()
        .beginClass<glm::fmat2>("fmat2")
            .addConstructor<void(*)(float)>()
            ADD_OPS_MAT(glm::fmat2)
        .endClass()
        .beginClass<glm::dmat4>("dmat4")
            .addConstructor<void(*)(float)>()
            ADD_OPS_MAT(glm::dmat4)
        .endClass()
        .beginClass<glm::dmat3>("dmat3")
            .addConstructor<void(*)(float)>()
            ADD_OPS_MAT(glm::dmat3)
        .endClass()
        .beginClass<glm::dmat2>("dmat2")
            .addConstructor<void(*)(float)>()
            ADD_OPS_MAT(glm::dmat2)
        .endClass()
        .beginClass<glm_mat4>("mat4")
            .addConstructor<void(*)(decimal)>()
            ADD_OPS_MAT(glm_mat4)
        .endClass()
        .beginClass<glm_mat3>("mat3")
            .addConstructor<void(*)(decimal)>()
            ADD_OPS_MAT(glm_mat3)
        .endClass()
        .beginClass<glm_mat2>("mat2")
            .addConstructor<void(*)(decimal)>()
            ADD_OPS_MAT(glm_mat2)
        .endClass()
#pragma endregion

#pragma region Vectors
        .beginClass<glm::ivec2>("ivec2")
            .addConstructor<void(*)(int, int)>()
            .addData("x", &glm::ivec2::x)
            .addData("y", &glm::ivec2::y)
            ADD_OPS_VEC(glm::ivec2)
        .endClass()

        .beginClass<glm::ivec3>("ivec3")
            .addConstructor<void(*)(int, int, int)>()
            .addData("x", &glm::ivec3::x)
            .addData("y", &glm::ivec3::y)
            .addData("z", &glm::ivec3::z)
            ADD_OPS_VEC(glm::ivec3)
        .endClass()

        .beginClass<glm::ivec4>("ivec4")
            .addConstructor<void(*)(int, int, int, int)>()
            .addData("x", &glm::ivec4::x)
            .addData("y", &glm::ivec4::y)
            .addData("z", &glm::ivec4::z)
            .addData("w", &glm::ivec4::w)
            ADD_OPS_VEC(glm::ivec4)
        .endClass()

        .beginClass<glm::dvec2>("dvec2")
            .addConstructor<void(*)(double, double)>()
            .addData("x", &glm::dvec2::x)
            .addData("y", &glm::dvec2::y)
            ADD_OPS_VEC(glm::dvec2)
        .endClass()

        .beginClass<glm::dvec3>("dvec3")
            .addConstructor<void(*)(double, double, double)>()
            .addData("x", &glm::dvec3::x)
            .addData("y", &glm::dvec3::y)
            .addData("z", &glm::dvec3::z)
            ADD_OPS_VEC(glm::dvec3)
        .endClass()

        .beginClass<glm::dvec4>("dvec4")
            .addConstructor<void(*)(double, double, double, double)>()
            .addData("x", &glm::dvec4::x)
            .addData("y", &glm::dvec4::y)
            .addData("z", &glm::dvec4::z)
            .addData("w", &glm::dvec4::w)
            ADD_OPS_VEC(glm::dvec4)
        .endClass()

        .beginClass<glm::fvec2>("fvec2")
            .addConstructor<void(*)(float, float)>()
            .addData("x", &glm::fvec2::x)
            .addData("y", &glm::fvec2::y)
            ADD_OPS_VEC(glm::fvec2)
        .endClass()

        .beginClass<glm::fvec3>("fvec3")
            .addConstructor<void(*)(float, float, float)>()
            .addData("x", &glm::fvec3::x)
            .addData("y", &glm::fvec3::y)
            .addData("z", &glm::fvec3::z)
            ADD_OPS_VEC(glm::fvec3)
        .endClass()

        .beginClass<glm::fvec4>("fvec4")
            .addConstructor<void(*)(float, float, float, float)>()
            .addData("x", &glm::fvec4::x)
            .addData("y", &glm::fvec4::y)
            .addData("z", &glm::fvec4::z)
            .addData("w", &glm::fvec4::w)
            ADD_OPS_VEC(glm::fvec4)
        .endClass()

        .beginClass<glm_vec2>("vec2")
            .addConstructor<void(*)(decimal, decimal)>()
            .addData("x", &glm_vec2::x)
            .addData("y", &glm_vec2::y)
            ADD_OPS_VEC(glm_vec2)
        .endClass()

        .beginClass<glm_vec3>("vec3")
            .addConstructor<void(*)(decimal, decimal, decimal)>()
            .addData("x", &glm_vec3::x)
            .addData("y", &glm_vec3::y)
            .addData("z", &glm_vec3::z)
            ADD_OPS_VEC(glm_vec3)
        .endClass()

        .beginClass<glm_vec4>("vec4")
            .addConstructor<void(*)(decimal, decimal, decimal, decimal)>()
            .addData("x", &glm_vec4::x)
            .addData("y", &glm_vec4::y)
            .addData("z", &glm_vec4::z)
            .addData("w", &glm_vec4::w)
            ADD_OPS_VEC(glm_vec4)
        .endClass()

        //bullet vector3 TODO: add more to this
        .beginClass<btVector3>("btVector3")
            .addConstructor<void(*)(const btScalar&, const btScalar&, const btScalar&)>()
            .addProperty("x", &btVector3::x, &btVector3::setX)
            .addProperty("y", &btVector3::y, &btVector3::setY)
            .addProperty("z", &btVector3::z, &btVector3::setZ)
            .addProperty("w", &btVector3::w, &btVector3::setW)
            .addFunction("angle", &btVector3::angle)
            .addFunction("cross", &btVector3::cross)
            .addFunction("absolute", &btVector3::absolute)
            .addFunction("closestAxis", &btVector3::closestAxis)
            .addFunction("dot", &btVector3::dot)
            .addFunction("distance", &btVector3::distance)
            .addFunction("distance2", &btVector3::distance2)
            .addFunction("isZero", &btVector3::isZero)
            .addFunction("rotate", &btVector3::rotate)
            .addFunction("triple", &btVector3::triple)
            .addFunction("dot3", &btVector3::dot3)
            .addFunction("lerp", &btVector3::lerp)
            .addFunction("minAxis", &btVector3::minAxis)
            .addFunction("maxAxis", &btVector3::maxAxis)
            .addFunction("normalize", &btVector3::normalize)
            .addFunction("norm", &btVector3::norm)
            .addFunction("normalized", &btVector3::normalized)
            .addFunction("furthestAxis", &btVector3::furthestAxis)
            .addFunction("__eq", std::function<bool(btVector3*, const btVector3&)>([](btVector3* a, const btVector3& b)->bool {return (*a) == b; }))
            .addFunction("__mul", std::function<btVector3(btVector3*, const btVector3&)>([](btVector3* vec, const btVector3& v) {(*vec) *= v; return *vec; }))
            .addFunction("__add", std::function<btVector3(btVector3*, const btVector3&)>([](btVector3* vec, const btVector3& v) {(*vec) += v; return *vec; }))
            .addFunction("__sub", std::function<btVector3(btVector3*, const btVector3&)>([](btVector3* vec, const btVector3& v) {(*vec) -= v; return *vec; }))
            .addFunction("__div", std::function<btVector3(btVector3*, const btVector3&)>([](btVector3* vec, const btVector3& v) {(*vec) = (*vec) / v; return *vec; }))
        .endClass()
#pragma endregion
    ;
}
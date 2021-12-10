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

void Engine::priv::lua::bindings::createBindingsMath(lua_State* L) {
    luabridge::getGlobalNamespace(L)

#pragma region Quaternions
        //glm mat4 TODO: add more to this
        .beginClass<glm::quat>("fquat")

        .endClass()

        //glm mat4 TODO: add more to this
        .beginClass<glm::dquat>("dquat")

        .endClass()
#pragma endregion

#pragma region Matrices
        //glm mat4 TODO: add more to this
        .beginClass<glm::mat4>("mat4")

        .endClass()
        //glm mat3 TODO: add more to this
        .beginClass<glm::mat3>("mat3")

        .endClass()
        //glm mat2 TODO: add more to this
        .beginClass<glm::mat2>("mat2")

        .endClass()
#pragma endregion

#pragma region Vectors
        .beginClass<glm::ivec2>("ivec2")
            .addConstructor<void(*)(int, int)>()
            .addData("x", &glm::ivec2::x)
            .addData("y", &glm::ivec2::y)
            ADD_OPS(glm::ivec2)
        .endClass()

        .beginClass<glm::ivec3>("ivec3")
            .addConstructor<void(*)(int, int, int)>()
            .addData("x", &glm::ivec3::x)
            .addData("y", &glm::ivec3::y)
            .addData("z", &glm::ivec3::z)
            ADD_OPS(glm::ivec3)
        .endClass()

        .beginClass<glm::ivec4>("ivec4")
            .addConstructor<void(*)(int, int, int, int)>()
            .addData("x", &glm::ivec4::x)
            .addData("y", &glm::ivec4::y)
            .addData("z", &glm::ivec4::z)
            .addData("w", &glm::ivec4::w)
            ADD_OPS(glm::ivec4)
        .endClass()

        .beginClass<glm::dvec2>("dvec2")
            .addConstructor<void(*)(double, double)>()
            .addData("x", &glm::dvec2::x)
            .addData("y", &glm::dvec2::y)
            ADD_OPS(glm::dvec2)
        .endClass()

        .beginClass<glm::dvec3>("dvec3")
            .addConstructor<void(*)(double, double, double)>()
            .addData("x", &glm::dvec3::x)
            .addData("y", &glm::dvec3::y)
            .addData("z", &glm::dvec3::z)
            ADD_OPS(glm::dvec3)
        .endClass()

        .beginClass<glm::dvec4>("dvec4")
            .addConstructor<void(*)(double, double, double, double)>()
            .addData("x", &glm::dvec4::x)
            .addData("y", &glm::dvec4::y)
            .addData("z", &glm::dvec4::z)
            .addData("w", &glm::dvec4::w)
            ADD_OPS(glm::dvec4)
        .endClass()

        .beginClass<glm::vec2>("fvec2")
            .addConstructor<void(*)(float, float)>()
            .addData("x", &glm::vec2::x)
            .addData("y", &glm::vec2::y)
            ADD_OPS(glm::vec2)
        .endClass()

        .beginClass<glm::vec3>("fvec3")
            .addConstructor<void(*)(float, float, float)>()
            .addData("x", &glm::vec3::x)
            .addData("y", &glm::vec3::y)
            .addData("z", &glm::vec3::z)
            ADD_OPS(glm::vec3)
        .endClass()

        .beginClass<glm::vec4>("fvec4")
            .addConstructor<void(*)(float, float, float, float)>()
            .addData("x", &glm::vec4::x)
            .addData("y", &glm::vec4::y)
            .addData("z", &glm::vec4::z)
            .addData("w", &glm::vec4::w)
            ADD_OPS(glm::vec4)
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
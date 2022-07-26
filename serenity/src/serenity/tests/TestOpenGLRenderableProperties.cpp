
#include <serenity/renderer/opengl/OpenGLRenderableProperties.h>

#include <catch.hpp>

TEST_CASE("OpenGLRenderableProperties") {

    OpenGLRenderableProperties test1;
    test1.setBlendEquation(GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT);
    REQUIRE((test1.getBlendEquation().first == GL_FUNC_SUBTRACT && test1.getBlendEquation().second == GL_FUNC_REVERSE_SUBTRACT));

    test1.setBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_CONSTANT_ALPHA);
    auto blendFuncSep = test1.getBlendFuncSeparate();
    REQUIRE(std::get<0>(blendFuncSep) == GL_SRC_ALPHA);
    REQUIRE(std::get<1>(blendFuncSep) == GL_ONE_MINUS_SRC_ALPHA);
    REQUIRE(std::get<2>(blendFuncSep) == GL_ONE);
    REQUIRE(std::get<3>(blendFuncSep) == GL_CONSTANT_ALPHA);
}

#include <serenity/ecs/ECS.h>
#include <serenity/ecs/systems/SystemTransformParentChild.h>

#include <catch.hpp>

TEST_CASE("SystemParentChild tests") {

    Engine::priv::ECS ecs;
    SystemTransformParentChild s{ ecs };
    REQUIRE(s.size() == 0);

    s.addChild(5, 3);
    s.addChild(5, 7);
    s.addChild(5, 9);

    auto block5 = s.getBlockIndices(5);
    REQUIRE((block5.first == 0 && block5.second == 3));

    s.removeChild(5, 7);

    block5 = s.getBlockIndices(5);
    REQUIRE((block5.first == 0 && block5.second == 2));

    s.addChild(15, 16);
    s.addChild(15, 17);
    s.addChild(15, 18);
    s.addChild(15, 19);
    s.addChild(15, 20);

    auto block15 = s.getBlockIndices(15);
    REQUIRE((block15.first == 3 && block15.second == 8));

    s.addChild(5, 15);
    block5 = s.getBlockIndices(5);
    REQUIRE((block5.first == 0 && block5.second == 8));

    s.removeChild(15, 16);
    s.removeChild(15, 17);
    s.removeChild(15, 18);
    s.removeChild(15, 19);
    s.removeChild(15, 20);
    s.removeChild(5, 3);
    s.removeChild(5, 9);

    REQUIRE(s.size() == 0);
}
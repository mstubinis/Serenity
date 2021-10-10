
#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/entity/EntityRAII.h>

#include <catch.hpp>

TEST_CASE("Entity tests") {
    Entity entity1;
    REQUIRE(entity1.null() == true);
    Entity entity2 = Entity{ 3,1,1 };
    REQUIRE(entity2.null() == false);
}
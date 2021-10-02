
#include <serenity/ecs/entity/Entity.h>
#include <serenity/ecs/entity/EntityBody.h>
#include <serenity/ecs/entity/EntityBodyRigid.h>
#include <serenity/ecs/entity/EntityRAII.h>
#include <serenity/ecs/entity/EntityRAIIBody.h>
#include <serenity/ecs/entity/EntityRAIIBodyRigid.h>

#include <catch.hpp>

TEST_CASE("Entity tests") {
    Entity entity1;
    REQUIRE(entity1.null() == true);
    Entity entity2 = Entity{ 3,1,1 };
    REQUIRE(entity2.null() == false);
}
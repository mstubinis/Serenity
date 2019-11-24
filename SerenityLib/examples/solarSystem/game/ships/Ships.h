#pragma once
#ifndef GAME_SHIPS_H
#define GAME_SHIPS_H

#include "akira/Akira.h"
#include "norway/Norway.h"
#include "intrepid/Intrepid.h"
#include "shrike/Shrike.h"
#include "brel/Brel.h"
#include "vorcha/Vorcha.h"
#include "defiant/Defiant.h"
#include "miranda/Miranda.h"
#include "constitution/Constitution.h"
#include "excelsior/Excelsior.h"
#include "nova/Nova.h"
#include "sovereign/Sovereign.h"

#include "../factions/Faction.h"
#include <core/engine/resources/Handle.h>

struct ShipInformation final {
    std::string            Class;
    FactionEnum::Type      Faction;
    FactionInformation     FactionInformation;
    std::vector<Handle>    MeshHandles;
    std::vector<Handle>    MaterialHandles;
    Handle                 IconTextureHandle;
    Handle                 IconBorderTextureHandle;
    double                 RespawnTime;
    float                  ThreatModifier;

    ShipInformation();
};
class Ships final{
    public:
        static std::unordered_map<std::string, ShipInformation> Database;

        static const glm::vec4& getShieldsColor(const FactionEnum::Type&);
        static const glm::vec4& getTextColor(const FactionEnum::Type&);
        static const std::string& getFactionNameShort(const FactionEnum::Type&);
        static const std::string& getFactionNameLong(const FactionEnum::Type&);

        static const FactionInformation& getFactionInformation(const std::string& shipClass);

        static void createShipEntry(const std::string& shipClass, const FactionEnum::Type& faction, const double respawnTime, const float threatModifier);

        static void init();
};

#endif
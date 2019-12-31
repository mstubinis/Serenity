#pragma once
#ifndef GAME_SHIPS_H
#define GAME_SHIPS_H

//federation
#include "defiant/Defiant.h"
#include "akira/Akira.h"
#include "sovereign/Sovereign.h"

#include "norway/Norway.h"
#include "intrepid/Intrepid.h"
#include "miranda/Miranda.h"
#include "constitution/Constitution.h"
#include "excelsior/Excelsior.h"
#include "nova/Nova.h"
#include "fed_def_platform/FedDefPlatform.h"
#include "fed_starbase_mushroom/FedStarbaseMushroom.h"

//klingon
#include "brel/Brel.h"
#include "vorcha/Vorcha.h"
#include "neghvar/Neghvar.h"

//romulan
#include "shrike/Shrike.h"
#include "venerex/Venerex.h"
#include "dderidex/Dderidex.h"

//borg
#include "probe/Probe.h"
#include "sphere/Sphere.h"
#include "diamond/Diamond.h"
#include "cube/Cube.h"


#include "../factions/Faction.h"
#include <core/engine/resources/Handle.h>

struct ShipInformation final {
    std::string                    Class;
    FactionEnum::Type              Faction;
    FactionInformation             FactionInformation;
    std::vector<Handle>            MeshHandles;
    std::vector<Handle>            MaterialHandles;
    Handle                         IconTextureHandle;
    Handle                         IconBorderTextureHandle;
    double                         RespawnTime;
    float                          ThreatModifier;
    bool                           PrintClassNameOnHUD;
    std::vector<glm::vec3>         HullImpactPoints;

    ShipInformation();
    ~ShipInformation();
};
class Ships final{
    public:
        static std::unordered_map<std::string, ShipInformation> Database;

        static const glm::vec4& getShieldsColor(const FactionEnum::Type&);
        static const glm::vec4& getTextColor(const FactionEnum::Type&);
        static const std::string& getFactionNameShort(const FactionEnum::Type&);
        static const std::string& getFactionNameLong(const FactionEnum::Type&);

        static const FactionInformation& getFactionInformation(const std::string& shipClass);

        static void createShipEntry(const std::string& shipClass, const FactionEnum::Type& faction, const double respawnTime, const float threatModifier, const bool printClassNameOnHUD = true);

        static void init();
        static void destruct();
};

#endif
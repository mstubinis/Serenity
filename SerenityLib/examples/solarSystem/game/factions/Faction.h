#pragma once
#ifndef GAME_FACTION_H
#define GAME_FACTION_H

#include <glm/vec4.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct FactionEnum final {enum Type {
    Unknown,
    Federation,
    Klingon,
    Romulan,
    Borg,
    //Cardassian,
    //Dominion,
    //Ferengi,
    _TOTAL,
};};

struct FactionInformation final {
    glm::vec4   ColorShield;
    glm::vec4   ColorText;
    std::string NameShort;
    std::string NameLong;

    FactionInformation();
    FactionInformation(const std::string& nameLong, const std::string& nameShort, const glm::vec4& colorText, const glm::vec4& colorShield);
};

class Factions final {
    public:
        static std::vector<FactionInformation> Database;
        static void init();
};

#endif
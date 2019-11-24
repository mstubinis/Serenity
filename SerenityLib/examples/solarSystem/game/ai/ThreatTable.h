#pragma once
#ifndef GAME_AI_THREAT_TABLE_H
#define GAME_AI_THREAT_TABLE_H

#include <unordered_map>
#include <vector>

class Map;
class Ship;
class ThreatTable {
    private:
        Map& m_Map;
        std::unordered_map<std::string, unsigned int>        m_ThreatTable;
        std::vector<std::pair<std::string, unsigned int>>    m_ThreatSorted;
    public:
        ThreatTable(Map&);
        ~ThreatTable();

        void modify_threat(const std::string& key, const unsigned int& val, const bool do_sort = true);

        void sort();
        void clear();

        Ship* getHighestThreat();

        std::vector<std::pair<std::string, unsigned int>>& getShipsWithThreat();

        const float getThreatPercent(Ship& ship);
        void clearSpecificSource(const std::string& source);
};

#endif
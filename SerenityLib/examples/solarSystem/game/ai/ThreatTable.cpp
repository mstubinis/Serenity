#include "ThreatTable.h"
#include <algorithm>
#include <execution>

#include "../map/Map.h"
#include "../Ship.h"

using namespace std;

ThreatTable::ThreatTable(Map& map) : m_Map(map){
    clear();
}
ThreatTable::~ThreatTable() {
    clear();
}

void ThreatTable::modify_threat(const string& key, const unsigned int& val, const bool do_sort) {
    //TODO: might need count checking and initialization to zero?
    m_ThreatTable[key] += val;

    //TODO: O(n) searching here, see if this can be optimized
    bool found = false;
    for (auto& pair_itr : m_ThreatSorted) {
        if (!found && pair_itr.first == key) {
            pair_itr.second += val;
            found = true;
            break;
        }
    }
    if (!found) {
        m_ThreatSorted.push_back(std::make_pair(key, val));
    }
    if (do_sort)
        sort();
}
void ThreatTable::sort() {
    auto lamda_sort = [&](pair<string, unsigned int>& lhs, pair<string, unsigned int>& rhs) {
        return lhs.second > rhs.second;
    };
    std::sort(  std::execution::par_unseq,  m_ThreatSorted.begin(),  m_ThreatSorted.end(),  lamda_sort);
}
void ThreatTable::clear() {
    m_ThreatTable.clear();
    m_ThreatSorted.clear();
}
Ship* ThreatTable::getHighestThreat() {
    if (m_ThreatSorted.size() > 0) {
        const auto& key = m_ThreatSorted[0].first;
        return m_Map.getShips().at(key);
    }
    return nullptr;
}
const float ThreatTable::getThreatPercent(Ship& ship) {
    const auto& key = ship.getMapKey();
    if (!m_ThreatTable.count(key)) {
        return 0.0f;
    }
    const auto& highest_threat = m_ThreatSorted[0].second;
    const auto& my_threat = m_ThreatTable[key];
    return static_cast<float>(my_threat / static_cast<float>(highest_threat)) * 100.0f;
}
void ThreatTable::clearSpecificSource(const string& source) {
    if (m_ThreatTable.count(source)) {
        m_ThreatTable.erase(source);
    }
    //TODO: O(n) searching here, see if this can be optimized
    for (size_t i = 0; i < m_ThreatSorted.size(); ++i) {
        if (m_ThreatSorted[i].first == source) {
            m_ThreatSorted.erase(m_ThreatSorted.begin() + i);
            break;
        }
    }
}
vector<pair<string, unsigned int>>& ThreatTable::getShipsWithThreat() {
    return m_ThreatSorted;
}
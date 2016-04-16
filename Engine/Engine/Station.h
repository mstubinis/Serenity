#pragma once
#ifndef STATION_H
#define STATION_H

#include "ObjectDynamic.h"
#include "ObjectDisplay.h"
#include <map>

enum STATION_PRESET{
    STATION_PRESET_SHIPYARD_SMALL,
    STATION_PRESET_SHIPYARD_MEDIUM,
    STATION_PRESET_SHIPYARD_LARGE,
    STATION_PRESET_COMMAND_SMALL,
    STATION_PRESET_COMMAND_MEDIUM,
    STATION_PRESET_COMMAND_LARGE
};

struct StationPartConnectorsInfo{
    std::vector<glm::vec3> connectorLocations;
    std::vector<glm::vec3> connectorDirections;
    StationPartConnectorsInfo(std::vector<glm::vec3> _connectors,std::vector<glm::vec3> _directions){
        connectorLocations = _connectors;
        connectorDirections = _directions;
    }
    ~StationPartConnectorsInfo(){
        connectorLocations.clear();
        connectorDirections.clear();
    }
};
struct StationPart{
    StationPart* parent;
    DisplayItem* displayItem;
    StationPartConnectorsInfo* connectorsInfo;
    std::vector<unsigned int> usedPartsIndices;
    StationPart(DisplayItem* _displayItem, StationPartConnectorsInfo* _s){
        displayItem = _displayItem;
        connectorsInfo = _s;
    }
    ~StationPart(){
        delete displayItem;
        delete connectorsInfo;
        usedPartsIndices.clear();
    }
};

static std::map<std::string,StationPartConnectorsInfo*> stationPartsInfo;

class Station: public ObjectDynamic{
    protected:
        std::vector<StationPart*> m_StationParts; // the first element is always the root element
    public:
        Station(std::string = "",
                std::string = "",
                glm::v3 = glm::v3(0),    //Position
                glm::vec3 = glm::vec3(1),//Scale
                std::string = "Station", //Object name
                Collision* = nullptr,    //Bullet Collision Shape
                Scene* = nullptr
               );
        virtual ~Station();

        glm::vec2 getGravityInfo(){ return glm::vec2(getRadius()*100,getRadius()*150); }

        virtual void update(float);

        virtual void addStationPart(StationPart*,unsigned int, StationPart* = nullptr,unsigned int = -1);
        virtual StationPart* getRootStationPart(){ return m_StationParts[0]; }
};
#endif
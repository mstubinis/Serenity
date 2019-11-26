#include "Ships.h"
#include <core/engine/resources/Engine_Resources.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialComponent.h>

using namespace std;
using namespace Engine;

unordered_map<string, ShipInformation> Ships::Database;


ShipInformation::ShipInformation() {
    Faction = FactionEnum::Unknown;
    Class   = "";
}


const glm::vec4& Ships::getShieldsColor(const FactionEnum::Type& faction) {
    return Factions::Database[faction].ColorShield;
}
const glm::vec4& Ships::getTextColor(const FactionEnum::Type& faction) {
    return Factions::Database[faction].ColorText;
}
const string& Ships::getFactionNameShort(const FactionEnum::Type& faction) {
    return Factions::Database[faction].NameShort;
}
const string& Ships::getFactionNameLong(const FactionEnum::Type& faction) {
    return Factions::Database[faction].NameLong;
}
const FactionInformation& Ships::getFactionInformation(const string& shipClass) {
    return Database[shipClass].FactionInformation;
}

void Ships::createShipEntry(const string& shipClass, const FactionEnum::Type& faction, const double respawnTime, const float threatModifier, const bool printClassNameOnHUD) {
    if (Database.count(shipClass))
        return;

    ShipInformation info;

    info.Class                  = shipClass;
    info.Faction                = faction;
    info.FactionInformation     = Factions::Database[faction];
    info.RespawnTime            = respawnTime;
    info.ThreatModifier         = threatModifier;
    info.PrintClassNameOnHUD    = printClassNameOnHUD;

    auto ship_class_lower = boost::algorithm::to_lower_copy(shipClass);
    boost::replace_all(ship_class_lower, "'", "");
    boost::replace_all(ship_class_lower, " ", "_");

    string root = "../data/Ships/" + ship_class_lower + "/";
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) 
        return;

    boost::filesystem::recursive_directory_iterator it(root);
    boost::filesystem::recursive_directory_iterator endit;

    map<string, boost::filesystem::path> objcc_files;
    vector<string> texture_files_contains{
        ship_class_lower,
        ship_class_lower + "_Normal",
        ship_class_lower + "_Glow",
        ship_class_lower + "_Specular",
        ship_class_lower + "_AO",
        ship_class_lower + "_Metalness",
        ship_class_lower + "_Smoothness",
    };
    unordered_map<string, boost::filesystem::path> texture_files;
    string diffuse, normal, glow, specular, ao, metalness, smoothness = "";

    while (it != endit){
        const auto extension = it->path().extension().string();
        const auto path_string = it->path().string();

        if (boost::filesystem::is_regular_file(*it)) {
            if (extension == ".objcc" || extension == ".obj") {
                objcc_files.emplace(path_string, it->path());
            }else if ( extension == ".dds") {
                for (auto& texture_contain : texture_files_contains) {
                    if (boost::algorithm::contains(path_string, texture_contain + ".dds")) {
                        auto& path = it->path();
                        texture_files.emplace(texture_contain, path);
                        break;
                    }
                }
                if (boost::algorithm::contains(path_string, ship_class_lower + "_icon_Border")) { //ship icon border texture
                    auto handle = Resources::loadTextureAsync(path_string); //TODO: include and modify second parameter?
                    info.IconBorderTextureHandle = handle;
                }else if (boost::algorithm::contains(path_string, ship_class_lower + "_icon")) { //ship icon texture
                    auto handle = Resources::loadTextureAsync(path_string); //TODO: include and modify second parameter?
                    info.IconTextureHandle = handle;
                }
            }
        }
        ++it;
    }
    //get objcc files
#pragma region Meshes
    for(auto& path : objcc_files){
        auto handles = Resources::loadMeshAsync(path.second.string());
        for (auto& handle : handles) {
            info.MeshHandles.push_back(handle);
        }
    }
#pragma endregion


#pragma region Materials
    for (auto& entry : texture_files) {
        const auto path = entry.second.string();
        if (!path.empty()) {
            if (boost::algorithm::contains(path, "_Normal")) {
                normal = path;
            }else if (boost::algorithm::contains(path, "_Glow")) {
                glow = path;
            }else if (boost::algorithm::contains(path, "_Specular")) {
                specular = path;
            }else if (boost::algorithm::contains(path, "_AO")) {
                ao = path;
            }else if (boost::algorithm::contains(path, "_Metalness")) {
                metalness = path;
            }else if (boost::algorithm::contains(path, "_Smoothness")) {
                smoothness = path;
            }else{
                diffuse = path;
            }
        }
    }
    auto mat_handle = Resources::loadMaterialAsync(shipClass + "Material", diffuse, normal, glow, specular, ao, metalness, smoothness);
    auto& mat = *(Material*)mat_handle.get();
    info.MaterialHandles.push_back(mat_handle);

#pragma endregion

    Database.emplace(shipClass, info);
}
void Ships::init() {
    createShipEntry("Defiant",          FactionEnum::Federation,   6,    1.0f);
    createShipEntry("Nova",             FactionEnum::Federation,   6,    1.0f);
    createShipEntry("Akira",            FactionEnum::Federation,   20,   1.2f);
    createShipEntry("Intrepid",         FactionEnum::Federation,   20,   1.1f);
    createShipEntry("Excelsior",        FactionEnum::Federation,   20,   1.2f);
    createShipEntry("Constitution",     FactionEnum::Federation,   20,   1.1f);
    createShipEntry("Miranda",          FactionEnum::Federation,   6,    1.0f);
    //createShipEntry("Saber",          FactionEnum::Federation,   6,    1.0f);
    //createShipEntry("Norway",         FactionEnum::Federation,   8,    1.05f);
    //createShipEntry("Steamrunner",    FactionEnum::Federation,   20,   1.1f);
    //createShipEntry("Galaxy",         FactionEnum::Federation,   30,   1.3f);
    //createShipEntry("Nebula",         FactionEnum::Federation,   30,   1.25f);
    createShipEntry("Sovereign",        FactionEnum::Federation,   30,   1.35f);
    //createShipEntry("Liberty",        FactionEnum::Federation,   40,   1.5f);
    createShipEntry("Federation Defense Platform", FactionEnum::Federation,   60,    1.0f, false);

    createShipEntry("B'rel",            FactionEnum::Klingon,      6,    1.0f);
    createShipEntry("Vor'cha",          FactionEnum::Klingon,      20,   1.2f);
    //createShipEntry("Negh'var",       FactionEnum::Klingon,      30,   1.35f);
    //createShipEntry("Kahless",        FactionEnum::Klingon,      40,   1.5f);

    createShipEntry("Shrike",           FactionEnum::Romulan,      6,    1.0f);
    //createShipEntry("Venerex",        FactionEnum::Romulan,      20,   1.2f);
    //createShipEntry("D'deridex",      FactionEnum::Romulan,      30,   1.35f);
    //createShipEntry("Aeterna",        FactionEnum::Romulan,      40,   1.5f);

    Material& defMat = *((Material*)Database["Defiant"].MaterialHandles[0].get());
    auto* layer = defMat.getComponent(0).addLayer();
    layer->setTexture("../data/Textures/Effects/Buzzards.dds");
    layer->setMask("../data/Ships/defiant/defiant_Mask_1.dds");
    layer->addUVModificationSimpleTranslation(0.02f, 0.02f);

    Material& novaMat = *((Material*)Database["Nova"].MaterialHandles[0].get());
    auto* layer1 = novaMat.getComponent(0).addLayer();
    layer1->setTexture("../data/Textures/Effects/Buzzards.dds");
    layer1->setMask("../data/Ships/nova/nova_Mask_1.dds");
    layer1->addUVModificationSimpleTranslation(0.02f, 0.02f);

    Material& vorchaMat = *((Material*)Database["Vor'cha"].MaterialHandles[0].get());
    auto* layer2 = vorchaMat.getComponent(0).addLayer();
    layer2->setTexture("../data/Textures/Effects/Buzzards.dds");
    layer2->setMask("../data/Ships/vorcha/vorcha_Mask_1.dds");
    layer2->addUVModificationSimpleTranslation(0.02f, 0.02f);
}

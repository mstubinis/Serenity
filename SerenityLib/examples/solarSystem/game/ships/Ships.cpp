#include "Ships.h"
#include <core/engine/resources/Engine_Resources.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <core/engine/materials/Material.h>

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

void Ships::createShipEntry(const string& shipClass, const FactionEnum::Type& faction) {
    if (Database.count(shipClass))
        return;

    ShipInformation info;

    info.Class = shipClass;
    info.Faction = faction;
    info.FactionInformation = Factions::Database[faction];

    const auto ship_class_lower = boost::algorithm::to_lower_copy(shipClass);

    //get objcc files
#pragma region Meshes
    map<string, boost::filesystem::path> objcc_files;
    string root = "../data/Ships/" + ship_class_lower + "/";
    if (!boost::filesystem::exists(root) || !boost::filesystem::is_directory(root)) 
        return;

    boost::filesystem::recursive_directory_iterator it(root);
    boost::filesystem::recursive_directory_iterator endit;
    while (it != endit){
        if (boost::filesystem::is_regular_file(*it) && it->path().extension() == ".objcc")
            objcc_files.emplace(it->path().filename().string(), it->path());
        ++it;

    }
    for(auto& path : objcc_files){
        auto handles = Resources::loadMeshAsync(path.second.string());
        for (auto& handle : handles) {
            info.MeshHandles.push_back(handle);
        }
    }
#pragma endregion

    //get material files (.dds)
#pragma region Materials
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

    boost::filesystem::recursive_directory_iterator it1(root);
    boost::filesystem::recursive_directory_iterator endit1;
    while (it1 != endit1) {
        if (boost::filesystem::is_regular_file(*it1) && it1->path().extension() == ".dds") {

            for (auto& texture_contain : texture_files_contains) {
                auto& contain_str = it1->path().string();
                if (boost::algorithm::contains(contain_str, texture_contain + ".dds")) {
                    auto& path = it1->path();
                    texture_files.emplace(texture_contain, path);
                    break;
                }
            }
        }
        ++it1;

    }
    string diffuse, normal, glow, specular, ao, metalness, smoothness = "";
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
    auto mat_handle = Resources::loadMaterialAsync(shipClass + "Material", diffuse, normal, glow, specular);
    auto& mat = *(Material*)mat_handle.get();
    info.MaterialHandles.push_back(mat_handle);

    if (!ao.empty()) {
        mat.addComponentAO(ao, 1.0f);
    }
    if (!metalness.empty()) {
        mat.addComponentMetalness(metalness, 1.0f);
    }
    if (!smoothness.empty()) {
        mat.addComponentSmoothness(smoothness, 1.0f);
    }

#pragma endregion

    Database.emplace(shipClass, info);
}
void Ships::init() {
    createShipEntry("Defiant", FactionEnum::Federation);
    createShipEntry("Nova", FactionEnum::Federation);
    createShipEntry("Akira", FactionEnum::Federation);
    createShipEntry("Intrepid", FactionEnum::Federation);
    createShipEntry("Excelsior", FactionEnum::Federation);
    createShipEntry("Constitution", FactionEnum::Federation);
    createShipEntry("Miranda", FactionEnum::Federation);

    createShipEntry("Brel", FactionEnum::Klingon);

    createShipEntry("Shrike", FactionEnum::Romulan);


    Material& defMat = *((Material*)Database["Defiant"].MaterialHandles[0].get());
    auto* layer = defMat.getComponent(0).addLayer();
    layer->setTexture("../data/Textures/Effects/Buzzards.dds");
    layer->setMask("../data/Ships/defiant/defiant_Mask_1.dds");
    layer->addUVModificationSimpleTranslation(0.02f, 0.02f);
    defMat.addComponent(MaterialComponentType::Normal, "../data/Ships/defiant/defiant_Normal.dds");
    defMat.addComponent(MaterialComponentType::Glow, "../data/Ships/defiant/defiant_Glow.dds");

    Material& novaMat = *((Material*)Database["Nova"].MaterialHandles[0].get());
    auto* layer1 = novaMat.getComponent(0).addLayer();
    layer1->setTexture("../data/Textures/Effects/Buzzards.dds");
    layer1->setMask("../data/Ships/nova/nova_Mask_1.dds");
    layer1->addUVModificationSimpleTranslation(0.02f, 0.02f);
    novaMat.addComponent(MaterialComponentType::Normal, "../data/Ships/nova/nova_Normal.dds");
    novaMat.addComponent(MaterialComponentType::Glow,  "../data/Ships/nova/nova_Glow.dds");
}

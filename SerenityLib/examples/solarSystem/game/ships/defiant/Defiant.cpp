#include "Defiant.h"
#include "../../ResourceManifest.h"
#include "../shipSystems/ShipSystemWeapons.h"
#include "../../weapons/cannons/PulsePhaser.h"
#include "../../weapons/beams/PhaserBeam.h"
#include "../../weapons/torpedos/QuantumTorpedo.h"
#include "../../weapons/torpedos/PhotonTorpedo.h"

#include "../../ships/shipSystems/ShipSystemCloakingDevice.h"
#include "../../ships/shipSystems/ShipSystemMainThrusters.h"
#include "../../ships/shipSystems/ShipSystemPitchThrusters.h"
#include "../../ships/shipSystems/ShipSystemReactor.h"
#include "../../ships/shipSystems/ShipSystemRollThrusters.h"
#include "../../ships/shipSystems/ShipSystemSensors.h"
#include "../../ships/shipSystems/ShipSystemShields.h"
#include "../../ships/shipSystems/ShipSystemWarpDrive.h"
#include "../../ships/shipSystems/ShipSystemYawThrusters.h"
#include "../../ships/shipSystems/ShipSystemHull.h"
#include "../../ai/AI.h"
#include "../Ships.h"

#include "../../config/Keybinds.h"

constexpr auto CLASS = "Defiant";

using namespace std;

Defiant::Defiant(Scene& scene, glm::vec3 position, glm::vec3 scale)
    :Ship(CLASS, scene, position, scale) {

}

Defiant::Defiant(AIType::Type& ai_type, Team& team, Client& client, Map& map, const string& name, glm::vec3 position, glm::vec3 scale, CollisionType::Type collisionType)
:Ship(team,client, CLASS,map,ai_type,name,position,scale, collisionType){

    m_Perks         = Defiant::Perks::None;
    m_UnlockedPerks = Defiant::Perks::None;

    auto& _this = *this;
    for (uint i = 0; i < ShipSystemType::_TOTAL; ++i) {
        ShipSystem* system = nullptr;
        if (i == 0)  system = NEW ShipSystemReactor(_this, 1000);
        else if (i == 1)  system = NEW ShipSystemPitchThrusters(_this);
        else if (i == 2)  system = NEW ShipSystemYawThrusters(_this);
        else if (i == 3)  system = NEW ShipSystemRollThrusters(_this);
        else if (i == 4)  system = NEW ShipSystemCloakingDevice(_this);
        else if (i == 5)  system = NEW ShipSystemShields(_this, map, 9000.0f, 9000.0f, 9000.0f, 9000.0f, 12000.0f, 12000.0f, glm::vec3(0.0f), glm::vec3(1.15f, 2.1f, 1.15f));
        else if (i == 6)  system = NEW ShipSystemMainThrusters(_this);
        else if (i == 7)  system = NEW ShipSystemWarpDrive(_this);
        else if (i == 8)  system = NEW ShipSystemSensors(_this, map);
        else if (i == 9)  system = NEW ShipSystemWeapons(_this);
        else if (i == 10)  system = NEW ShipSystemHull(_this, map, 17000.0f + (m_UnlockedPerks & Defiant::Perks::AblativeArmor) ? 4000.0f : 0.0f);
        m_ShipSystems.emplace(i, system);
    }
    internal_finialize_init(ai_type);

    /*
    1x dorsal phaser bank
    1x port-ventral phaser bank
    1x starboard-ventral phaser bank

    4x fwd pulse phaser cannons

    2x fwd quantum torpedos
    1x aft photon torpedo
    */

    auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

    auto* leftTopCannon     = NEW PulsePhaser(_this, map, glm::vec3(-0.573355f, 0.072793f, -0.207088f), glm::vec3(0.0055f, 0, -1), 17.0f, 6,250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);
    auto* leftBottomCannon = NEW PulsePhaser(_this, map, glm::vec3(-0.434018f, -0.163775, -0.093399), glm::vec3(0.005f, 0, -1), 17.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);
    auto* rightBottomCannon = NEW PulsePhaser(_this, map, glm::vec3(0.434018f, -0.163775, -0.093399), glm::vec3(-0.005f, 0, -1), 17.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);
    auto* rightTopCannon = NEW PulsePhaser(_this, map, glm::vec3(0.573355f, 0.072793f, -0.207088f), glm::vec3(-0.0055f, 0, -1), 17.0f, 6, 250, 0.7f, 2.5f, 1.8f, 40.5f, 25.0f);

    auto* leftTorp = NEW QuantumTorpedo(_this, map, glm::vec3(-0.358887f, 0.023574f, -0.657542f), glm::vec3(0, 0, -1), 15.0f);
    auto* rightTorp = NEW QuantumTorpedo(_this, map, glm::vec3(0.358887f, 0.023574f, -0.657542f), glm::vec3(0, 0, -1), 15.0f);
    auto* aftTorp = NEW PhotonTorpedo(_this, map, glm::vec3(0.0f, 0.019991f, 1.04274f), glm::vec3(0, 0, 1), 15.0f);

    weapons.addPrimaryWeaponCannon(*leftTopCannon, true);
    weapons.addPrimaryWeaponCannon(*leftBottomCannon, true);
    weapons.addPrimaryWeaponCannon(*rightBottomCannon, true);
    weapons.addPrimaryWeaponCannon(*rightTopCannon, true);

    weapons.addSecondaryWeaponTorpedo(*leftTorp, true);
    weapons.addSecondaryWeaponTorpedo(*rightTorp, true);
    weapons.addSecondaryWeaponTorpedo(*aftTorp);

    //phasers

    vector<glm::vec3> top_beam_pts{
        glm::vec3(-0.095116f, 0.150892f, -0.010849f),
        glm::vec3(-0.050611f, 0.150892f, 0.017642f),
        glm::vec3(0.0f, 0.150892f, 0.026531f),
        glm::vec3(0.050611f, 0.150892f, 0.017642f),
        glm::vec3(0.095116f, 0.150892f, -0.010849f),
    };
    vector<glm::vec3> ventral_left_beam_pts{
        glm::vec3(-0.782141f, -0.087931f, 0.135678f),
        glm::vec3(-0.735904f, -0.113648f, 0.135678f),
        glm::vec3(-0.685293f, -0.129982f, 0.135678f),
        glm::vec3(-0.633076f, -0.144912f, 0.135678f),
        glm::vec3(-0.58733f, -0.157489f, 0.135678f),
    };
    vector<glm::vec3> ventral_right_beam_pts{
        glm::vec3(0.782141f, -0.087931f, 0.135678f),
        glm::vec3(0.735904f, -0.113648f, 0.135678f),
        glm::vec3(0.685293f, -0.129982f, 0.135678f),
        glm::vec3(0.633076f, -0.144912f, 0.135678f),
        glm::vec3(0.58733f, -0.157489f, 0.135678f),
    };
    auto* topBeam = NEW PhaserBeam(_this, map, glm::vec3(0.0f, 0.150892f, 0.026531f), glm::vec3(0.0f, 1.0f, 0.0f), 65.0f, top_beam_pts);
    auto* ventralLeftBeam = NEW PhaserBeam(_this, map, glm::vec3(-0.685293f, -0.129982f, 0.135678f), glm::vec3(-0.056611f, -0.228363f, 0.0f), 70.0f, ventral_left_beam_pts);
    auto* ventralRightBeam = NEW PhaserBeam(_this, map, glm::vec3(0.685293f, -0.129982f, 0.135678f), glm::vec3(0.056611f, -0.228363f, 0.0f), 70.0f, ventral_right_beam_pts);

    weapons.addPrimaryWeaponBeam(*topBeam);
    weapons.addPrimaryWeaponBeam(*ventralRightBeam);
    weapons.addPrimaryWeaponBeam(*ventralLeftBeam);
    


    //bonus weapons only (perk 1)
    {
        vector<glm::vec3> _1_pts{
            glm::vec3(-0.573355f, 0.072793f, -0.207088f),
        };
        vector<glm::vec3> _2_pts{
            glm::vec3(-0.434018f, -0.163775, -0.093399),
        };
        vector<glm::vec3> _3_pts{
            glm::vec3(0.434018f, -0.163775, -0.093399),
        };
        vector<glm::vec3> _4_pts{
            glm::vec3(0.573355f, 0.072793f, -0.207088f),
        };
        auto* leftTopBeamPerk = NEW PhaserBeam(_this, map, _1_pts[0], glm::vec3(0.0055f, 0, -1), 22.0f, _1_pts);
        auto* leftBottomBeamPerk = NEW PhaserBeam(_this, map, _2_pts[0], glm::vec3(0.005f, 0, -1), 22.0f, _2_pts);
        auto* rightBottomBeamPerk = NEW PhaserBeam(_this, map, _3_pts[0], glm::vec3(-0.005f, 0, -1), 22.0f, _3_pts);
        auto* rightTopBeamPerk = NEW PhaserBeam(_this, map, _4_pts[0], glm::vec3(-0.0055f, 0, -1), 22.0f, _4_pts);

        weapons.addPrimaryWeaponBeam(*leftTopBeamPerk, true);
        weapons.addPrimaryWeaponBeam(*leftBottomBeamPerk, true);
        weapons.addPrimaryWeaponBeam(*rightBottomBeamPerk, true);
        weapons.addPrimaryWeaponBeam(*rightTopBeamPerk, true);
    }

    if (Ships::Database[CLASS].HullImpactPoints.size() == 0) {
        Ships::Database[CLASS].HullImpactPoints = {
            glm::vec3(0.001f,0.001f,-0.944632f),
            glm::vec3(0.001f, 0.024758f, -0.671542f),
            glm::vec3(-0.237229f, 0.024758f, -0.444627f),
            glm::vec3(0.237229f, 0.024758f, -0.444627f),
            glm::vec3(-0.260109f, 0.024758f, -0.012753f),
            glm::vec3(0.260109f, 0.024758f, -0.012753f),
            glm::vec3(-0.499616f, -0.018225f, 0.2071f),
            glm::vec3(0.499616f, -0.018225f, 0.2071f),
            glm::vec3(-0.188694f, 0.0429f, 0.404107f),
            glm::vec3(0.188694f, 0.0429f, 0.404107f),
            glm::vec3(-0.188694f, 0.0429f, 0.750788f),
            glm::vec3(0.188694f, 0.0429f, 0.750788f),
            glm::vec3(0.0f, 0.026626f, 0.870882f),
            glm::vec3(0.0f, 0.026626f, 0.443375f),
            glm::vec3(0.0f, 0.026626f, 0.136259f),
            glm::vec3(0.0f, 0.026626f, -0.23),
            glm::vec3(0.0f, 0.026626f, -0.46),
            glm::vec3(-0.118821f,0.026626f, -0.201665f),
            glm::vec3(0.118821f,0.026626f, -0.201665f),
            glm::vec3(-0.714321f,-0.010267f,-0.013934f),
            glm::vec3(0.714321f,-0.010267f,-0.013934f),
            glm::vec3(-0.697608f,-0.010267f,0.401503f),
            glm::vec3(0.697608f,-0.010267f,0.401503f),
            glm::vec3(-0.571067f,-0.010267f,0.279737f),
            glm::vec3(0.571067f,-0.010267f,0.279737f),
            glm::vec3(-0.633144f,-0.010267f,0.055305f),
            glm::vec3(0.633144f,-0.010267f,0.055305f),
            glm::vec3(-0.458851f,-0.010267f,-0.076011f),
            glm::vec3(0.458851f,-0.010267f,-0.076011f),
            glm::vec3(-0.110266f,0.070924f,0.203335f),
            glm::vec3(0.110266f,0.070924f,0.203335f),
            glm::vec3(-0.09021f,0.070924f,0.498439f),
            glm::vec3(0.09021f,0.070924f,0.498439f),
            glm::vec3(-0.164702f,0.030813f,0.87663f),
            glm::vec3(0.164702f,0.030813f,0.87663f),
        };
    }

    m_AI->installFireAtWill(ai_type, _this, map, *static_cast<ShipSystemSensors*>(m_ShipSystems[ShipSystemType::Sensors]), weapons);
    m_AI->installThreatTable(map);
}
Defiant::~Defiant() {

}
void Defiant::fireBeams(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship) {
    vector<ShipSystemWeapons::WeaponBeam> beams;
    auto& weapon_beams = weapons.getBeams();
    if (m_Perks & Defiant::Perks::FrequencyModulators) {
        beams = weapon_beams;
    }else{
        for (int i = 0; i < weapon_beams.size() - 4; ++i) { //omit the 4 perk beams
            beams.push_back(weapon_beams[i]);
        }
    }
    weapons.fireBeamWeapons(target, target_as_ship, beams);
}
void Defiant::fireCannons(ShipSystemWeapons& weapons, EntityWrapper* target, Ship* target_as_ship) {
    if (m_Perks & Defiant::Perks::FrequencyModulators) {
        return; //perk is active, cannons are now beams, so we cannot fire cannons
    }
    weapons.fireCannonWeapons(target, target_as_ship, weapons.getCannons());
}
bool Defiant::cloak(const bool sendPacket) {
    if (m_UnlockedPerks & Defiant::Perks::CloakingDevice) {
        return Ship::cloak(sendPacket);
    }
    return false;
}
bool Defiant::decloak(const bool sendPacket) {
    //TODO: do we need a check here? ideally we WANT it to decloak if the perk is not active, because it shouldn't have been cloaked in the first place! :P
    return Ship::decloak(sendPacket);
}
void Defiant::togglePerk1CannonsToBeams() {
    if (m_UnlockedPerks & Defiant::Perks::FrequencyModulators) {
        auto& weapons = *static_cast<ShipSystemWeapons*>(getShipSystem(ShipSystemType::Weapons));

        if (m_Perks & Defiant::Perks::FrequencyModulators) {
            m_Perks &= ~(Defiant::Perks::FrequencyModulators); //turn beams off, enable cannons

            for (auto& cannon : weapons.getCannons()) {
                cannon.cannon->numRounds = 0;
                cannon.cannon->rechargeTimer = 0.0f;
            }
        }else {
            m_Perks |= Defiant::Perks::FrequencyModulators; //turn beams on, disable cannons

            auto& beams = weapons.getBeams();
            for (size_t i = beams.size() - 4; i < beams.size(); ++i) {
                auto& beam = *beams[i].beam;
                beam.rechargeTimer = 0.0f;
                beam.numRounds = 0;
                beam.chargeTimer = 0.0f;
                beam.state = BeamWeaponState::JustTurnedOff;
            }
        }
    }
}
void Defiant::update(const double& dt) {
    if (Keybinds::isPressedDownOnce(KeybindEnum::TogglePerk1)) {
        if (IsPlayer() && !isDestroyed() && !isCloaked()) {
            togglePerk1CannonsToBeams();
        }
    }
    Ship::update(dt);
}
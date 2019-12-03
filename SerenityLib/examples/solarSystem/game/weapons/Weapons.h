#pragma once
#ifndef GAME_WEAPONS_H
#define GAME_WEAPONS_H

#include "WeaponIncludes.h"
#include "../Ship.h"
#include "../Helper.h"
#include "../ResourceManifest.h"
#include "../map/Map.h"
#include <core/engine/math/Engine_Math.h>
#include <core/engine/sounds/Engine_Sounds.h>

#include "cannons/DisruptorCannon.h"
#include "cannons/PlasmaCannon.h"
#include "cannons/PulsePhaser.h"

#include "beams/PhaserBeam.h"
#include "beams/PlasmaBeam.h"
#include "beams/BorgBeam.h"
#include "beams/BorgCuttingBeam.h"

#include "torpedos/PhotonTorpedo.h"
#include "torpedos/PhotonTorpedoOld.h"
#include "torpedos/QuantumTorpedo.h"
#include "torpedos/KlingonPhotonTorpedo.h"
#include "torpedos/PlasmaTorpedo.h"
#include "torpedos/BorgTorpedo.h"

#include <glm/gtc/matrix_transform.hpp>

using namespace Engine;


class Weapons {
    public:
        static void spawnProjectile(EntityWrapper* target, ShipWeapon& weapon, Ship& ship, Map& map, const glm_vec3& localPosition, const glm_vec3& forward, const int projectile_index, glm_vec3& chosen_target_pos, const unsigned int modelIndex) {
            auto& shipBody = *ship.getComponent<ComponentBody>();
            auto& shipModel = *ship.getComponent<ComponentModel>();
            glm_vec3 final_world_position;
            
            const auto attenuation = 0.28f;

            if (modelIndex > 0) {
                auto& model = shipModel.getModel(modelIndex);
                auto modelPos = glm_vec3(model.position());
                auto modelRot = model.orientation();
                auto part1 = Math::rotate_vec3(modelRot, localPosition);
                glm_vec3 localPos = modelPos + part1;
                auto shipMatrix = shipBody.modelMatrix();
                shipMatrix = glm::translate(shipMatrix, localPos);
                final_world_position = glm_vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
            }else{
                auto shipMatrix = shipBody.modelMatrix();
                shipMatrix = glm::translate(shipMatrix, localPosition);
                final_world_position = glm_vec3(shipMatrix[3][0], shipMatrix[3][1], shipMatrix[3][2]);
            }
            if(target)
                chosen_target_pos = target->getComponent<ComponentBody>()->position() + chosen_target_pos;

            switch (weapon.type) {
                case WeaponType::BorgTorpedo: {
                    auto& w = static_cast<BorgTorpedo&>(weapon);
                    auto* projectile = new BorgTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundBorgTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::BorgBeam: {
                    break;
                }
                case WeaponType::BorgCuttingBeam: {
                    break;
                }
                case WeaponType::DisruptorBeam: {
                    break;
                }
                case WeaponType::DisruptorCannon: {
                    auto& w = static_cast<DisruptorCannon&>(weapon);
                    auto* projectile = new DisruptorCannonProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addCannonProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundDisruptorCannon);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::KlingonPhotonTorpedo: {
                    auto& w = static_cast<KlingonPhotonTorpedo&>(weapon);
                    auto* projectile = new KlingonPhotonTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundKlingonTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PhaserBeam: {
                    break;
                }
                case WeaponType::PhotonTorpedo: {
                    auto& w = static_cast<PhotonTorpedo&>(weapon);
                    auto* projectile = new PhotonTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPhotonTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PhotonTorpedoOld: {
                    auto& w = static_cast<PhotonTorpedoOld&>(weapon);
                    auto* projectile = new PhotonTorpedoOldProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPhotonTorpedoOld);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PlasmaBeam: {
                    break;
                }
                case WeaponType::PlasmaCannon: {
                    auto& w = static_cast<PlasmaCannon&>(weapon);
                    auto* projectile = new PlasmaCannonProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addCannonProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaCannon);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PlasmaTorpedo: {
                    auto& w = static_cast<PlasmaTorpedo&>(weapon);
                    auto* projectile = new PlasmaTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPlasmaTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::PulsePhaser: {
                    auto& w = static_cast<PulsePhaser&>(weapon);
                    auto* projectile = new PulsePhaserProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addCannonProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundPulsePhaser);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                case WeaponType::QuantumTorpedo: {
                    auto& w = static_cast<QuantumTorpedo&>(weapon);
                    auto* projectile = new QuantumTorpedoProjectile(target, w, map, final_world_position, forward, projectile_index, chosen_target_pos);
                    const auto res = map.addTorpedoProjectile(projectile, projectile_index);
                    if (res >= 0) {
                        w.soundEffect = Engine::Sound::playEffect(ResourceManifest::SoundQuantumTorpedo);
                        if (w.soundEffect) {
                            w.soundEffect->setVolume(w.volume);
                            w.soundEffect->setPosition(final_world_position);
                            w.soundEffect->setAttenuation(attenuation);
                        }
                        --weapon.numRounds;
                    }
                    break;
                }
                default: {
                    break;
                }

            }
        }
};

#endif

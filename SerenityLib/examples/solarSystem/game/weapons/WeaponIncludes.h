#pragma once
#ifndef GAME_WEAPON_INCLUDES_H
#define GAME_WEAPON_INCLUDES_H

struct WeaponType final { enum Type {
    DisruptorCannon,
    PlasmaCannon,
    PulsePhaser,

    PhaserBeam,
    PlasmaBeam,
    DisruptorBeam,
    CuttingBeam,

    PhotonTorpedo,
    PhotonTorpedoOld,
    QuantumTorpedo,
    KlingonPhotonTorpedo,
    PlasmaTorpedo,
    BorgTorpedo,
};};

#endif
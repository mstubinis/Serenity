#pragma once
#ifndef ENGINE_PHYSICS_INCLUDES_H
#define ENGINE_PHYSICS_INCLUDES_H

#include <limits>
#include <serenity/system/TypeDefs.h>
#include <serenity/system/Macros.h>

class CollisionType {
	public:
		enum Type : uint32_t {
			BOX_SHAPE_PROXYTYPE,
			TRIANGLE_SHAPE_PROXYTYPE,
			TETRAHEDRAL_SHAPE_PROXYTYPE,
			CONVEX_TRIANGLEMESH_SHAPE_PROXYTYPE,
			CONVEX_HULL_SHAPE_PROXYTYPE,
			CONVEX_POINT_CLOUD_SHAPE_PROXYTYPE,
			CUSTOM_POLYHEDRAL_SHAPE_TYPE,
			//implicit convex shapes
			IMPLICIT_CONVEX_SHAPES_START_HERE,
			SPHERE_SHAPE_PROXYTYPE,
			MULTI_SPHERE_SHAPE_PROXYTYPE,
			CAPSULE_SHAPE_PROXYTYPE,
			CONE_SHAPE_PROXYTYPE,
			CONVEX_SHAPE_PROXYTYPE,
			CYLINDER_SHAPE_PROXYTYPE,
			UNIFORM_SCALING_SHAPE_PROXYTYPE,
			MINKOWSKI_SUM_SHAPE_PROXYTYPE,
			MINKOWSKI_DIFFERENCE_SHAPE_PROXYTYPE,
			BOX_2D_SHAPE_PROXYTYPE,
			CONVEX_2D_SHAPE_PROXYTYPE,
			CUSTOM_CONVEX_SHAPE_TYPE,
			//concave shapes
			CONCAVE_SHAPES_START_HERE,
			//keep all the convex shapetype below here, for the check IsConvexShape in broadphase proxy!
			TRIANGLE_MESH_SHAPE_PROXYTYPE,
			SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE,
			///used for demo integration FAST/Swift collision library and Bullet
			FAST_CONCAVE_MESH_PROXYTYPE,
			//terrain
			TERRAIN_SHAPE_PROXYTYPE,
			///Used for GIMPACT Trimesh integration
			GIMPACT_SHAPE_PROXYTYPE,
			///Multimaterial mesh
			MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE,
			EMPTY_SHAPE_PROXYTYPE,
			STATIC_PLANE_PROXYTYPE,
			CUSTOM_CONCAVE_SHAPE_TYPE,
			SDF_SHAPE_PROXYTYPE = CUSTOM_CONCAVE_SHAPE_TYPE,
			CONCAVE_SHAPES_END_HERE,
			COMPOUND_SHAPE_PROXYTYPE,
			SOFTBODY_SHAPE_PROXYTYPE,
			HFFLUID_SHAPE_PROXYTYPE,
			HFFLUID_BUOYANT_CONVEX_SHAPE_PROXYTYPE,
			INVALID_SHAPE_PROXYTYPE,
			MAX_BROADPHASE_COLLISION_TYPES,
		};
		BUILD_ENUM_CLASS_MEMBERS(CollisionType, Type)
};
struct CollisionFlag final { enum Flag : MaskType {
    None                          =      0,
    StaticObject                  = 1 << 0,
    KinematicObject               = 1 << 1,
    NoContactResponse             = 1 << 2, //no collision based forces i think
    CustomMaterialCallback        = 1 << 3, //this allows per-triangle material (friction/restitution)
    CharacterObject               = 1 << 4,
    DisableVisualizeObject        = 1 << 5, //disable debug drawing
    DisableSPUCollisionProcessing = 1 << 6, //disable parallel/SPU processing
    HasContactStiffnessDamping    = 1 << 7,
    HasCustomDebugRenderingColor  = 1 << 8,
    HasFrictionAnchor             = 1 << 9,
    HasCollisionSoundTrigger      = 1 << 10,
    _Custom_1                     = 1 << 11,
    _Custom_2                     = 1 << 12,
    _Custom_3                     = 1 << 13,
    _Custom_4                     = 1 << 14,
    _Custom_5                     = 1 << 15,
    AllFilter                     = -1,
};};
struct CollisionFilter final { enum Filter : MaskType {
    NoFilter        =      0,
    DefaultFilter   = 1 << 0,
    StaticFilter    = 1 << 1,
    KinematicFilter = 1 << 2,
    DebrisFilter    = 1 << 3,
    SensorTrigger   = 1 << 4,
    CharacterFilter = 1 << 5,
    _Custom_1       = 1 << 6,
    _Custom_2       = 1 << 7,
    _Custom_3       = 1 << 8,
    _Custom_4       = 1 << 9,
    _Custom_5       = 1 << 10,
    AllFilter       = -1,
};};

#endif
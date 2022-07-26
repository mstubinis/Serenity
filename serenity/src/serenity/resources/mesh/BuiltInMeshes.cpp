
#include <serenity/resources/mesh/BuiltInMeshes.h>
#include <serenity/resources/mesh/Mesh.h>
#include <serenity/resources/Engine_Resources.h>

Engine::priv::BuiltInMeshses::BuiltInMeshses() {
    //init();
}

Handle Engine::priv::BuiltInMeshses::getPointLightBounds() {
    return m_BuiltInMeshes[BuiltInMeshEnum::PointLight];
}
Handle Engine::priv::BuiltInMeshses::getSpotLightBounds() {
    return m_BuiltInMeshes[BuiltInMeshEnum::SpotLight];
}
Handle Engine::priv::BuiltInMeshses::getRodLightBounds() {
    return m_BuiltInMeshes[BuiltInMeshEnum::RodLight];
}
Handle Engine::priv::BuiltInMeshses::getProjectionLightBounds() {
    return m_BuiltInMeshes[BuiltInMeshEnum::ProjectionLight];
}
Handle Engine::priv::BuiltInMeshses::getTriangleMesh() {
    return m_BuiltInMeshes[BuiltInMeshEnum::Triangle];
}
Handle Engine::priv::BuiltInMeshses::getCubeMesh() {
    return m_BuiltInMeshes[BuiltInMeshEnum::Cube];
}
Handle Engine::priv::BuiltInMeshses::getPlaneMesh() {
    return m_BuiltInMeshes[BuiltInMeshEnum::Plane];
}
Handle Engine::priv::BuiltInMeshses::getPlane2DMesh() {
    return m_BuiltInMeshes[BuiltInMeshEnum::Plane2D];
}
Handle Engine::priv::BuiltInMeshses::getFontMesh() {
    return m_BuiltInMeshes[BuiltInMeshEnum::Font];
}
Handle Engine::priv::BuiltInMeshses::getParticleMesh() {
    return m_BuiltInMeshes[BuiltInMeshEnum::Particle];
}

bool Engine::priv::BuiltInMeshses::init() {
    if (m_BuiltInMeshes[0]) {
        return false;
    }

    if (!build_point_light_mesh())
        return false;
    if (!build_spot_light_mesh())
        return false;
    if (!build_rod_light_mesh())
        return false;
    if (!build_projection_light_mesh())
        return false;
    if (!build_triangle_mesh())
        return false;
    if (!build_cube_mesh())
        return false;
    if (!build_plane_mesh())
        return false;
    if (!build_plane_2d_mesh())
        return false;
    if (!build_font_mesh())
        return false;
    if (!build_particle_mesh())
        return false;

    return true;
}
bool Engine::priv::BuiltInMeshses::build_point_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::PointLight])
        return false;
    std::string pointLightMesh;
    {
        pointLightMesh = R"(
v 0.000000 -1.000000 0.000000
v 0.723607 -0.447220 0.525725
v -0.276388 -0.447220 0.850649
v -0.894426 -0.447216 0.000000
v -0.276388 -0.447220 -0.850649
v 0.723607 -0.447220 -0.525725
v 0.276388 0.447220 0.850649
v -0.723607 0.447220 0.525725
v -0.723607 0.447220 -0.525725
v 0.276388 0.447220 -0.850649
v 0.894426 0.447216 0.000000
v 0.000000 1.000000 0.000000
v -0.232822 -0.657519 0.716563
v -0.162456 -0.850654 0.499995
v -0.077607 -0.967950 0.238853
v 0.203181 -0.967950 0.147618
v 0.425323 -0.850654 0.309011
v 0.609547 -0.657519 0.442856
v 0.531941 -0.502302 0.681712
v 0.262869 -0.525738 0.809012
v -0.029639 -0.502302 0.864184
v 0.812729 -0.502301 -0.295238
v 0.850648 -0.525736 0.000000
v 0.812729 -0.502301 0.295238
v 0.203181 -0.967950 -0.147618
v 0.425323 -0.850654 -0.309011
v 0.609547 -0.657519 -0.442856
v -0.753442 -0.657515 0.000000
v -0.525730 -0.850652 0.000000
v -0.251147 -0.967949 0.000000
v -0.483971 -0.502302 0.716565
v -0.688189 -0.525736 0.499997
v -0.831051 -0.502299 0.238853
v -0.232822 -0.657519 -0.716563
v -0.162456 -0.850654 -0.499995
v -0.077607 -0.967950 -0.238853
v -0.831051 -0.502299 -0.238853
v -0.688189 -0.525736 -0.499997
v -0.483971 -0.502302 -0.716565
v -0.029639 -0.502302 -0.864184
v 0.262869 -0.525738 -0.809012
v 0.531941 -0.502302 -0.681712
v 0.956626 0.251149 0.147618
v 0.951058 -0.000000 0.309013
v 0.860698 -0.251151 0.442858
v 0.860698 -0.251151 -0.442858
v 0.951058 0.000000 -0.309013
v 0.956626 0.251149 -0.147618
v 0.155215 0.251152 0.955422
v 0.000000 -0.000000 1.000000
v -0.155215 -0.251152 0.955422
v 0.687159 -0.251152 0.681715
v 0.587786 0.000000 0.809017
v 0.436007 0.251152 0.864188
v -0.860698 0.251151 0.442858
v -0.951058 -0.000000 0.309013
v -0.956626 -0.251149 0.147618
v -0.436007 -0.251152 0.864188
v -0.587786 0.000000 0.809017
v -0.687159 0.251152 0.681715
v -0.687159 0.251152 -0.681715
v -0.587786 -0.000000 -0.809017
v -0.436007 -0.251152 -0.864188
v -0.956626 -0.251149 -0.147618
v -0.951058 0.000000 -0.309013
v -0.860698 0.251151 -0.442858
v 0.436007 0.251152 -0.864188
v 0.587786 -0.000000 -0.809017
v 0.687159 -0.251152 -0.681715
v -0.155215 -0.251152 -0.955422
v 0.000000 0.000000 -1.000000
v 0.155215 0.251152 -0.955422
v 0.831051 0.502299 0.238853
v 0.688189 0.525736 0.499997
v 0.483971 0.502302 0.716565
v 0.029639 0.502302 0.864184
v -0.262869 0.525738 0.809012
v -0.531941 0.502302 0.681712
v -0.812729 0.502301 0.295238
v -0.850648 0.525736 0.000000
v -0.812729 0.502301 -0.295238
v -0.531941 0.502302 -0.681712
v -0.262869 0.525738 -0.809012
v 0.029639 0.502302 -0.864184
v 0.483971 0.502302 -0.716565
v 0.688189 0.525736 -0.499997
v 0.831051 0.502299 -0.238853
v 0.077607 0.967950 0.238853
v 0.162456 0.850654 0.499995
v 0.232822 0.657519 0.716563
v 0.753442 0.657515 0.000000
v 0.525730 0.850652 0.000000
v 0.251147 0.967949 0.000000
v -0.203181 0.967950 0.147618
v -0.425323 0.850654 0.309011
v -0.609547 0.657519 0.442856
v -0.203181 0.967950 -0.147618
v -0.425323 0.850654 -0.309011
v -0.609547 0.657519 -0.442856
v 0.077607 0.967950 -0.238853
v 0.162456 0.850654 -0.499995
v 0.232822 0.657519 -0.716563
v 0.361800 0.894429 -0.262863
v 0.638194 0.723610 -0.262864
v 0.447209 0.723612 -0.525728
v -0.138197 0.894430 -0.425319
v -0.052790 0.723612 -0.688185
v -0.361804 0.723612 -0.587778
v -0.447210 0.894429 0.000000
v -0.670817 0.723611 -0.162457
v -0.670817 0.723611 0.162457
v -0.138197 0.894430 0.425319
v -0.361804 0.723612 0.587778
v -0.052790 0.723612 0.688185
v 0.361800 0.894429 0.262863
v 0.447209 0.723612 0.525728
v 0.638194 0.723610 0.262864
v 0.861804 0.276396 -0.425322
v 0.809019 0.000000 -0.587782
v 0.670821 0.276397 -0.688189
v -0.138199 0.276397 -0.951055
v -0.309016 -0.000000 -0.951057
v -0.447215 0.276397 -0.850649
v -0.947213 0.276396 -0.162458
v -1.000000 0.000001 0.000000
v -0.947213 0.276397 0.162458
v -0.447216 0.276397 0.850648
v -0.309017 -0.000001 0.951056
v -0.138199 0.276397 0.951055
v 0.670820 0.276396 0.688190
v 0.809019 -0.000002 0.587783
v 0.861804 0.276394 0.425323
v 0.309017 -0.000000 -0.951056
v 0.447216 -0.276398 -0.850648
v 0.138199 -0.276398 -0.951055
v -0.809018 -0.000000 -0.587783
v -0.670819 -0.276397 -0.688191
v -0.861803 -0.276396 -0.425324
v -0.809018 0.000000 0.587783
v -0.861803 -0.276396 0.425324
v -0.670819 -0.276397 0.688191
v 0.309017 0.000000 0.951056
v 0.138199 -0.276398 0.951055
v 0.447216 -0.276398 0.850648
v 1.000000 0.000000 0.000000
v 0.947213 -0.276396 0.162458
v 0.947213 -0.276396 -0.162458
v 0.361803 -0.723612 -0.587779
v 0.138197 -0.894429 -0.425321
v 0.052789 -0.723611 -0.688186
v -0.447211 -0.723612 -0.525727
v -0.361801 -0.894429 -0.262863
v -0.638195 -0.723609 -0.262863
v -0.638195 -0.723609 0.262864
v -0.361801 -0.894428 0.262864
v -0.447211 -0.723610 0.525729
v 0.670817 -0.723611 -0.162457
v 0.670818 -0.723610 0.162458
v 0.447211 -0.894428 0.000001
v 0.052790 -0.723612 0.688185
v 0.138199 -0.894429 0.425321
v 0.361805 -0.723611 0.587779
f 1 16 15
f 2 18 24
f 1 15 30
f 1 30 36
f 1 36 25
f 2 24 45
f 3 21 51
f 4 33 57
f 5 39 63
f 6 42 69
f 2 45 52
f 3 51 58
f 4 57 64
f 5 63 70
f 6 69 46
f 7 75 90
f 8 78 96
f 9 81 99
f 10 84 102
f 11 87 91
f 93 100 12
f 92 103 93
f 91 104 92
f 93 103 100
f 103 101 100
f 92 104 103
f 104 105 103
f 103 105 101
f 105 102 101
f 91 87 104
f 87 86 104
f 104 86 105
f 86 85 105
f 105 85 102
f 85 10 102
f 100 97 12
f 101 106 100
f 102 107 101
f 100 106 97
f 106 98 97
f 101 107 106
f 107 108 106
f 106 108 98
f 108 99 98
f 102 84 107
f 84 83 107
f 107 83 108
f 83 82 108
f 108 82 99
f 82 9 99
f 97 94 12
f 98 109 97
f 99 110 98
f 97 109 94
f 109 95 94
f 98 110 109
f 110 111 109
f 109 111 95
f 111 96 95
f 99 81 110
f 81 80 110
f 110 80 111
f 80 79 111
f 111 79 96
f 79 8 96
f 94 88 12
f 95 112 94
f 96 113 95
f 94 112 88
f 112 89 88
f 95 113 112
f 113 114 112
f 112 114 89
f 114 90 89
f 96 78 113
f 78 77 113
f 113 77 114
f 77 76 114
f 114 76 90
f 76 7 90
f 88 93 12
f 89 115 88
f 90 116 89
f 88 115 93
f 115 92 93
f 89 116 115
f 116 117 115
f 115 117 92
f 117 91 92
f 90 75 116
f 75 74 116
f 116 74 117
f 74 73 117
f 117 73 91
f 73 11 91
f 48 87 11
f 47 118 48
f 46 119 47
f 48 118 87
f 118 86 87
f 47 119 118
f 119 120 118
f 118 120 86
f 120 85 86
f 46 69 119
f 69 68 119
f 119 68 120
f 68 67 120
f 120 67 85
f 67 10 85
f 72 84 10
f 71 121 72
f 70 122 71
f 72 121 84
f 121 83 84
f 71 122 121
f 122 123 121
f 121 123 83
f 123 82 83
f 70 63 122
f 63 62 122
f 122 62 123
f 62 61 123
f 123 61 82
f 61 9 82
f 66 81 9
f 65 124 66
f 64 125 65
f 66 124 81
f 124 80 81
f 65 125 124
f 125 126 124
f 124 126 80
f 126 79 80
f 64 57 125
f 57 56 125
f 125 56 126
f 56 55 126
f 126 55 79
f 55 8 79
f 60 78 8
f 59 127 60
f 58 128 59
f 60 127 78
f 127 77 78
f 59 128 127
f 128 129 127
f 127 129 77
f 129 76 77
f 58 51 128
f 51 50 128
f 128 50 129
f 50 49 129
f 129 49 76
f 49 7 76
f 54 75 7
f 53 130 54
f 52 131 53
f 54 130 75
f 130 74 75
f 53 131 130
f 131 132 130
f 130 132 74
f 132 73 74
f 52 45 131
f 45 44 131
f 131 44 132
f 44 43 132
f 132 43 73
f 43 11 73
f 67 72 10
f 68 133 67
f 69 134 68
f 67 133 72
f 133 71 72
f 68 134 133
f 134 135 133
f 133 135 71
f 135 70 71
f 69 42 134
f 42 41 134
f 134 41 135
f 41 40 135
f 135 40 70
f 40 5 70
f 61 66 9
f 62 136 61
f 63 137 62
f 61 136 66
f 136 65 66
f 62 137 136
f 137 138 136
f 136 138 65
f 138 64 65
f 63 39 137
f 39 38 137
f 137 38 138
f 38 37 138
f 138 37 64
f 37 4 64
f 55 60 8
f 56 139 55
f 57 140 56
f 55 139 60
f 139 59 60
f 56 140 139
f 140 141 139
f 139 141 59
f 141 58 59
f 57 33 140
f 33 32 140
f 140 32 141
f 32 31 141
f 141 31 58
f 31 3 58
f 49 54 7
f 50 142 49
f 51 143 50
f 49 142 54
f 142 53 54
f 50 143 142
f 143 144 142
f 142 144 53
f 144 52 53
f 51 21 143
f 21 20 143
f 143 20 144
f 20 19 144
f 144 19 52
f 19 2 52
f 43 48 11
f 44 145 43
f 45 146 44
f 43 145 48
f 145 47 48
f 44 146 145
f 146 147 145
f 145 147 47
f 147 46 47
f 45 24 146
f 24 23 146
f 146 23 147
f 23 22 147
f 147 22 46
f 22 6 46
f 27 42 6
f 26 148 27
f 25 149 26
f 27 148 42
f 148 41 42
f 26 149 148
f 149 150 148
f 148 150 41
f 150 40 41
f 25 36 149
f 36 35 149
f 149 35 150
f 35 34 150
f 150 34 40
f 34 5 40
f 34 39 5
f 35 151 34
f 36 152 35
f 34 151 39
f 151 38 39
f 35 152 151
f 152 153 151
f 151 153 38
f 153 37 38
f 36 30 152
f 30 29 152
f 152 29 153
f 29 28 153
f 153 28 37
f 28 4 37
f 28 33 4
f 29 154 28
f 30 155 29
f 28 154 33
f 154 32 33
f 29 155 154
f 155 156 154
f 154 156 32
f 156 31 32
f 30 15 155
f 15 14 155
f 155 14 156
f 14 13 156
f 156 13 31
f 13 3 31
f 22 27 6
f 23 157 22
f 24 158 23
f 22 157 27
f 157 26 27
f 23 158 157
f 158 159 157
f 157 159 26
f 159 25 26
f 24 18 158
f 18 17 158
f 158 17 159
f 17 16 159
f 159 16 25
f 16 1 25
f 13 21 3
f 14 160 13
f 15 161 14
f 13 160 21
f 160 20 21
f 14 161 160
f 161 162 160
f 160 162 20
f 162 19 20
f 15 16 161
f 16 17 161
f 161 17 162
f 17 18 162
f 162 18 19
f 18 2 19
)";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::PointLight] = Engine::Resources::addResource<Mesh>(pointLightMesh, 0.0005f);
    m_BuiltInMeshes[BuiltInMeshEnum::PointLight].get<Mesh>()->setName("MeshPointLight");
    return true;
}
bool Engine::priv::BuiltInMeshses::build_spot_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::SpotLight])
        return false;
    std::string spotLightData;
    {
        spotLightData = R"(
v 0.000000 -1.000000 0.000000
v -0.894426 -0.447216 0.000000
v -0.276388 -0.447220 -0.850649
v 0.723607 -0.447220 -0.525725
v -0.723607 0.447220 -0.525725
v 0.276388 0.447220 -0.850649
v 0.894426 0.447216 0.000000
v 0.000000 1.000000 0.000000
v 0.812729 -0.502301 -0.295238
v 0.850648 -0.525736 0.000000
v 0.203181 -0.967950 -0.147618
v 0.425323 -0.850654 -0.309011
v 0.609547 -0.657519 -0.442856
v -0.753442 -0.657515 0.000000
v -0.525730 -0.850652 0.000000
v -0.251147 -0.967949 0.000000
v -0.232822 -0.657519 -0.716563
v -0.162456 -0.850654 -0.499995
v -0.077607 -0.967950 -0.238853
v -0.831051 -0.502299 -0.238853
v -0.688189 -0.525736 -0.499997
v -0.483971 -0.502302 -0.716565
v -0.029639 -0.502302 -0.864184
v 0.262869 -0.525738 -0.809012
v 0.531941 -0.502302 -0.681712
v 0.860698 -0.251151 -0.442858
v 0.951058 0.000000 -0.309013
v 0.956626 0.251149 -0.147618
v -0.687159 0.251152 -0.681715
v -0.587786 -0.000000 -0.809017
v -0.436007 -0.251152 -0.864188
v -0.956626 -0.251149 -0.147618
v -0.951058 0.000000 -0.309013
v -0.860698 0.251151 -0.442858
v 0.436007 0.251152 -0.864188
v 0.587786 -0.000000 -0.809017
v 0.687159 -0.251152 -0.681715
v -0.155215 -0.251152 -0.955422
v 0.000000 0.000000 -1.000000
v 0.155215 0.251152 -0.955422
v -0.850648 0.525736 0.000000
v -0.812729 0.502301 -0.295238
v -0.531941 0.502302 -0.681712
v -0.262869 0.525738 -0.809012
v 0.029639 0.502302 -0.864184
v 0.483971 0.502302 -0.716565
v 0.688189 0.525736 -0.499997
v 0.831051 0.502299 -0.238853
v 0.753442 0.657515 0.000000
v 0.525730 0.850652 0.000000
v 0.251147 0.967949 0.000000
v -0.203181 0.967950 -0.147618
v -0.425323 0.850654 -0.309011
v -0.609547 0.657519 -0.442856
v 0.077607 0.967950 -0.238853
v 0.162456 0.850654 -0.499995
v 0.232822 0.657519 -0.716563
v 0.361800 0.894429 -0.262863
v 0.638194 0.723610 -0.262864
v 0.447209 0.723612 -0.525728
v -0.138197 0.894430 -0.425319
v -0.052790 0.723612 -0.688185
v -0.361804 0.723612 -0.587778
v -0.447210 0.894429 0.000000
v -0.670817 0.723611 -0.162457
v 0.861804 0.276396 -0.425322
v 0.809019 0.000000 -0.587782
v 0.670821 0.276397 -0.688189
v -0.138199 0.276397 -0.951055
v -0.309016 -0.000000 -0.951057
v -0.447215 0.276397 -0.850649
v -0.947213 0.276396 -0.162458
v -1.000000 0.000001 0.000000
v 0.309017 -0.000000 -0.951056
v 0.447216 -0.276398 -0.850648
v 0.138199 -0.276398 -0.951055
v -0.809018 -0.000000 -0.587783
v -0.670819 -0.276397 -0.688191
v -0.861803 -0.276396 -0.425324
v 1.000000 0.000000 0.000000
v 0.947213 -0.276396 -0.162458
v 0.361803 -0.723612 -0.587779
v 0.138197 -0.894429 -0.425321
v 0.052789 -0.723611 -0.688186
v -0.447211 -0.723612 -0.525727
v -0.361801 -0.894429 -0.262863
v -0.638195 -0.723609 -0.262863
v 0.670817 -0.723611 -0.162457
v 0.447211 -0.894428 0.000001
f 1 16 19
f 1 19 11
f 3 22 31
f 4 25 37
f 3 31 38
f 4 37 26
f 5 42 54
f 6 45 57
f 7 48 49
f 51 55 8
f 50 58 51
f 49 59 50
f 51 58 55
f 58 56 55
f 50 59 58
f 59 60 58
f 58 60 56
f 60 57 56
f 49 48 59
f 48 47 59
f 59 47 60
f 47 46 60
f 60 46 57
f 46 6 57
f 55 52 8
f 56 61 55
f 57 62 56
f 55 61 52
f 61 53 52
f 56 62 61
f 62 63 61
f 61 63 53
f 63 54 53
f 57 45 62
f 45 44 62
f 62 44 63
f 44 43 63
f 63 43 54
f 43 5 54
f 53 64 52
f 54 65 53
f 53 65 64
f 54 42 65
f 42 41 65
f 28 48 7
f 27 66 28
f 26 67 27
f 28 66 48
f 66 47 48
f 27 67 66
f 67 68 66
f 66 68 47
f 68 46 47
f 26 37 67
f 37 36 67
f 67 36 68
f 36 35 68
f 68 35 46
f 35 6 46
f 40 45 6
f 39 69 40
f 38 70 39
f 40 69 45
f 69 44 45
f 39 70 69
f 70 71 69
f 69 71 44
f 71 43 44
f 38 31 70
f 31 30 70
f 70 30 71
f 30 29 71
f 71 29 43
f 29 5 43
f 34 42 5
f 33 72 34
f 32 73 33
f 34 72 42
f 72 41 42
f 33 73 72
f 35 40 6
f 36 74 35
f 37 75 36
f 35 74 40
f 74 39 40
f 36 75 74
f 75 76 74
f 74 76 39
f 76 38 39
f 37 25 75
f 25 24 75
f 75 24 76
f 24 23 76
f 76 23 38
f 23 3 38
f 29 34 5
f 30 77 29
f 31 78 30
f 29 77 34
f 77 33 34
f 30 78 77
f 78 79 77
f 77 79 33
f 79 32 33
f 31 22 78
f 22 21 78
f 78 21 79
f 21 20 79
f 79 20 32
f 20 2 32
f 80 27 28
f 80 81 27
f 81 26 27
f 10 9 81
f 81 9 26
f 9 4 26
f 13 25 4
f 12 82 13
f 11 83 12
f 13 82 25
f 82 24 25
f 12 83 82
f 83 84 82
f 82 84 24
f 84 23 24
f 11 19 83
f 19 18 83
f 83 18 84
f 18 17 84
f 84 17 23
f 17 3 23
f 17 22 3
f 18 85 17
f 19 86 18
f 17 85 22
f 85 21 22
f 18 86 85
f 86 87 85
f 85 87 21
f 87 20 21
f 19 16 86
f 16 15 86
f 86 15 87
f 15 14 87
f 87 14 20
f 14 2 20
f 9 13 4
f 10 88 9
f 9 88 13
f 88 12 13
f 88 89 12
f 89 11 12
f 65 41 64
f 41 72 73
f 32 2 73
f 11 89 1
f 88 10 89
f 81 80 10
f 7 80 28
f 52 64 8
f 14 41 2
f 41 73 2
f 16 64 15
f 64 41 15
f 16 8 64
f 15 41 14
f 8 1 51
f 1 89 51
f 51 89 50
f 50 89 49
f 89 10 49
f 49 10 7
f 80 7 10
f 16 1 8
)";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::SpotLight] = Engine::Resources::addResource<Mesh>(spotLightData, 0.0005f);
    m_BuiltInMeshes[BuiltInMeshEnum::SpotLight].get<Mesh>()->setName("MeshSpotLight");
    return true;
}
bool Engine::priv::BuiltInMeshses::build_rod_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::RodLight])
        return false;
    std::string rodLightData;
    {
        rodLightData = R"(
v -0.000000 1.000000 -1.000000
v -0.000000 1.000000 1.000000
v 0.284630 0.959493 -1.000000
v 0.284630 0.959493 1.000000
v 0.546200 0.841254 -1.000000
v 0.546200 0.841254 1.000000
v 0.763521 0.654861 -1.000000
v 0.763521 0.654861 1.000000
v 0.918986 0.415415 -1.000000
v 0.918986 0.415415 1.000000
v 1.000000 0.142315 -1.000000
v 1.000000 0.142315 1.000000
v 1.000000 -0.142315 -1.000000
v 1.000000 -0.142315 1.000000
v 0.918986 -0.415415 -1.000000
v 0.918986 -0.415415 1.000000
v 0.763521 -0.654860 -1.000000
v 0.763521 -0.654861 1.000000
v 0.546200 -0.841253 -1.000000
v 0.546200 -0.841253 1.000000
v 0.284630 -0.959493 -1.000000
v 0.284630 -0.959493 1.000000
v 0.000000 -1.000000 -1.000000
v 0.000000 -1.000000 1.000000
v -0.284629 -0.959493 -1.000000
v -0.284629 -0.959493 1.000000
v -0.546200 -0.841253 -1.000000
v -0.546200 -0.841254 1.000000
v -0.763521 -0.654861 -1.000000
v -0.763521 -0.654861 1.000000
v -0.918986 -0.415415 -1.000000
v -0.918986 -0.415415 1.000000
v -1.000000 -0.142315 -1.000000
v -1.000000 -0.142315 1.000000
v -1.000000 0.142314 -1.000000
v -1.000000 0.142314 1.000000
v -0.918986 0.415415 -1.000000
v -0.918986 0.415414 1.000000
v -0.763522 0.654860 -1.000000
v -0.763522 0.654860 1.000000
v -0.546201 0.841253 -1.000000
v -0.546201 0.841253 1.000000
v -0.284631 0.959493 -1.000000
v -0.284631 0.959493 1.000000
f 2 3 1
f 4 5 3
f 6 7 5
f 8 9 7
f 10 11 9
f 12 13 11
f 14 15 13
f 16 17 15
f 18 19 17
f 20 21 19
f 21 24 23
f 24 25 23
f 26 27 25
f 28 29 27
f 29 32 31
f 32 33 31
f 34 35 33
f 35 38 37
f 38 39 37
f 39 42 41
f 25 1 23
f 42 43 41
f 44 1 43
f 3 23 1
f 27 43 25
f 29 41 27
f 31 39 29
f 33 37 31
f 5 21 3
f 7 19 5
f 9 17 7
f 11 15 9
f 2 22 4
f 24 44 26
f 26 42 28
f 28 40 30
f 30 38 32
f 32 36 34
f 6 22 20
f 8 20 18
f 8 16 10
f 12 16 14
f 2 4 3
f 4 6 5
f 6 8 7
f 8 10 9
f 10 12 11
f 12 14 13
f 14 16 15
f 16 18 17
f 18 20 19
f 20 22 21
f 21 22 24
f 24 26 25
f 26 28 27
f 28 30 29
f 29 30 32
f 32 34 33
f 34 36 35
f 35 36 38
f 38 40 39
f 39 40 42
f 25 43 1
f 42 44 43
f 44 2 1
f 3 21 23
f 27 41 43
f 29 39 41
f 31 37 39
f 33 35 37
f 5 19 21
f 7 17 19
f 9 15 17
f 11 13 15
f 2 24 22
f 24 2 44
f 26 44 42
f 28 42 40
f 30 40 38
f 32 38 36
f 6 4 22
f 8 6 20
f 8 18 16
f 12 10 16
)";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::RodLight] = Engine::Resources::addResource<Mesh>(rodLightData, 0.0005f);
    m_BuiltInMeshes[BuiltInMeshEnum::RodLight].get<Mesh>()->setName("MeshRodLight");
    return true;
}
bool Engine::priv::BuiltInMeshses::build_projection_light_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::ProjectionLight])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::ProjectionLight] = Engine::Resources::addResource<Mesh>("ProjectionLightMesh", 1.0f, 1.0f, 0.0005f, VertexDataFormat::VertexDataNoLighting);
    return true;
}
bool Engine::priv::BuiltInMeshses::build_triangle_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Triangle])
        return false;
    auto& vertexData = *(NEW VertexData{ VertexDataFormat::VertexData2D });
    m_BuiltInMeshes[BuiltInMeshEnum::Triangle] = Engine::Resources::addResource<Mesh>(vertexData, "TriangleMesh", 0.0005f);
    auto& triangleMesh = *m_BuiltInMeshes[BuiltInMeshEnum::Triangle].get<Mesh>();

    auto positions = Engine::create_and_reserve<std::vector<glm::vec3>>(3);
    auto uvs       = Engine::create_and_reserve<std::vector<glm::vec2>>(3);
    auto colors    = Engine::create_and_resize<std::vector<glm::u8vec4>>(3, glm::u8vec4{ 255_uc });
    auto indices   = Engine::create_and_reserve<std::vector<uint32_t>>(3);

    uvs.emplace_back(0.5f, 0.0f);
    uvs.emplace_back(1.0f, 1.0f);
    uvs.emplace_back(0.0f, 1.0f);

    positions.emplace_back(0.0f, -0.948008f, 0.0f);
    positions.emplace_back(1.0f, 0.689086, 0.0f);
    positions.emplace_back(-1.0f, 0.689086f, 0.0f);

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);

    vertexData.setData(0, positions.data(), positions.size(), MeshModifyFlags::None);
    vertexData.setData(1, uvs.data(), uvs.size(), MeshModifyFlags::None);
    if (vertexData.m_Format.getAttributes()[2].type == GL_UNSIGNED_BYTE) {
        vertexData.setData(2, colors.data(), colors.size(), MeshModifyFlags::None);
    }
    vertexData.setIndices(indices.data(), indices.size(), MeshModifyFlags::RecalculateTriangles);

    triangleMesh.m_CPUData.internal_calculate_radius();
    triangleMesh.m_CPUData.m_CollisionFactory = NEW MeshCollisionFactory{ triangleMesh.m_CPUData };

    triangleMesh.load(false);
    triangleMesh.setName("MeshTriangle");
    return true;
}
bool Engine::priv::BuiltInMeshses::build_cube_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Cube])
        return false;
    std::string cubeMesh;
    {
        cubeMesh = R"(
v 1.0 -1.0 -1.0
v 1.0 -1.0 1.0
v -1.0 -1.0 1.0
v -1.0 -1.0 -1.0
v 1.0 1.0 -1.0
v 1.0 1.0 1.0
v -1.0 1.0 1.0
v -1.0 1.0 -1.0
vt 1.0 0.0
vt 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0
vt 0.0 1.0
vt 0.0 0.0
vt 1.0 0.0
vt 0.0 1.0
vt 1.0 0.0
vt 0.0 1.0
vt 0.0 0.0
vt 0.0 0.0
vt 1.0 1.0
vt 1.0 0.0
vt 0.0 1.0
vt 1.0 1.0
vt 1.0 1.0
vt 1.0 1.0
vt 1.0 0.0
vt 1.0 1.0
vn 0.0 -1.0 0.0
vn 0.0 1.0 0.0
vn 1.0 0.0 0.0
vn 0.0 0.0 1.0
vn -1.0 0.0 0.0
vn 0.0 0.0 -1.0
f 2/1/1 4/2/1 1/3/1
f 8/4/2 6/5/2 5/6/2
f 5/7/3 2/8/3 1/3/3
f 6/9/4 3/10/4 2/11/4
f 3/12/5 8/13/5 4/2/5
f 1/14/6 8/15/6 5/6/6
f 2/1/1 3/16/1 4/2/1
f 8/4/2 7/17/2 6/5/2
f 5/7/3 6/18/3 2/8/3
f 6/9/4 7/17/4 3/10/4
f 3/12/5 7/19/5 8/13/5
f 1/14/6 4/20/6 8/15/6
)";
    }

    m_BuiltInMeshes[BuiltInMeshEnum::Cube] = Engine::Resources::addResource<Mesh>(cubeMesh, 0.0005f);
    m_BuiltInMeshes[BuiltInMeshEnum::Cube].get<Mesh>()->setName("MeshCube");
    return true;
}
bool Engine::priv::BuiltInMeshses::build_plane_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Plane])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Plane] = Engine::Resources::addResource<Mesh>("Plane", 1.0f, 1.0f, 0.0005f, VertexDataFormat::VertexDataNoLighting);
    return true;
}
bool Engine::priv::BuiltInMeshses::build_plane_2d_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Plane2D])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Plane2D] = Engine::Resources::addResource<Mesh>("Plane", 1.0f, 1.0f, 0.0005f, VertexDataFormat::VertexData2D);
    return true;
}
bool Engine::priv::BuiltInMeshses::build_font_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Font])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Font] = Engine::Resources::addResource<Mesh>("FontPlane", 1.0f, 1.0f, 0.0005f, VertexDataFormat::VertexData2D);
    return true;
}
bool Engine::priv::BuiltInMeshses::build_particle_mesh() {
    if (m_BuiltInMeshes[BuiltInMeshEnum::Particle])
        return false;
    m_BuiltInMeshes[BuiltInMeshEnum::Particle] = Engine::Resources::addResource<Mesh>("ParticlePlane", 1.0f, 1.0f, 0.0005f, VertexDataFormat::VertexDataNoLighting);
    return true;
}
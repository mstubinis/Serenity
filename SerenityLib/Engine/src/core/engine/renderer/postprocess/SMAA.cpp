#include <core/engine/renderer/postprocess/SMAA.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/renderer/SMAA_LUT.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/shaders/Shader.h>
#include <core/engine/threading/Engine_ThreadManager.h>

using namespace std;

Engine::priv::SMAA Engine::priv::SMAA::smaa;

Engine::priv::SMAA::SMAA() {
    THRESHOLD                        = 0.05f;
    MAX_SEARCH_STEPS                 = 32;
    MAX_SEARCH_STEPS_DIAG            = 16;
    CORNER_ROUNDING                  = 25;
    LOCAL_CONTRAST_ADAPTATION_FACTOR = 2.0f;
    DEPTH_THRESHOLD                  = (0.1f * THRESHOLD);
    PREDICATION                      = false;
    PREDICATION_THRESHOLD            = 0.01f;
    PREDICATION_SCALE                = 2.0f;
    PREDICATION_STRENGTH             = 0.4f;
    REPROJECTION                     = false;
    REPROJECTION_WEIGHT_SCALE        = 30.0f;
    AREATEX_MAX_DISTANCE             = 16;
    AREATEX_MAX_DISTANCE_DIAG        = 20;
    AREATEX_PIXEL_SIZE               = glm::vec2(glm::vec2(1.0f) / glm::vec2(160.0f, 560.0f));
    AREATEX_SUBTEX_SIZE              = 0.14285714285f; //(1 / 7)
    AreaTexture                      = 0;
    SearchTexture                    = 0;


    m_Vertex_Shaders.resize(PassStage::_TOTAL, nullptr);
    m_Fragment_Shaders.resize(PassStage::_TOTAL, nullptr);
    m_Shader_Programs.resize(PassStage::_TOTAL, nullptr);

    m_Vertex_Shaders_Code.resize(PassStage::_TOTAL, "");
    m_Fragment_Shaders_Code.resize(PassStage::_TOTAL, "");
}
Engine::priv::SMAA::~SMAA() {
    glDeleteTextures(1, &SearchTexture);
    glDeleteTextures(1, &AreaTexture);

    SAFE_DELETE_VECTOR(m_Vertex_Shaders);
    SAFE_DELETE_VECTOR(m_Fragment_Shaders);
    SAFE_DELETE_VECTOR(m_Shader_Programs);
}
const bool Engine::priv::SMAA::init_shaders() {

    if (!m_Vertex_Shaders_Code[0].empty())
        return false;

#pragma region common

    string smaa_common =
        "vec4 mad(vec4 a, vec4 b, vec4 c){ return (a * b) + c; }\n"
        "vec3 mad(vec3 a, vec3 b, vec3 c){ return (a * b) + c; }\n"
        "vec2 mad(vec2 a, vec2 b, vec2 c){ return (a * b) + c; }\n"
        "float mad(float a, float b, float c){ return (a * b) + c; }\n"
        "vec4 saturate(vec4 a){ return clamp(a,0.0,1.0); }\n"
        "vec3 saturate(vec3 a){ return clamp(a,0.0,1.0); }\n"
        "vec2 saturate(vec2 a){ return clamp(a,0.0,1.0); }\n"
        "float saturate(float a){ return clamp(a,0.0,1.0); }\n"
        "vec4 round(vec4 a){ return floor(a + vec4(0.5)); }\n"
        "vec3 round(vec3 a){ return floor(a + vec3(0.5)); }\n"
        "vec2 round(vec2 a){ return floor(a + vec2(0.5)); }\n"
        "float round(float a){ return floor(a + 0.5); }\n"
        "void SMAAMovc(bvec2 cond, inout vec2 variable, vec2 value) {\n"
        "    if (cond.x) variable.x = value.x;\n"
        "    if (cond.y) variable.y = value.y;\n"
        "}\n"
        "void SMAAMovc(bvec4 cond, inout vec4 variable, vec4 value) {\n"
        "    SMAAMovc(cond.xy, variable.xy, value.xy);\n"
        "    SMAAMovc(cond.zw, variable.zw, value.zw);\n"
        "}\n"
        "\n"
        "float API_V_DIR(float v){ return -v; }\n"
        //"float API_V_COORD(float v){ return 1.0 - v; }\n"
        "bool API_V_BELOW(float v1, float v2){ if(v1 < v2) return true; return false; }\n"
        "bool API_V_ABOVE(float v1, float v2){ if(v1 > v2) return true; return false; }\n"
        "\n"
        //"float API_V_DIR(float v){ return v; }\n"
        "float API_V_COORD(float v){ return v; }\n"
        //"bool API_V_BELOW(float v1, float v2){ if(v1 > v2) return true; return false; }\n"
        //"bool API_V_ABOVE(float v1, float v2){ if(v1 < v2) return true; return false; }\n"
        "\n";

#pragma endregion

#pragma region VertexEdge

    m_Vertex_Shaders_Code[PassStage::Edge] = smaa_common +
        "\n"//edge vert
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "\n"
        "uniform vec4 SMAA_PIXEL_SIZE;\n" //make this globally inherit for all smaa shaders
        "uniform mat4 Model;\n"
        "uniform mat4 VP;\n"
        "uniform vec2 screenSizeDivideBy2;\n"
        "\n"
        "varying vec2 uv;\n"
        "varying vec4 _offset[3];\n"
        "\n"
        "void main(){\n"
        "    uv = UV;\n"
        "    vec3 vert = position;\n"
        "    vert.x *= screenSizeDivideBy2.x;\n"
        "    vert.y *= screenSizeDivideBy2.y;\n"
        "    _offset[0] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-1.0, 0.0, 0.0, API_V_DIR(-1.0)),uv.xyxy);\n"
        "    _offset[1] = mad(SMAA_PIXEL_SIZE.xyxy,vec4( 1.0, 0.0, 0.0,  API_V_DIR(1.0)),uv.xyxy);\n"
        "    _offset[2] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-2.0, 0.0, 0.0, API_V_DIR(-2.0)),uv.xyxy);\n"
        "    gl_Position = VP * Model * vec4(vert,1.0);\n"
        "}\n";

#pragma endregion

#pragma region FragEdge

    m_Fragment_Shaders_Code[PassStage::Edge] = smaa_common +
        "\n"//edge frag
        "\n"
        "uniform int SMAA_PREDICATION;\n"
        "const vec2 comparison = vec2(1.0,1.0);\n"
        "\n"
        "uniform sampler2D textureMap;\n"
        "uniform sampler2D texturePredication;\n"
        "\n"
        "uniform vec4 SMAAInfo1Floats;\n" //SMAA_THRESHOLD,SMAA_DEPTH_THRESHOLD,SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR,SMAA_PREDICATION_THRESHOLD
        "uniform vec4 SMAAInfo1FloatsA;\n" //SMAA_PREDICATION_SCALE,SMAA_PREDICATION_STRENGTH
        "\n"
        "varying vec2 uv;\n"
        "varying vec4 _offset[3];\n"
        "\n"
        "vec3 SMAAGatherNeighbours(vec2 texcoord,vec4 offset[3],sampler2D tex) {\n"
        "    float P = texture2D(tex, texcoord).r;\n"
        "    float Pleft = texture2D(tex, offset[0].xy).r;\n"
        "    float Ptop  = texture2D(tex, offset[0].zw).r;\n"
        "    return vec3(P, Pleft, Ptop);\n"
        "}\n"
        "vec2 SMAACalculatePredicatedThreshold(vec2 texcoord,vec4 offset[3]){\n"
        "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset,texturePredication);\n"
        "    vec2 delta = abs(neighbours.xx - neighbours.yz);\n"
        "    vec2 edges = step(SMAAInfo1Floats.w, delta);\n"
        "    return SMAAInfo1FloatsA.x * SMAAInfo1Floats.x * (1.0 - SMAAInfo1FloatsA.y * edges);\n"
        "}\n"
        "vec2 SMAADepthEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D depthTex) {\n"
        "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset, depthTex);\n"
        "    vec2 delta = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));\n"
        "    vec2 edges = step(SMAAInfo1Floats.y, delta);\n"
        "    if (dot(edges, comparison) == 0.0)\n"
        "        discard;\n"
        "    return edges;\n"
        "}\n"
        "vec2 SMAAColorEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D colorTex){\n"
        "    vec2 threshold;\n"
        "    if(SMAA_PREDICATION == 1){\n"
        "        threshold = SMAACalculatePredicatedThreshold(texcoord, offset);\n"
        "    }else{\n"
        "        threshold = vec2(SMAAInfo1Floats.x, SMAAInfo1Floats.x);\n"
        "    }\n"
        "    vec4 delta;\n"
        "    vec3 C = texture2D(colorTex, texcoord).rgb;\n"
        "    vec3 Cleft = texture2D(colorTex, offset[0].xy).rgb;\n"
        "    vec3 t = abs(C - Cleft);\n"
        "    delta.x = max(max(t.r, t.g), t.b);\n"
        "    vec3 Ctop  = texture2D(colorTex, offset[0].zw).rgb;\n"
        "    t = abs(C - Ctop);\n"
        "    delta.y = max(max(t.r, t.g), t.b);\n"
        "    vec2 edges = step(threshold, delta.xy);\n"
        "    if (dot(edges, comparison) == 0.0)\n"
        "        discard;\n"
        "    vec3 Cright = texture2D(colorTex, offset[1].xy).rgb;\n"
        "    t = abs(C - Cright);\n"
        "    delta.z = max(max(t.r, t.g), t.b);\n"
        "    vec3 Cbottom  = texture2D(colorTex, offset[1].zw).rgb;\n"
        "    t = abs(C - Cbottom);\n"
        "    delta.w = max(max(t.r, t.g), t.b);\n"
        "    vec2 maxDelta = max(delta.xy, delta.zw);\n"
        "    vec3 Cleftleft  = texture2D(colorTex, offset[2].xy).rgb;\n"
        "    t = abs(C - Cleftleft);\n"
        "    delta.z = max(max(t.r, t.g), t.b);\n"
        "    vec3 Ctoptop = texture2D(colorTex, offset[2].zw).rgb;\n"
        "    t = abs(C - Ctoptop);\n"
        "    delta.w = max(max(t.r, t.g), t.b);\n"
        "    maxDelta = max(maxDelta.xy, delta.zw);\n"
        "    float finalDelta = max(maxDelta.x, maxDelta.y);\n"
        "    edges.xy *= step((finalDelta), (SMAAInfo1Floats.z) * delta.xy);\n" //do we need this line in opengl?
        "    return edges;\n"
        "}\n"
        "vec2 SMAALumaEdgeDetectionPS(vec2 texcoord,vec4 offset[3],sampler2D colorTex) {\n"
        "    vec2 threshold;\n"
        "    if(SMAA_PREDICATION == 1){\n"
        "        threshold = SMAACalculatePredicatedThreshold(texcoord, offset);\n"
        "    }else{\n"
        "        threshold = vec2(SMAAInfo1Floats.x, SMAAInfo1Floats.x);\n"
        "    }\n"
        "    vec3 weights = vec3(0.2126, 0.7152, 0.0722);\n"
        "    float L =     dot(texture2D(colorTex, texcoord).rgb,     weights);\n"
        "    float Lleft = dot(texture2D(colorTex, offset[0].xy).rgb, weights);\n"
        "    float Ltop  = dot(texture2D(colorTex, offset[0].zw).rgb, weights);\n"
        "    vec4 delta;\n"
        "    delta.xy = abs(L - vec2(Lleft, Ltop));\n"
        "    vec2 edges = step(threshold, delta.xy);\n"
        "    if (dot(edges, comparison) == 0.0)\n"
        "        discard;\n"
        "    float Lright = dot(texture2D(colorTex, offset[1].xy).rgb, weights);\n"
        "    float Lbottom  = dot(texture2D(colorTex, offset[1].zw).rgb, weights);\n"
        "    delta.zw = abs(L - vec2(Lright, Lbottom));\n"
        "    vec2 maxDelta = max(delta.xy, delta.zw);\n"
        "    float Lleftleft = dot(texture2D(colorTex, offset[2].xy).rgb, weights);\n"
        "    float Ltoptop = dot(texture2D(colorTex, offset[2].zw).rgb, weights);\n"
        "    delta.zw = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));\n"
        "    maxDelta = max(maxDelta.xy, delta.zw);\n"
        "    float finalDelta = max(maxDelta.x, maxDelta.y);\n"
        "    edges.xy *= step((finalDelta), (SMAAInfo1Floats.z) * delta.xy);\n" //do we need this line in opengl?
        "    return edges;\n"
        "}\n"
        "void main(){\n"
        "    gl_FragColor = vec4(SMAAColorEdgeDetectionPS(uv, _offset, textureMap),0.0,1.0);\n"
        //"    gl_FragColor = vec4(SMAADepthEdgeDetectionPS(uv, _offset, textureMap),0.0,1.0);\n"
        //"    gl_FragColor = vec4(SMAALumaEdgeDetectionPS(uv, _offset, textureMap),0.0,1.0);\n"
        "}\n";

#pragma endregion

#pragma region VertBlend

        m_Vertex_Shaders_Code[PassStage::Blend] = smaa_common +
        "\n"//blend vert
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "\n"
        "uniform mat4 Model;\n"
        "uniform mat4 VP;\n"
        "uniform vec4 SMAA_PIXEL_SIZE;\n" //make this globally inherit for all smaa shaders
        "uniform int SMAA_MAX_SEARCH_STEPS;\n" //make this globally inherit for all smaa shaders
        "uniform vec2 screenSizeDivideBy2;\n"
        "\n"
        "varying vec2 uv;\n"
        "varying vec2 pixCoord;\n"
        "varying vec4 _offset[3];\n"
        "\n"
        "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
        "\n"
        "void main(){\n"
        "    uv = UV;\n"
        "    vec3 vert = position;\n"
        "    vert.x *= screenSizeDivideBy2.x;\n"
        "    vert.y *= screenSizeDivideBy2.y;\n"
        "    pixCoord = uv * SMAA_PIXEL_SIZE.zw;\n"
        "    _offset[0] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-0.25,API_V_DIR(-0.125), 1.25,API_V_DIR(-0.125)),uv.xyxy);\n"
        "    _offset[1] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-0.125,API_V_DIR(-0.25),-0.125,API_V_DIR(1.25)),uv.xyxy);\n"
        "    _offset[2] = mad(SMAA_PIXEL_SIZE.xxyy,vec4(-2.0, 2.0, API_V_DIR(-2.0), API_V_DIR(2.0)) * float(SMAA_MAX_SEARCH_STEPS),vec4(_offset[0].xz, _offset[1].yw));\n"
        "    gl_Position = VP * Model * vec4(vert,1.0);\n"
        "    _SMAA_PIXEL_SIZE = SMAA_PIXEL_SIZE;\n"
        "}\n";

#pragma endregion

#pragma region FragBlend

    m_Fragment_Shaders_Code[PassStage::Blend] = smaa_common +
        "\n"
        "\n"//blend frag
        "uniform sampler2D edge_tex;\n"
        "uniform sampler2D area_tex;\n"
        "uniform sampler2D search_tex;\n"
        "\n"
        "varying vec2 uv;\n"
        "varying vec2 pixCoord;\n"
        "varying vec4 _offset[3];\n"
        "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
        "\n"
        "uniform ivec4 SMAAInfo2Ints;\n" //x = MAX_SEARCH_STEPS_DIAG, y = AREATEX_MAX_DISTANCE, z = AREATEX_MAX_DISTANCE_DIAG, w = CORNER_ROUNDING
        "uniform vec4 SMAAInfo2Floats;\n" //x, y = SMAA_AREATEX_PIXEL_SIZE(x & y), z = SMAA_AREATEX_SUBTEX_SIZE, w = SMAA_CORNER_ROUNDING_NORM
        "\n"
        "float SMAASearchLength(sampler2D searchTex, vec2 e, float offset) {\n"
        "    vec2 scale = vec2(66.0, 33.0) * vec2(0.5, -1.0);\n"
        "    vec2 bias = vec2(66.0, 33.0) * vec2(offset, 1.0);\n"
        "    scale += vec2(-1.0,  1.0);\n"
        "    bias  += vec2( 0.5, -0.5);\n"
        "    scale *= 1.0 / vec2(64.0, 16.0);\n"
        "    bias *= 1.0 / vec2(64.0, 16.0);\n"
        "    vec2 coord = mad(scale, e, bias);\n"
        "    coord.y = API_V_COORD(coord.y);\n"
        "    return (texture2D(searchTex, coord)).r;\n"
        "}\n"
        "float SMAASearchXLeft(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {\n"
        "    vec2 e = vec2(0.0, 1.0);\n"
        "    while (texcoord.x > end && e.g > 0.8281 && e.r == 0.0) {\n"
        "        e = texture2D(edgesTex, texcoord).rg;\n"
        "        texcoord = mad(-vec2(2.0, 0.0), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
        "    }\n"
        "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e, 0.0), 3.25);\n"
        "    return mad(_SMAA_PIXEL_SIZE.x, offset, texcoord.x);\n"
        "}\n"
        "float SMAASearchXRight(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {\n"
        "    vec2 e = vec2(0.0, 1.0);\n"
        "    while (texcoord.x < end && e.g > 0.8281 && e.r == 0.0) {\n"
        "        e = texture2D(edgesTex, texcoord).rg;\n"
        "        texcoord = mad(vec2(2.0, 0.0), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
        "    }\n"
        "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e, 0.5), 3.25);\n"
        "    return mad(-_SMAA_PIXEL_SIZE.x, offset, texcoord.x);\n"
        "}\n"
        "float SMAASearchYUp(sampler2D edgesTex, sampler2D searchTex, vec2 texcoord, float end) {\n"
        "    vec2 e = vec2(1.0, 0.0);\n"
        "    while (API_V_BELOW(texcoord.y,end) && e.r > 0.8281 && e.g == 0.0) {\n"
        "        e = texture2D(edgesTex, texcoord).rg;\n"
        "        texcoord = mad(-vec2(0.0, API_V_DIR(2.0)), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
        "    }\n"
        "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e.gr, 0.0), 3.25);\n"
        "    return mad(_SMAA_PIXEL_SIZE.y, API_V_DIR(offset), texcoord.y);\n"
        "}\n"
        "float SMAASearchYDown(sampler2D edgesTex,sampler2D searchTex, vec2 texcoord, float end) {\n"
        "    vec2 e = vec2(1.0, 0.0);\n"
        "    while (API_V_ABOVE(texcoord.y,end) && e.r > 0.8281 && e.g == 0.0) {\n"
        "        e = texture2D(edgesTex, texcoord).rg;\n"
        "        texcoord = mad(vec2(0.0, API_V_DIR(2.0)), _SMAA_PIXEL_SIZE.xy, texcoord);\n"
        "    }\n"
        "    float offset = mad(-(255.0 / 127.0), SMAASearchLength(searchTex, e.gr, 0.5), 3.25);\n"
        "    return mad(-_SMAA_PIXEL_SIZE.y, API_V_DIR(offset), texcoord.y);\n"
        "}\n"
        "vec2 SMAAAreaDiag(sampler2D areaTex, vec2 dist, vec2 e, float offset) {\n"
        "    vec2 texcoord = mad(vec2(SMAAInfo2Ints.z, SMAAInfo2Ints.z), e, dist);\n"
        "    texcoord = mad(SMAAInfo2Floats.xy, texcoord, 0.5 * SMAAInfo2Floats.xy);\n"
        "    texcoord.x += 0.5;\n"
        "    texcoord.y += SMAAInfo2Floats.z * offset;\n"
        "    texcoord.y = API_V_COORD(texcoord.y);\n"
        "    return (texture2D(areaTex, texcoord)).rg;\n"
        "}\n"
        "vec2 SMAADecodeDiagBilinearAccess(vec2 e) {\n"
        "    e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);\n"
        "    return round(e);\n"
        "}\n"
        "vec4 SMAADecodeDiagBilinearAccess(vec4 e) {\n"
        "    e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);\n"
        "    return round(e);\n"
        "}\n"
        "vec2 SMAASearchDiag1(sampler2D edgesTex,vec2 texcoord,vec2 dir,out vec2 e) {\n"
        "    dir.y = API_V_DIR(dir.y);\n"
        "    vec4 coord = vec4(texcoord, -1.0, 1.0);\n"
        "    vec3 t = vec3(_SMAA_PIXEL_SIZE.xy, 1.0);\n"
        "    while (coord.z < float(SMAAInfo2Ints.x - 1) && coord.w > 0.9) {\n"
        "        coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);\n"
        "        e = texture2D(edgesTex, coord.xy).rg;\n"
        "        coord.w = dot(e, vec2(0.5));\n"
        "    }\n"
        "    return coord.zw;\n"
        "}\n"
        "vec2 SMAASearchDiag2(sampler2D edgesTex,vec2 texcoord,vec2 dir,out vec2 e) {\n"
        "    dir.y = API_V_DIR(dir.y);\n"
        "    vec4 coord = vec4(texcoord, -1.0, 1.0);\n"
        "    coord.x += 0.25 * _SMAA_PIXEL_SIZE.x;\n" // See @SearchDiag2Optimization
        "    vec3 t = vec3(_SMAA_PIXEL_SIZE.xy, 1.0);\n"
        "    while (coord.z < float(SMAAInfo2Ints.x - 1) && coord.w > 0.9) {\n"
        "        coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);\n"
        "        e = texture2D(edgesTex, coord.xy).rg;\n"
        "        e = SMAADecodeDiagBilinearAccess(e);\n"
        "        coord.w = dot(e, vec2(0.5));\n"
        "    }\n"
        "    return coord.zw;\n"
        "}\n"
        "vec2 SMAACalculateDiagWeights(sampler2D edgesTex,sampler2D areaTex,vec2 texcoord,vec2 e,vec4 subsampleIndices){\n"
        "    vec2 weights = vec2(0.0);\n"
        "    vec4 d;\n"
        "    vec2 end;\n"
        "    if (e.r > 0.0) {\n"
        "        d.xz = SMAASearchDiag1(edgesTex,texcoord,vec2(-1.0,1.0),end);\n"
        "        d.x += float(end.y > 0.9);\n"
        "    }else\n"
        "        d.xz = vec2(0.0);\n"
        "    d.yw = SMAASearchDiag1(edgesTex,texcoord,vec2(1.0,-1.0),end);\n"
        "    if (d.x + d.y > 2.0) {\n" // d.x + d.y + 1 > 3
        "        vec4 coords = mad(vec4(-d.x + 0.25, API_V_DIR(d.x), d.y, API_V_DIR(-d.y - 0.25)), _SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);\n"
        "        vec4 c;\n"
        "        c.xy = texture2D(edgesTex, coords.xy + vec2(-1.0, 0.0) * _SMAA_PIXEL_SIZE.xy).rg;\n"
        "        c.zw = texture2D(edgesTex, coords.zw + vec2( 1.0, 0.0) * _SMAA_PIXEL_SIZE.xy).rg;\n"
        "        c.yxwz = SMAADecodeDiagBilinearAccess(c.xyzw);\n"
        "        vec2 cc = mad(vec2(2.0), c.xz, c.yw);\n"
        "        SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));\n"
        "        weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.z);\n"
        "    }\n"
        "    d.xz = SMAASearchDiag2(edgesTex,texcoord,vec2(-1.0,-1.0), end);\n"
        "    if (texture2D(edgesTex, texcoord + vec2(1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).r > 0.0) {\n"
        "        d.yw = SMAASearchDiag2(edgesTex, texcoord, vec2(1.0), end);\n"
        "        d.y += float(end.y > 0.9);\n"
        "    }else\n"
        "        d.yw = vec2(0.0);\n"
        "    if (d.x + d.y > 2.0) {\n" // d.x + d.y + 1 > 3
        "        vec4 coords = mad(vec4(-d.x, API_V_DIR(-d.x), d.y, API_V_DIR(d.y)), _SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);\n"
        "        vec4 c;\n"
        "        c.x  = texture2D(edgesTex, coords.xy + vec2(-1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).g;\n"
        "        c.y  = texture2D(edgesTex, coords.xy + vec2( 0.0, API_V_DIR(-1.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
        "        c.zw = texture2D(edgesTex, coords.zw + vec2( 1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).gr;\n"
        "        vec2 cc = mad(vec2(2.0), c.xz, c.yw);\n"
        "        SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));\n"
        "        weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.w).gr;\n"
        "    }\n"
        "    return weights;\n"
        "}\n"
        "vec2 SMAAArea(sampler2D areaTex, vec2 dist, float e1, float e2, float offset) {\n"
        "    vec2 texcoord = mad(vec2(SMAAInfo2Ints.y, SMAAInfo2Ints.y), round(4.0 * vec2(e1, e2)), dist);\n"
        "    texcoord = mad(SMAAInfo2Floats.xy, texcoord, 0.5 * SMAAInfo2Floats.xy);\n"
        "    texcoord.y = mad(SMAAInfo2Floats.z, offset, texcoord.y);\n"
        "    texcoord.y = API_V_COORD(texcoord.y);\n"
        "    return (texture2D(areaTex, texcoord)).rg;\n"
        "}\n"
        "void SMAADetectHorizontalCornerPattern(sampler2D edgesTex, inout vec2 weights, vec4 texcoord, vec2 d) {\n"
        "    if(SMAAInfo2Ints.w == 0) return;\n"
        "    vec2 leftRight = step(d.xy, d.yx);\n"
        "    vec2 rounding = (1.0 - SMAAInfo2Floats.w) * leftRight;\n"
        "    rounding /= leftRight.x + leftRight.y;\n"
        "    vec2 factor = vec2(1.0, 1.0);\n"
        "    factor.x -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2(0.0,  API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
        "    factor.x -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2(1.0,  API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
        "    factor.y -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2(0.0, API_V_DIR(-2.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
        "    factor.y -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2(1.0, API_V_DIR(-2.0))*_SMAA_PIXEL_SIZE.xy).r;\n"
        "    weights *= saturate(factor);\n"
        "}\n"
        "void SMAADetectVerticalCornerPattern(sampler2D edgesTex, inout vec2 weights, vec4 texcoord, vec2 d) {\n"
        "    if(SMAAInfo2Ints.w == 0) return;\n"
        "    vec2 leftRight = step(d.xy, d.yx);\n"
        "    vec2 rounding = (1.0 - SMAAInfo2Floats.w) * leftRight;\n"
        "    rounding /= leftRight.x + leftRight.y;\n"
        "    vec2 factor = vec2(1.0, 1.0);\n"
        "    factor.x -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2( 1.0, 0.0)*_SMAA_PIXEL_SIZE.xy).g;\n"
        "    factor.x -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2( 1.0, API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).g;\n"
        "    factor.y -= rounding.x * texture2D(edgesTex, texcoord.xy + vec2(-2.0, 0.0)*_SMAA_PIXEL_SIZE.xy).g;\n"
        "    factor.y -= rounding.y * texture2D(edgesTex, texcoord.zw + vec2(-2.0, API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).g;\n"
        "    weights *= saturate(factor);\n"
        "}\n"
        "vec4 SMAABlendingWeightCalculationPS(vec2 texcoord,vec2 pixcoord,vec4 offset[3],sampler2D edgesTex,sampler2D areaTex,sampler2D searchTex,vec4 subsampleIndices) {\n"
        "    vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);\n"
        "    vec2 e = texture2D(edgesTex, texcoord).rg;\n"
        "    if (e.g > 0.0) {\n"
        "        if(SMAAInfo2Ints.x > 0){\n"
        "            weights.rg = SMAACalculateDiagWeights(edgesTex,areaTex, texcoord, e, subsampleIndices);\n"
        "            if (weights.r == -weights.g) {\n"
        "                vec2 d;\n"
        "                vec3 coords;\n"
        "                coords.x = SMAASearchXLeft(edgesTex,searchTex, offset[0].xy, offset[2].x);\n"
        "                coords.y = offset[1].y;\n"
        "                d.x = coords.x;\n"
        "                float e1 = texture2D(edgesTex, coords.xy).r;\n"
        "                coords.z = SMAASearchXRight(edgesTex,searchTex,offset[0].zw, offset[2].y);\n"
        "                d.y = coords.z;\n"
        "                d = abs(round(mad(_SMAA_PIXEL_SIZE.zz, d, -pixcoord.xx)));\n"
        "                vec2 sqrt_d = sqrt(d);\n"
        "                float e2 = texture2D(edgesTex, coords.zy + vec2(1.0,0.0)*_SMAA_PIXEL_SIZE.xy).r;\n"
        "                weights.rg = SMAAArea(areaTex, sqrt_d, e1, e2, subsampleIndices.y);\n"
        "                coords.y = texcoord.y;\n"
        "                SMAADetectHorizontalCornerPattern(edgesTex, weights.rg, coords.xyzy, d);\n"
        "            }else{ \n"
        "                e.r = 0.0; \n"
        "            }\n"
        "        }else{\n"
        "           vec2 d;\n"
        "           vec3 coords;\n"
        "           coords.x = SMAASearchXLeft(edgesTex,searchTex, offset[0].xy, offset[2].x);\n"
        "           coords.y = offset[1].y;\n"
        "           d.x = coords.x;\n"
        "           float e1 = texture2D(edgesTex, coords.xy).r;\n"
        "           coords.z = SMAASearchXRight(edgesTex,searchTex,offset[0].zw, offset[2].y);\n"
        "           d.y = coords.z;\n"
        "           d = abs(round(mad(_SMAA_PIXEL_SIZE.zz, d, -pixcoord.xx)));\n"
        "           vec2 sqrt_d = sqrt(d);\n"
        "           float e2 = texture2D(edgesTex, coords.zy + vec2(1.0,0.0)*_SMAA_PIXEL_SIZE.xy).r;\n"
        "           weights.rg = SMAAArea(areaTex, sqrt_d, e1, e2, subsampleIndices.y);\n"
        "           coords.y = texcoord.y;\n"
        "           SMAADetectHorizontalCornerPattern(edgesTex, weights.rg, coords.xyzy, d);\n"
        "        }\n"
        "    }\n"
        "    if (e.r > 0.0) {\n"
        "        vec2 d;\n"
        "        vec3 coords;\n"
        "        coords.y = SMAASearchYUp(edgesTex,searchTex, offset[1].xy, offset[2].z);\n"
        "        coords.x = offset[0].x;\n"
        "        d.x = coords.y;\n"
        "        float e1 = texture2D(edgesTex, coords.xy).g;\n"
        "        coords.z = SMAASearchYDown(edgesTex,searchTex, offset[1].zw, offset[2].w);\n"
        "        d.y = coords.z;\n"
        "        d = abs(round(mad(_SMAA_PIXEL_SIZE.ww, d, -pixcoord.yy)));\n"
        "        vec2 sqrt_d = sqrt(d);\n"
        "        float e2 = texture2D(edgesTex, coords.xz + vec2(0.0, API_V_DIR(1.0))*_SMAA_PIXEL_SIZE.xy).g;\n"
        "        weights.ba = SMAAArea(areaTex, sqrt_d, e1, e2, subsampleIndices.x);\n"
        "        coords.x = texcoord.x;\n"
        "        SMAADetectVerticalCornerPattern(edgesTex, weights.ba, coords.xyxz, d);\n"
        "    }\n"
        "    return weights;\n"
        "}\n"
        "void main(){\n"
        "    vec4 subSamples = vec4( 0.0 , 0.0 , 0.0 , 0.0 );\n"
        "    gl_FragColor = SMAABlendingWeightCalculationPS(uv,pixCoord,_offset,edge_tex,area_tex,search_tex,subSamples);\n"
        "}\n";

#pragma endregion

#pragma region VertNeighbor

    m_Vertex_Shaders_Code[PassStage::Neighbor] = smaa_common +
        "\n"//neightbor vert
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "\n"
        "uniform mat4 Model;\n"
        "uniform mat4 VP;\n"
        "uniform vec4 SMAA_PIXEL_SIZE;\n" //make this globally inherit for all smaa shaders
        "uniform vec2 screenSizeDivideBy2;\n"
        "\n"
        "varying vec2 uv;\n"
        "varying vec4 _offset;\n"
        "\n"
        "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
        "\n"
        "void main(){\n"
        "    uv = UV;\n"
        "    vec3 vert = position;\n"
        "    vert.x *= screenSizeDivideBy2.x;\n"
        "    vert.y *= screenSizeDivideBy2.y;\n"
        "    _offset = mad(SMAA_PIXEL_SIZE.xyxy,vec4(1.0,0.0,0.0,API_V_DIR(1.0)),uv.xyxy);\n"
        "    gl_Position = VP * Model * vec4(vert,1.0);\n"
        "    _SMAA_PIXEL_SIZE = SMAA_PIXEL_SIZE;\n"
        "}\n";

#pragma endregion

#pragma region FragNeighbor

    m_Fragment_Shaders_Code[PassStage::Neighbor] = smaa_common +
        "\n"//neighbor frag
        "uniform sampler2D textureMap;\n"
        "uniform sampler2D blend_tex;\n"
        "\n"
        "varying vec2 uv;\n"
        "varying vec4 _offset;\n"
        "\n"
        "flat varying vec4 _SMAA_PIXEL_SIZE;\n"
        "\n"
        "vec4 SMAANeighborhoodBlendingPS(vec2 texcoord,vec4 offset,sampler2D colorTex,sampler2D blendTex) {\n"
        "    vec4 a;\n"
        "    a.x = texture2D(blendTex, offset.xy).a;\n"
        "    a.y = texture2D(blendTex, offset.zw).g;\n"
        "    a.wz = texture2D(blendTex, texcoord).xz;\n"
        "    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) <= 1e-5) {\n"
        "        vec4 color = texture2DLod(colorTex, texcoord,0.0);\n"
        "        return color;\n"
        "    }else{\n"
        "        bool h = max(a.x, a.z) > max(a.y, a.w);\n"
        "        vec4 blendingOffset = vec4(0.0, API_V_DIR(a.y), 0.0, API_V_DIR(a.w));\n"
        "        vec2 blendingWeight = a.yw;\n"
        "        SMAAMovc(bvec4(h, h, h, h), blendingOffset, vec4(a.x, 0.0, a.z, 0.0));\n"
        "        SMAAMovc(bvec2(h, h), blendingWeight, a.xz);\n"
        "        blendingWeight /= dot(blendingWeight, vec2(1.0, 1.0));\n"
        "        vec4 blendingCoord = mad(blendingOffset, vec4(_SMAA_PIXEL_SIZE.xy, -_SMAA_PIXEL_SIZE.xy), texcoord.xyxy);\n"
        "        vec4 color = blendingWeight.x * texture2D(colorTex, blendingCoord.xy);\n"
        "        color += blendingWeight.y * texture2D(colorTex, blendingCoord.zw);\n"
        "        return color;\n"
        "    }\n"
        "}\n"
        "void main(){\n"
        "    gl_FragColor = SMAANeighborhoodBlendingPS(uv, _offset, textureMap, blend_tex);\n"
        "}";

#pragma endregion

    //vertex & frag 4 are optional passes

#pragma region VertFinal

    m_Vertex_Shaders_Code[PassStage::Final] = smaa_common +
        "\n"
        "layout (location = 0) in vec3 position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "\n"
        "uniform mat4 Model;\n"
        "uniform mat4 VP;\n"
        "uniform vec2 screenSizeDivideBy2;\n"
        "\n"
        "varying vec2 uv;\n"
        "\n"
        "void SMAAResolveVS(inout vec2 uv){\n"
        "}\n"
        "void SMAASeparateVS(inout vec2 uv){\n"
        "}\n"
        "void main(){\n"
        "    uv = UV;\n"
        "    vec3 vert = position;\n"
        "    vert.x *= screenSizeDivideBy2.x;\n"
        "    vert.y *= screenSizeDivideBy2.y;\n"
        "    gl_Position = VP * Model * vec4(vert,1.0);\n"
        "}";
#pragma endregion

#pragma region FragFinal
    m_Fragment_Shaders_Code[PassStage::Final] = smaa_common +
        "\n"
        "varying vec2 uv;\n"
        "vec4 SMAAResolvePS(vec2 texcoord,sampler2D currentColorTex,sampler2D previousColorTex){\n"
        "    vec4 current = texture2D(currentColorTex, texcoord);\n"
        "    vec4 previous = texture2D(previousColorTex, texcoord);\n"
        "    return mix(current, previous, 0.5);\n"
        "}\n"
        "void main(){\n"
        "    gl_FragColor = vec4(0.0,0.0,0.0,1.0);\n"
        "}";
#pragma endregion

    for (unsigned int i = 0; i < PassStage::_TOTAL; ++i) {
        auto lambda_part_a = [&, i]() {
            m_Vertex_Shaders[i] = NEW Shader(m_Vertex_Shaders_Code[i], ShaderType::Vertex, false);
            m_Fragment_Shaders[i] = NEW Shader(m_Fragment_Shaders_Code[i], ShaderType::Fragment, false);
        };
        auto lambda_part_b = [&, i]() {
            m_Shader_Programs[i] = NEW ShaderProgram("SMAA_" + std::to_string(i), *m_Vertex_Shaders[i], *m_Fragment_Shaders[i]);
        };
        Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);
    }
    return true;
}


void Engine::priv::SMAA::init() {
    Engine::Renderer::genAndBindTexture(GL_TEXTURE_2D, AreaTexture);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 160, 560, 0, GL_RG, GL_UNSIGNED_BYTE, SMAA_areaTexBytes);

    Engine::Renderer::genAndBindTexture(GL_TEXTURE_2D, SearchTexture);
    Texture::setFilter(GL_TEXTURE_2D, TextureFilter::Linear);
    Texture::setWrapping(GL_TEXTURE_2D, TextureWrap::ClampToBorder);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 64, 16, 0, GL_RED, GL_UNSIGNED_BYTE, SMAA_searchTexBytes);
}
void Engine::priv::SMAA::passEdge(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, const unsigned int& sceneTexture, const unsigned int& outTexture) {
    gbuffer.bindFramebuffers(outTexture); //probably the lighting buffer
    m_Shader_Programs[PassStage::Edge]->bind();

    const auto& dimensions = viewport.getViewportDimensions();

    Engine::Renderer::Settings::clear(true, false, true);//lighting rgba, stencil is completely filled with 0's

    Engine::Renderer::stencilMask(0xFFFFFFFF);
    Engine::Renderer::stencilFunc(GL_ALWAYS, 0xFFFFFFFF, 0xFFFFFFFF);
    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR, GL_INCR);
    Engine::Renderer::GLEnable(GL_STENCIL_TEST);

    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendUniform4Safe("SMAAInfo1Floats", THRESHOLD, DEPTH_THRESHOLD, LOCAL_CONTRAST_ADAPTATION_FACTOR, PREDICATION_THRESHOLD);
    Engine::Renderer::sendUniform2Safe("SMAAInfo1FloatsA", PREDICATION_SCALE, PREDICATION_STRENGTH);

    Engine::Renderer::sendUniform1Safe("SMAA_PREDICATION", static_cast<int>(PREDICATION));

    Engine::Renderer::sendTexture("textureMap", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("texturePredication", gbuffer.getTexture(GBufferType::Diffuse), 1);

    Engine::Renderer::renderFullscreenTriangle(0,0, dimensions.z, dimensions.w);

    Engine::Renderer::stencilMask(0xFFFFFFFF);
    Engine::Renderer::stencilFunc(GL_EQUAL, 0x00000001, 0x00000001);
    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil
}
void Engine::priv::SMAA::passBlend(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, const unsigned int& outTexture) {
    gbuffer.bindFramebuffers(GBufferType::Normal);
    Engine::Renderer::Settings::clear(true, false, false); //clear color only

    const auto& dimensions = viewport.getViewportDimensions();

    m_Shader_Programs[PassStage::Blend]->bind();
    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendTexture("edge_tex", gbuffer.getTexture(outTexture), 0);
    Engine::Renderer::sendTexture("area_tex", AreaTexture, 1, GL_TEXTURE_2D);
    Engine::Renderer::sendTexture("search_tex", SearchTexture, 2, GL_TEXTURE_2D);

    Engine::Renderer::sendUniform1Safe("SMAA_MAX_SEARCH_STEPS", MAX_SEARCH_STEPS);

    Engine::Renderer::sendUniform4Safe("SMAAInfo2Ints", MAX_SEARCH_STEPS_DIAG, AREATEX_MAX_DISTANCE, AREATEX_MAX_DISTANCE_DIAG, CORNER_ROUNDING);
    Engine::Renderer::sendUniform4Safe("SMAAInfo2Floats", AREATEX_PIXEL_SIZE.x, AREATEX_PIXEL_SIZE.y, AREATEX_SUBTEX_SIZE, (static_cast<float>(CORNER_ROUNDING) / 100.0f));

    Engine::Renderer::renderFullscreenTriangle(0,0, dimensions.z, dimensions.w);

    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
}
void Engine::priv::SMAA::passNeighbor(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, const unsigned int& sceneTexture) {
    m_Shader_Programs[PassStage::Neighbor]->bind();

    const auto& dimensions = viewport.getViewportDimensions();

    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);
    Engine::Renderer::sendTextureSafe("textureMap", gbuffer.getTexture(sceneTexture), 0); //need original final image from first smaa pass
    Engine::Renderer::sendTextureSafe("blend_tex", gbuffer.getTexture(GBufferType::Normal), 1);

    Engine::Renderer::renderFullscreenTriangle(0,0, dimensions.z, dimensions.w);
}
void Engine::priv::SMAA::passFinal(Engine::priv::GBuffer& gbuffer, const Viewport& viewport) {
    /*
    //this pass is optional. lets skip it for now
    //gbuffer.bindFramebuffers(GBufferType::Lighting);
    gbuffer.stop();
    m_Shader_Programs[PassStage::Final]->bind();
    Engine::Renderer::renderFullscreenTriangle(0,0,fboWidth,fboHeight);
    */
}
void Engine::Renderer::smaa::setThreshold(const float f) {
    Engine::priv::SMAA::smaa.THRESHOLD = f;
}
void Engine::Renderer::smaa::setSearchSteps(const unsigned int s) {
    Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = s;
}
void Engine::Renderer::smaa::disableCornerDetection() {
    Engine::priv::SMAA::smaa.CORNER_ROUNDING = 0;
}
void Engine::Renderer::smaa::enableCornerDetection(const unsigned int c) {
    Engine::priv::SMAA::smaa.CORNER_ROUNDING = c;
}
void Engine::Renderer::smaa::disableDiagonalDetection() {
    Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 0;
}
void Engine::Renderer::smaa::enableDiagonalDetection(const unsigned int d) {
    Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = d;
}
void Engine::Renderer::smaa::setPredicationThreshold(const float f) {
    Engine::priv::SMAA::smaa.PREDICATION_THRESHOLD = f;
}
void Engine::Renderer::smaa::setPredicationScale(const float f) {
    Engine::priv::SMAA::smaa.PREDICATION_SCALE = f;
}
void Engine::Renderer::smaa::setPredicationStrength(const float s) {
    Engine::priv::SMAA::smaa.PREDICATION_STRENGTH = s;
}
void Engine::Renderer::smaa::setReprojectionScale(const float s) {
    Engine::priv::SMAA::smaa.REPROJECTION_WEIGHT_SCALE = s;
}
void Engine::Renderer::smaa::enablePredication(const bool b) {
    Engine::priv::SMAA::smaa.PREDICATION = b;
}
void Engine::Renderer::smaa::disablePredication() {
    Engine::priv::SMAA::smaa.PREDICATION = false;
}
void Engine::Renderer::smaa::enableReprojection(const bool b) {
    Engine::priv::SMAA::smaa.REPROJECTION = b;
}
void Engine::Renderer::smaa::disableReprojection() {
    Engine::priv::SMAA::smaa.REPROJECTION = false;
}
void Engine::Renderer::smaa::setQuality(const SMAAQualityLevel::Level level) {
    switch (level) {
        case SMAAQualityLevel::Low: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.15f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 4;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 0;
            break;
        }case SMAAQualityLevel::Medium: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.1f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 8;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 0;
            break;
        }case SMAAQualityLevel::High: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.1f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 16;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 8;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 25;
            break;
        }case SMAAQualityLevel::Ultra: {
            Engine::priv::SMAA::smaa.THRESHOLD = 0.05f;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS = 32;
            Engine::priv::SMAA::smaa.MAX_SEARCH_STEPS_DIAG = 16;
            Engine::priv::SMAA::smaa.CORNER_ROUNDING = 25;
            break;
        }default: {
            break;
        }
    }
}
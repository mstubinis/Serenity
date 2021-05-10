
#include <serenity/renderer/postprocess/SMAA.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/renderer/SMAA_LUT.h>
#include <serenity/renderer/GBuffer.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/scene/Viewport.h>
#include <serenity/resources/shader/ShaderProgram.h>
#include <serenity/resources/shader/Shader.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/resources/Engine_Resources.h>

Engine::priv::SMAA Engine::priv::SMAA::STATIC_SMAA;

Engine::priv::SMAA::SMAA() {
    m_Vertex_Shaders.resize(PassStage::_TOTAL, Handle{});
    m_Fragment_Shaders.resize(PassStage::_TOTAL, Handle{});
    m_Shader_Programs.resize(PassStage::_TOTAL, Handle{});

    m_Vertex_Shaders_Code.resize(PassStage::_TOTAL, "");
    m_Fragment_Shaders_Code.resize(PassStage::_TOTAL, "");
}
Engine::priv::SMAA::~SMAA() {
    glDeleteTextures(1, &SearchTexture);
    glDeleteTextures(1, &AreaTexture);
}
void Engine::priv::SMAA::internal_init_edge_vertex_code(const std::string& common) {
    STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Edge] = common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;

uniform vec4 SMAA_PIXEL_SIZE; //make this globally inherit for all smaa shaders
uniform mat4 Model;
uniform mat4 VP;

varying vec2 uv;
varying vec4 _offset[3];

void main(){
    uv = UV;
    vec3 vert = position;
    _offset[0] = mad(SMAA_PIXEL_SIZE.xyxy, vec4(-1.0, 0.0, 0.0, API_V_DIR(-1.0)), uv.xyxy);
    _offset[1] = mad(SMAA_PIXEL_SIZE.xyxy, vec4( 1.0, 0.0, 0.0,  API_V_DIR(1.0)), uv.xyxy);
    _offset[2] = mad(SMAA_PIXEL_SIZE.xyxy, vec4(-2.0, 0.0, 0.0, API_V_DIR(-2.0)), uv.xyxy);
    gl_Position = VP * Model * vec4(vert, 1.0);
}
    )";
}
void Engine::priv::SMAA::internal_init_edge_fragment_code(const std::string& common) {
    STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Edge] = common +
        "uniform int SMAA_PREDICATION;\n"
        "const vec2 comparison = vec2(1.0,1.0);\n"
        "\n"
        "uniform SAMPLER_TYPE_2D textureMap;\n"
        "uniform SAMPLER_TYPE_2D texturePredication;\n"
        "\n"
        "uniform vec4 SMAAInfo1Floats;\n" //SMAA_THRESHOLD,SMAA_DEPTH_THRESHOLD,SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR,SMAA_PREDICATION_THRESHOLD
        "uniform vec2 SMAAInfo1FloatsA;\n" //SMAA_PREDICATION_SCALE,SMAA_PREDICATION_STRENGTH
        "\n"
        "varying vec2 uv;\n"
        "varying vec4 _offset[3];\n"
        "\n"
        "vec3 SMAAGatherNeighbours(vec2 texcoord, vec4 offset[3], sampler2D inTexture) {\n"
        "    float P = texture2D(inTexture, texcoord).r;\n"
        "    float Pleft = texture2D(inTexture, offset[0].xy).r;\n"
        "    float Ptop  = texture2D(inTexture, offset[0].zw).r;\n"
        "    return vec3(P, Pleft, Ptop);\n"
        "}\n"
        "vec2 SMAACalculatePredicatedThreshold(vec2 texcoord, vec4 offset[3], sampler2D inTexturePredication){\n"
        "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset, inTexturePredication);\n"
        "    vec2 delta = abs(neighbours.xx - neighbours.yz);\n"
        "    vec2 edges = step(SMAAInfo1Floats.w, delta);\n"
        "    return SMAAInfo1FloatsA.x * SMAAInfo1Floats.x * (1.0 - SMAAInfo1FloatsA.y * edges);\n"
        "}\n"
        "vec2 SMAADepthEdgeDetectionPS(vec2 texcoord, vec4 offset[3],sampler2D depthTex) {\n"
        "    vec3 neighbours = SMAAGatherNeighbours(texcoord, offset, depthTex);\n"
        "    vec2 delta = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));\n"
        "    vec2 edges = step(SMAAInfo1Floats.y, delta);\n"
        "    if (dot(edges, comparison) == 0.0)\n"
        "        discard;\n"
        "    return edges;\n"
        "}\n"
        "vec2 SMAAColorEdgeDetectionPS(vec2 texcoord, vec4 offset[3], sampler2D colorTex, sampler2D inTexturePredication){\n"
        "    vec2 threshold;\n"
        "    if(SMAA_PREDICATION == 1){\n"
        "        threshold = SMAACalculatePredicatedThreshold(texcoord, offset, inTexturePredication);\n"
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
        "vec2 SMAALumaEdgeDetectionPS(vec2 texcoord,vec4 offset[3], sampler2D colorTex, sampler2D inTexturePredication) {\n"
        "    vec2 threshold;\n"
        "    if(SMAA_PREDICATION == 1){\n"
        "        threshold = SMAACalculatePredicatedThreshold(texcoord, offset, inTexturePredication);\n"
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
        "    gl_FragColor = vec4(SMAAColorEdgeDetectionPS(uv, _offset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(texturePredication)), 0.0, 1.0);\n"
        //"    gl_FragColor = vec4(SMAADepthEdgeDetectionPS(uv, _offset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(texturePredication)), 0.0, 1.0);\n"
        //"    gl_FragColor = vec4(SMAALumaEdgeDetectionPS(uv, _offset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(texturePredication)), 0.0, 1.0);\n"
        "}\n";
}
void Engine::priv::SMAA::internal_init_blend_vertex_code(const std::string& common) {
    STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Blend] = common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;
uniform mat4 Model;
uniform mat4 VP;
uniform vec4 SMAA_PIXEL_SIZE; //make this globally inherit for all smaa shaders
uniform int SMAA_MAX_SEARCH_STEPS; //make this globally inherit for all smaa shaders
varying vec2 uv;
varying vec2 pixCoord;
varying vec4 _offset[3];
flat varying vec4 _SMAA_PIXEL_SIZE;
void main(){
    uv = UV;
    vec3 vert = position;
    pixCoord = uv * SMAA_PIXEL_SIZE.zw;
    _offset[0] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-0.25,API_V_DIR(-0.125), 1.25,API_V_DIR(-0.125)),uv.xyxy);
    _offset[1] = mad(SMAA_PIXEL_SIZE.xyxy,vec4(-0.125,API_V_DIR(-0.25),-0.125,API_V_DIR(1.25)),uv.xyxy);
    _offset[2] = mad(SMAA_PIXEL_SIZE.xxyy,vec4(-2.0, 2.0, API_V_DIR(-2.0), API_V_DIR(2.0)) * float(SMAA_MAX_SEARCH_STEPS),vec4(_offset[0].xz, _offset[1].yw));
    gl_Position = VP * Model * vec4(vert,1.0);
    _SMAA_PIXEL_SIZE = SMAA_PIXEL_SIZE;
}
    )";
}
void Engine::priv::SMAA::internal_init_blend_fragment_code(const std::string& common) {
    STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Blend] = common +
        "uniform SAMPLER_TYPE_2D edge_tex;\n"
        "uniform SAMPLER_TYPE_2D area_tex;\n"
        "uniform SAMPLER_TYPE_2D search_tex;\n"

        "varying vec2 uv;\n"
        "varying vec2 pixCoord;\n"
        "varying vec4 _offset[3];\n"
        "flat varying vec4 _SMAA_PIXEL_SIZE;\n"

        "uniform ivec4 SMAAInfo2Ints;\n" //x = MAX_SEARCH_STEPS_DIAG, y = AREATEX_MAX_DISTANCE, z = AREATEX_MAX_DISTANCE_DIAG, w = CORNER_ROUNDING
        "uniform vec4 SMAAInfo2Floats;\n" //x, y = SMAA_AREATEX_PIXEL_SIZE(x & y), z = SMAA_AREATEX_SUBTEX_SIZE, w = SMAA_CORNER_ROUNDING_NORM

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
        "    gl_FragColor = SMAABlendingWeightCalculationPS(uv, pixCoord, _offset, USE_SAMPLER_2D(edge_tex), USE_SAMPLER_2D(area_tex), USE_SAMPLER_2D(search_tex), subSamples);\n"
        "}\n";
}
void Engine::priv::SMAA::internal_init_neighbor_vertex_code(const std::string& common) {
    STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Neighbor] = common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;
uniform mat4 Model;
uniform mat4 VP;
uniform vec4 SMAA_PIXEL_SIZE;
varying vec2 uv;
varying vec4 _offset;
flat varying vec4 _SMAA_PIXEL_SIZE;
void main(){
    uv = UV;
    vec3 vert = position;
    _offset = mad(SMAA_PIXEL_SIZE.xyxy,vec4(1.0, 0.0, 0.0, API_V_DIR(1.0)), uv.xyxy);
    gl_Position = VP * Model * vec4(vert, 1.0);
    _SMAA_PIXEL_SIZE = SMAA_PIXEL_SIZE;
}
    )";
}
void Engine::priv::SMAA::internal_init_neighbor_fragment_code(const std::string& common) {
    STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Neighbor] = common + R"(
uniform SAMPLER_TYPE_2D textureMap;
uniform SAMPLER_TYPE_2D blend_tex;
varying vec2 uv;
varying vec4 _offset;
flat varying vec4 _SMAA_PIXEL_SIZE;
vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 offset, sampler2D inColorTexture, sampler2D inBlendTexture) {
    vec4 a;
    a.x = texture2D(inBlendTexture, offset.xy).a;
    a.y = texture2D(inBlendTexture, offset.zw).g;
    a.wz = texture2D(inBlendTexture, texcoord).xz;
    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) <= 1e-5) {
        vec4 color = texture2DLod(inColorTexture, texcoord,0.0);
        return color;
    }else{
        bool h = max(a.x, a.z) > max(a.y, a.w);
        vec4 blendingOffset = vec4(0.0, API_V_DIR(a.y), 0.0, API_V_DIR(a.w));
        vec2 blendingWeight = a.yw;
        SMAAMovc(bvec4(h, h, h, h), blendingOffset, vec4(a.x, 0.0, a.z, 0.0));
        SMAAMovc(bvec2(h, h), blendingWeight, a.xz);
        blendingWeight /= dot(blendingWeight, vec2(1.0, 1.0));
        vec4 blendingCoord = mad(blendingOffset, vec4(_SMAA_PIXEL_SIZE.xy, -_SMAA_PIXEL_SIZE.xy), texcoord.xyxy);
        vec4 color = blendingWeight.x * texture2D(inColorTexture, blendingCoord.xy);
        color += blendingWeight.y * texture2D(inColorTexture, blendingCoord.zw);
        return color;
    }
}
void main(){
    gl_FragColor = SMAANeighborhoodBlendingPS(uv, _offset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(blend_tex));
}
    )";
}
bool Engine::priv::SMAA::init() {

    if (!STATIC_SMAA.m_Vertex_Shaders_Code[0].empty())
        return false;

    std::string smaa_common = R"(
vec4 mad(vec4 a, vec4 b, vec4 c){ return (a * b) + c; }
vec3 mad(vec3 a, vec3 b, vec3 c){ return (a * b) + c; }
vec2 mad(vec2 a, vec2 b, vec2 c){ return (a * b) + c; }
float mad(float a, float b, float c){ return (a * b) + c; }
vec4 saturate(vec4 a){ return clamp(a,0.0,1.0); }
vec3 saturate(vec3 a){ return clamp(a,0.0,1.0); }
vec2 saturate(vec2 a){ return clamp(a,0.0,1.0); }
float saturate(float a){ return clamp(a,0.0,1.0); }
vec4 round(vec4 a){ return floor(a + vec4(0.5)); }
vec3 round(vec3 a){ return floor(a + vec3(0.5)); }
vec2 round(vec2 a){ return floor(a + vec2(0.5)); }
float round(float a){ return floor(a + 0.5); }
void SMAAMovc(bvec2 cond, inout vec2 variable, vec2 value) {
    if (cond.x) variable.x = value.x;
    if (cond.y) variable.y = value.y;
}
void SMAAMovc(bvec4 cond, inout vec4 variable, vec4 value) {
    SMAAMovc(cond.xy, variable.xy, value.xy);
    SMAAMovc(cond.zw, variable.zw, value.zw);
}
float API_V_DIR(float v){ return -v; }
//float API_V_COORD(float v){ return 1.0 - v; }
bool API_V_BELOW(float v1, float v2){ if(v1 < v2) return true; return false; }
bool API_V_ABOVE(float v1, float v2){ if(v1 > v2) return true; return false; }

//float API_V_DIR(float v){ return v; }
float API_V_COORD(float v){ return v; }
//bool API_V_BELOW(float v1, float v2){ if(v1 > v2) return true; return false; }
//bool API_V_ABOVE(float v1, float v2){ if(v1 < v2) return true; return false; }
     )";

    STATIC_SMAA.internal_init_edge_vertex_code(smaa_common);
    STATIC_SMAA.internal_init_edge_fragment_code(smaa_common);

    STATIC_SMAA.internal_init_blend_vertex_code(smaa_common);
    STATIC_SMAA.internal_init_blend_fragment_code(smaa_common);

    STATIC_SMAA.internal_init_neighbor_vertex_code(smaa_common);
    STATIC_SMAA.internal_init_neighbor_fragment_code(smaa_common);

    //vertex & frag 4 are optional passes

#pragma region VertFinal
        STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Final] = smaa_common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;
uniform mat4 Model;
uniform mat4 VP;
varying vec2 uv;
void SMAAResolveVS(inout vec2 uv){}
void SMAASeparateVS(inout vec2 uv){}
void main(){
    uv = UV;
    vec3 vert = position;
    gl_Position = VP * Model * vec4(vert, 1.0);
}
    )";
#pragma endregion

#pragma region FragFinal
        STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Final] = smaa_common + R"(
varying vec2 uv;
vec4 SMAAResolvePS(vec2 texcoord, sampler2D currentColorTex, sampler2D previousColorTex){
    vec4 current = texture2D(currentColorTex, texcoord);
    vec4 previous = texture2D(previousColorTex, texcoord);
    return mix(current, previous, 0.5);
}
void main(){
    gl_FragColor = vec4(0.0,0.0,0.0,1.0);
}
    )";
#pragma endregion

    for (uint32_t i = 0; i < PassStage::_TOTAL; ++i) {
        auto lambda_part_a = [i]() {
            STATIC_SMAA.m_Vertex_Shaders[i]   = Engine::Resources::addResource<Shader>(STATIC_SMAA.m_Vertex_Shaders_Code[i], ShaderType::Vertex, false);
            STATIC_SMAA.m_Fragment_Shaders[i] = Engine::Resources::addResource<Shader>(STATIC_SMAA.m_Fragment_Shaders_Code[i], ShaderType::Fragment, false);
        };
        auto lambda_part_b = [i]() {
            STATIC_SMAA.m_Shader_Programs[i]  = Engine::Resources::addResource<ShaderProgram>("SMAA_" + std::to_string(i), STATIC_SMAA.m_Vertex_Shaders[i], STATIC_SMAA.m_Fragment_Shaders[i]);
        };
        Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);
    }

    const TextureType textureType = TextureType::Texture2D;
    Engine::Renderer::genAndBindTexture(textureType, STATIC_SMAA.AreaTexture);
    glTexImage2D(textureType.toGLType(), 0, GL_RG8, 160, 560, 0, GL_RG, GL_UNSIGNED_BYTE, SMAA_areaTexBytes);
    Texture::setFilter(textureType, TextureFilter::Linear);
    TextureBaseClass::setWrapping(textureType, TextureWrap::ClampToBorder);

    Engine::Renderer::genAndBindTexture(textureType, STATIC_SMAA.SearchTexture);
    glTexImage2D(textureType.toGLType(), 0, GL_R8, 64, 16, 0, GL_RED, GL_UNSIGNED_BYTE, SMAA_searchTexBytes);
    Texture::setFilter(textureType, TextureFilter::Linear);
    TextureBaseClass::setWrapping(textureType, TextureWrap::ClampToBorder);

    return true;
}

constexpr uint32_t TEXTURE_INTERMEDIARY = Engine::priv::GBufferType::Normal;

void Engine::priv::SMAA::passEdge(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t sceneTexture, uint32_t outTexture, const Engine::priv::RenderModule& renderer) {
    gbuffer.bindFramebuffers(outTexture, "RGBA"); //probably the lighting buffer
    renderer.bind(m_Shader_Programs[PassStage::Edge].get<ShaderProgram>());

    Engine::Renderer::Settings::clear(true, false, true);//lighting rgba, stencil is completely filled with 0's

    Engine::Renderer::GLEnable(GL_STENCIL_TEST);
    Engine::Renderer::stencilMask(0b11111111);

    //fragments written during the edge test always pass. the edge fragment pass will discard bad pixels, leaving their stencil value in the buffer at zero.
    Engine::Renderer::stencilFunc(GL_ALWAYS, 0b11111111, 0b11111111);

    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR, GL_INCR);
    
    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendUniform4Safe("SMAAInfo1Floats", THRESHOLD, DEPTH_THRESHOLD, LOCAL_CONTRAST_ADAPTATION_FACTOR, PREDICATION_THRESHOLD);
    Engine::Renderer::sendUniform2Safe("SMAAInfo1FloatsA", PREDICATION_SCALE, PREDICATION_STRENGTH);

    Engine::Renderer::sendUniform1Safe("SMAA_PREDICATION", (int)PREDICATION);

    Engine::Renderer::sendTexture("textureMap", gbuffer.getTexture(sceneTexture), 0);
    Engine::Renderer::sendTextureSafe("texturePredication", gbuffer.getTexture(GBufferType::Diffuse), 1);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);

    Engine::Renderer::stencilMask(0b11111111);
    //only if the stored stencil value is 0b00000001 will the fragments be evaluated
    Engine::Renderer::stencilFunc(GL_EQUAL, 0b00000001, 0b11111111);
    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil
}
void Engine::priv::SMAA::passBlend(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t outTexture, const Engine::priv::RenderModule& renderer) {
    gbuffer.bindFramebuffers(TEXTURE_INTERMEDIARY, "RGBA");
    Engine::Renderer::Settings::clear(true, false, false); //clear color only

    renderer.bind(m_Shader_Programs[PassStage::Blend].get<ShaderProgram>());

    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendTexture("edge_tex", gbuffer.getTexture(outTexture), 0);
    Engine::Renderer::sendTexture("area_tex", AreaTexture, 1, GL_TEXTURE_2D);
    Engine::Renderer::sendTexture("search_tex", SearchTexture, 2, GL_TEXTURE_2D);

    Engine::Renderer::sendUniform1Safe("SMAA_MAX_SEARCH_STEPS", MAX_SEARCH_STEPS);

    Engine::Renderer::sendUniform4Safe("SMAAInfo2Ints", MAX_SEARCH_STEPS_DIAG, AREATEX_MAX_DISTANCE, AREATEX_MAX_DISTANCE_DIAG, CORNER_ROUNDING);
    Engine::Renderer::sendUniform4Safe("SMAAInfo2Floats", AREATEX_PIXEL_SIZE.x, AREATEX_PIXEL_SIZE.y, AREATEX_SUBTEX_SIZE, (float)CORNER_ROUNDING / 100.0f);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(2, GL_TEXTURE_2D);

    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
}
void Engine::priv::SMAA::passNeighbor(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Programs[PassStage::Neighbor].get<ShaderProgram>());

    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);
    Engine::Renderer::sendTextureSafe("textureMap", gbuffer.getTexture(sceneTexture), 0); //need original final image from first smaa pass
    Engine::Renderer::sendTextureSafe("blend_tex", gbuffer.getTexture(TEXTURE_INTERMEDIARY), 1);

    Engine::Renderer::renderFullscreenQuad();

    Engine::Renderer::clearTexture(0, GL_TEXTURE_2D);
    Engine::Renderer::clearTexture(1, GL_TEXTURE_2D);
}
void Engine::priv::SMAA::passFinal(Engine::priv::GBuffer& gbuffer, const Viewport& viewport, const Engine::priv::RenderModule& renderer) {
    /*
    //this pass is optional. lets skip it for now
    //gbuffer.bindFramebuffers(GBufferType::Lighting);
    gbuffer.stop();

    renderer.bind(m_Shader_Programs[PassStage::Final]);

    Engine::Renderer::renderFullscreenTriangle(0,0,fboWidth,fboHeight);
    */
}
void Engine::Renderer::smaa::setQuality(SMAAQualityLevel qualityLevel) {
    switch (qualityLevel) {
        case SMAAQualityLevel::Low: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.15f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 4;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 0;
            break;
        }case SMAAQualityLevel::Medium: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.1f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 8;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 0;
            break;
        }case SMAAQualityLevel::High: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.1f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 16;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 8;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 25;
            break;
        }case SMAAQualityLevel::Ultra: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.05f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 32;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 16;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 25;
            break;
        }
    }
}
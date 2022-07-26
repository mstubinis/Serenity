
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
#include <serenity/renderer/opengl/BindTextureRAII.h>

Engine::priv::SMAA Engine::priv::SMAA::STATIC_SMAA;

Engine::priv::SMAA::SMAA() {
    m_Vertex_Shaders.resize(PassStage::_TOTAL);
    m_Fragment_Shaders.resize(PassStage::_TOTAL);
    m_Shader_Programs.resize(PassStage::_TOTAL);

    m_Vertex_Shaders_Code.resize(PassStage::_TOTAL);
    m_Fragment_Shaders_Code.resize(PassStage::_TOTAL);
}
Engine::priv::SMAA::~SMAA() {
    glDeleteTextures(1, &SearchTexture);
    glDeleteTextures(1, &AreaTexture);
}
void Engine::priv::SMAA::internal_init_edge_vertex_code(const std::string& common) {
    STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Edge] = common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;

uniform vec4 SMAA_PIXEL_SIZE;
uniform mat4 Model;
uniform mat4 VP;

varying vec2 varUV;
varying vec4 varOffset[3];

void main() {
    varUV         = ViewportUVCalculation(UV);
    varOffset[0]  = mad(SMAA_PIXEL_SIZE.xyxy, vec4(-1.0, 0.0, 0.0, API_V_DIR(-1.0)), varUV.xyxy);
    varOffset[1]  = mad(SMAA_PIXEL_SIZE.xyxy, vec4( 1.0, 0.0, 0.0, API_V_DIR( 1.0)), varUV.xyxy);
    varOffset[2]  = mad(SMAA_PIXEL_SIZE.xyxy, vec4(-2.0, 0.0, 0.0, API_V_DIR(-2.0)), varUV.xyxy);
    gl_Position   = VP * Model * vec4(position, 1.0);
}
    )";
}
void Engine::priv::SMAA::internal_init_edge_fragment_code(const std::string& common) {
//uniform vec4 SMAAInfo1Floats;  //SMAA_THRESHOLD, SMAA_DEPTH_THRESHOLD, SMAA_LOCAL_CONTRAST_ADAPTATION_FACTOR, SMAA_PREDICATION_THRESHOLD
//uniform vec2 SMAAInfo1FloatsA; //SMAA_PREDICATION_SCALE, SMAA_PREDICATION_STRENGTH
    STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Edge] = common + R"(
uniform int SMAA_PREDICATION;
const vec2 comparison = vec2(1.0, 1.0);

uniform SAMPLER_TYPE_2D textureMap;
uniform SAMPLER_TYPE_2D texturePredication;

uniform vec4 SMAAInfo1Floats;
uniform vec2 SMAAInfo1FloatsA;

varying vec2 varUV;
varying vec4 varOffset[3];

vec3 SMAAGatherNeighbours(vec2 inUV, vec4 offset[3], sampler2D inTexture) {
    float P     = texture2D(USE_SAMPLER_2D(inTexture), inUV).r;
    float Pleft = texture2D(USE_SAMPLER_2D(inTexture), offset[0].xy).r;
    float Ptop  = texture2D(USE_SAMPLER_2D(inTexture), offset[0].zw).r;
    return vec3(P, Pleft, Ptop);
}
vec2 SMAACalculatePredicatedThreshold(vec2 inUV, vec4 offset[3], sampler2D inTexturePredication) {
    vec3 neighbours = SMAAGatherNeighbours(inUV, offset, inTexturePredication);
    vec2 delta      = abs(neighbours.xx - neighbours.yz);
    vec2 edges      = step(SMAAInfo1Floats.w, delta);
    return SMAAInfo1FloatsA.x * SMAAInfo1Floats.x * (1.0 - SMAAInfo1FloatsA.y * edges);
}
vec2 SMAADepthEdgeDetectionPS(vec2 inUV, vec4 offset[3], sampler2D depthTex) {
    vec3 neighbours = SMAAGatherNeighbours(inUV, offset, depthTex);
    vec2 delta      = abs(neighbours.xx - vec2(neighbours.y, neighbours.z));
    vec2 edges      = step(SMAAInfo1Floats.y, delta);
    if (dot(edges, comparison) == 0.0)
        discard;
    return edges;
}
vec2 SMAAColorEdgeDetectionPS(vec2 inUV, vec4 offset[3], sampler2D inColorTexture, sampler2D inTexturePredication) {
    vec2 threshold;
    if (SMAA_PREDICATION == 1) {
        threshold = SMAACalculatePredicatedThreshold(inUV, offset, inTexturePredication);
    } else {
        threshold = vec2(SMAAInfo1Floats.x, SMAAInfo1Floats.x);
    }
    vec4 delta;
    vec3 C     = texture2D(USE_SAMPLER_2D(inColorTexture), inUV).rgb;
    vec3 Cleft = texture2D(USE_SAMPLER_2D(inColorTexture), offset[0].xy).rgb;
    vec3 t     = abs(C - Cleft);
    delta.x    = max(max(t.r, t.g), t.b);
    vec3 Ctop  = texture2D(USE_SAMPLER_2D(inColorTexture), offset[0].zw).rgb;
    t          = abs(C - Ctop);
    delta.y    = max(max(t.r, t.g), t.b);
    vec2 edges = step(threshold, delta.xy);
    if (dot(edges, comparison) == 0.0)
        discard;
    vec3 Cright      = texture2D(USE_SAMPLER_2D(inColorTexture), offset[1].xy).rgb;
    t                = abs(C - Cright);
    delta.z          = max(max(t.r, t.g), t.b);
    vec3 Cbottom     = texture2D(USE_SAMPLER_2D(inColorTexture), offset[1].zw).rgb;
    t                = abs(C - Cbottom);
    delta.w          = max(max(t.r, t.g), t.b);
    vec2 maxDelta    = max(delta.xy, delta.zw);
    vec3 Cleftleft   = texture2D(USE_SAMPLER_2D(inColorTexture), offset[2].xy).rgb;
    t                = abs(C - Cleftleft);
    delta.z          = max(max(t.r, t.g), t.b);
    vec3 Ctoptop     = texture2D(USE_SAMPLER_2D(inColorTexture), offset[2].zw).rgb;
    t                = abs(C - Ctoptop);
    delta.w          = max(max(t.r, t.g), t.b);
    maxDelta         = max(maxDelta.xy, delta.zw);
    float finalDelta = max(maxDelta.x, maxDelta.y);
    edges.xy        *= step(finalDelta, SMAAInfo1Floats.z * delta.xy);
    return edges;
}
vec2 SMAALumaEdgeDetectionPS(vec2 inUV, vec4 offset[3], sampler2D inColorTexture, sampler2D inTexturePredication) {
    vec2 threshold;
    if (SMAA_PREDICATION == 1) {
        threshold = SMAACalculatePredicatedThreshold(inUV, offset, inTexturePredication);
    } else {
        threshold = vec2(SMAAInfo1Floats.x, SMAAInfo1Floats.x);
    }
    vec3 weights = vec3(0.2126, 0.7152, 0.0722);
    float L      = dot(texture2D(USE_SAMPLER_2D(inColorTexture), inUV).rgb,     weights);
    float Lleft  = dot(texture2D(USE_SAMPLER_2D(inColorTexture), offset[0].xy).rgb, weights);
    float Ltop   = dot(texture2D(USE_SAMPLER_2D(inColorTexture), offset[0].zw).rgb, weights);
    vec4 delta;
    delta.xy     = abs(L - vec2(Lleft, Ltop));
    vec2 edges   = step(threshold, delta.xy);
    if (dot(edges, comparison) == 0.0)
        discard;
    float Lright     = dot(texture2D(USE_SAMPLER_2D(inColorTexture), offset[1].xy).rgb, weights);
    float Lbottom    = dot(texture2D(USE_SAMPLER_2D(inColorTexture), offset[1].zw).rgb, weights);
    delta.zw         = abs(L - vec2(Lright, Lbottom));
    vec2 maxDelta    = max(delta.xy, delta.zw);
    float Lleftleft  = dot(texture2D(USE_SAMPLER_2D(inColorTexture), offset[2].xy).rgb, weights);
    float Ltoptop    = dot(texture2D(USE_SAMPLER_2D(inColorTexture), offset[2].zw).rgb, weights);
    delta.zw         = abs(vec2(Lleft, Ltop) - vec2(Lleftleft, Ltoptop));
    maxDelta         = max(maxDelta.xy, delta.zw);
    float finalDelta = max(maxDelta.x, maxDelta.y);
    edges.xy        *= step(finalDelta, SMAAInfo1Floats.z * delta.xy);
    return edges;
}
void main() {
    vec2 uvs = varUV;
    gl_FragColor = vec4(SMAAColorEdgeDetectionPS(uvs, varOffset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(texturePredication)), 0.0, 1.0);
//    gl_FragColor = vec4(SMAADepthEdgeDetectionPS(uvs, varOffset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(texturePredication)), 0.0, 1.0);
//    gl_FragColor = vec4(SMAALumaEdgeDetectionPS(uvs, varOffset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(texturePredication)), 0.0, 1.0);
}
)";
}
void Engine::priv::SMAA::internal_init_blend_vertex_code(const std::string& common) {
    STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Blend] = common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;

uniform mat4 Model;
uniform mat4 VP;
uniform vec4 SMAA_PIXEL_SIZE;
uniform int SMAA_MAX_SEARCH_STEPS;

varying vec2 varUV;
varying vec2 varPixCoord;
varying vec4 varOffset[3];

void main() {
    varUV         = ViewportUVCalculation(UV);
    varPixCoord   = varUV * SMAA_PIXEL_SIZE.zw;
    varOffset[0]  = mad(SMAA_PIXEL_SIZE.xyxy, vec4(-0.25, API_V_DIR(-0.125), 1.25,API_V_DIR(-0.125)), varUV.xyxy);
    varOffset[1]  = mad(SMAA_PIXEL_SIZE.xyxy, vec4(-0.125, API_V_DIR(-0.25), -0.125,API_V_DIR(1.25)), varUV.xyxy);
    varOffset[2]  = mad(SMAA_PIXEL_SIZE.xxyy, vec4(-2.0, 2.0, API_V_DIR(-2.0), API_V_DIR(2.0)) * float(SMAA_MAX_SEARCH_STEPS), vec4(varOffset[0].xz, varOffset[1].yw));
    gl_Position   = VP * Model * vec4(position, 1.0);
}
    )";
}
void Engine::priv::SMAA::internal_init_blend_fragment_code(const std::string& common) {
//uniform ivec4 SMAAInfo2Ints;  x = MAX_SEARCH_STEPS_DIAG, y = AREATEX_MAX_DISTANCE, z = AREATEX_MAX_DISTANCE_DIAG, w = CORNER_ROUNDING
//uniform vec4 SMAAInfo2Floats; x, y = SMAA_AREATEX_PIXEL_SIZE(x & y), z = SMAA_AREATEX_SUBTEX_SIZE, w = SMAA_CORNER_ROUNDING_NORM
    STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Blend] = common + R"(
uniform SAMPLER_TYPE_2D edge_tex;
uniform SAMPLER_TYPE_2D area_tex;
uniform SAMPLER_TYPE_2D search_tex;

varying vec2 varUV;
varying vec2 varPixCoord;
varying vec4 varOffset[3];

uniform ivec4 SMAAInfo2Ints;
uniform vec4 SMAAInfo2Floats;
uniform vec4 SMAA_PIXEL_SIZE;

float SMAASearchLength(sampler2D inSearchTexture, vec2 inE, float inOffset) {
    vec2 scale = vec2(66.0, 33.0) * vec2(0.5, -1.0);
    vec2 bias  = vec2(66.0, 33.0) * vec2(inOffset, 1.0);
    scale     += vec2(-1.0,  1.0);
    bias      += vec2( 0.5, -0.5);
    scale     *= 1.0 / vec2(64.0, 16.0);
    bias      *= 1.0 / vec2(64.0, 16.0);
    vec2 coord = mad(scale, inE, bias);
    coord.y    = API_V_COORD(coord.y);
    return texture2D(USE_SAMPLER_2D(inSearchTexture), coord).r;
}
float SMAASearchXLeft(sampler2D inEdgeTexture, sampler2D inSearchTexture, vec2 inUV, float inEnd) {
    vec2 e = vec2(0.0, 1.0);
    while (inUV.x > inEnd && e.g > 0.8281 && e.r == 0.0) {
        e    = texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV).rg;
        inUV = mad(-vec2(2.0, 0.0), SMAA_PIXEL_SIZE.xy, inUV);
    }
    float offset = mad(-255.0 / 127.0, SMAASearchLength(USE_SAMPLER_2D(inSearchTexture), e, 0.0), 3.25);
    return mad(SMAA_PIXEL_SIZE.x, offset, inUV.x);
}
float SMAASearchXRight(sampler2D inEdgeTexture, sampler2D inSearchTexture, vec2 inUV, float inEnd) {
    vec2 e = vec2(0.0, 1.0);
    while (inUV.x < inEnd && e.g > 0.8281 && e.r == 0.0) {
        e    = texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV).rg;
        inUV = mad(vec2(2.0, 0.0), SMAA_PIXEL_SIZE.xy, inUV);
    }
    float offset = mad(-255.0 / 127.0, SMAASearchLength(USE_SAMPLER_2D(inSearchTexture), e, 0.5), 3.25);
    return mad(-SMAA_PIXEL_SIZE.x, offset, inUV.x);
}
float SMAASearchYUp(sampler2D inEdgeTexture, sampler2D inSearchTexture, vec2 inUV, float inEnd) {
    vec2 e = vec2(1.0, 0.0);
    while (API_V_BELOW(inUV.y, inEnd) && e.r > 0.8281 && e.g == 0.0) {
        e    = texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV).rg;
        inUV = mad(-vec2(0.0, API_V_DIR(2.0)), SMAA_PIXEL_SIZE.xy, inUV);
    }
    float offset = mad(-255.0 / 127.0, SMAASearchLength(USE_SAMPLER_2D(inSearchTexture), e.gr, 0.0), 3.25);
    return mad(SMAA_PIXEL_SIZE.y, API_V_DIR(offset), inUV.y);
}
float SMAASearchYDown(sampler2D inEdgeTexture, sampler2D inSearchTexture, vec2 inUV, float inEnd) {
    vec2 e = vec2(1.0, 0.0);
    while (API_V_ABOVE(inUV.y, inEnd) && e.r > 0.8281 && e.g == 0.0) {
        e    = texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV).rg;
        inUV = mad(vec2(0.0, API_V_DIR(2.0)), SMAA_PIXEL_SIZE.xy, inUV);
    }
    float offset = mad(-255.0 / 127.0, SMAASearchLength(USE_SAMPLER_2D(inSearchTexture), e.gr, 0.5), 3.25);
    return mad(-SMAA_PIXEL_SIZE.y, API_V_DIR(offset), inUV.y);
}
vec2 SMAAAreaDiag(sampler2D inAreaTexture, vec2 inDist, vec2 inE, float inOffset) {
    vec2 uvs = mad(vec2(SMAAInfo2Ints.z, SMAAInfo2Ints.z), inE, inDist);
    uvs      = mad(SMAAInfo2Floats.xy, uvs, 0.5 * SMAAInfo2Floats.xy);
    uvs.x   += 0.5;
    uvs.y   += SMAAInfo2Floats.z * inOffset;
    uvs.y    = API_V_COORD(uvs.y);
    return texture2D(USE_SAMPLER_2D(inAreaTexture), uvs).rg;
}
vec2 SMAADecodeDiagBilinearAccess(vec2 inE) {
    inE.r = inE.r * abs(5.0 * inE.r - 5.0 * 0.75);
    return round(inE);
}
vec4 SMAADecodeDiagBilinearAccess(vec4 inE) {
    inE.rb = inE.rb * abs(5.0 * inE.rb - 5.0 * 0.75);
    return round(inE);
}
vec2 SMAASearchDiag1(sampler2D inEdgeTexture, vec2 inUV, vec2 inDir, out vec2 e) {
    inDir.y    = API_V_DIR(inDir.y);
    vec4 coord = vec4(inUV, -1.0, 1.0);
    vec3 t     = vec3(SMAA_PIXEL_SIZE.xy, 1.0);
    while (coord.z < float(SMAAInfo2Ints.x - 1) && coord.w > 0.9) {
        coord.xyz = mad(t, vec3(inDir, 1.0), coord.xyz);
        e         = texture2D(USE_SAMPLER_2D(inEdgeTexture), coord.xy).rg;
        coord.w   = dot(e, vec2(0.5));
    }
    return coord.zw;
}
vec2 SMAASearchDiag2(sampler2D inEdgeTexture, vec2 inUV, vec2 inDir, out vec2 e) {
    inDir.y    = API_V_DIR(inDir.y);
    vec4 coord = vec4(inUV, -1.0, 1.0);
    coord.x   += 0.25 * SMAA_PIXEL_SIZE.x;
    vec3 t     = vec3(SMAA_PIXEL_SIZE.xy, 1.0);
    while (coord.z < float(SMAAInfo2Ints.x - 1) && coord.w > 0.9) {
        coord.xyz = mad(t, vec3(inDir, 1.0), coord.xyz);
        e         = texture2D(USE_SAMPLER_2D(inEdgeTexture), coord.xy).rg;
        e         = SMAADecodeDiagBilinearAccess(e);
        coord.w   = dot(e, vec2(0.5));
    }
    return coord.zw;
}
vec2 SMAACalculateDiagWeights(sampler2D edgesTex, sampler2D areaTex, vec2 texcoord, vec2 e, vec4 subsampleIndices) {
    vec2 weights = vec2(0.0);
    vec4 d;
    vec2 end;
    if (e.r > 0.0) {
        d.xz = SMAASearchDiag1(edgesTex, texcoord, vec2(-1.0, 1.0), end);
        d.x += float(end.y > 0.9);
    } else {
        d.xz = vec2(0.0);
    }
    d.yw = SMAASearchDiag1(edgesTex, texcoord, vec2(1.0,-1.0), end);
    if (d.x + d.y > 2.0) {
        vec4 coords = mad(vec4(-d.x + 0.25, API_V_DIR(d.x), d.y, API_V_DIR(-d.y - 0.25)), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
        vec4 c;
        c.xy = texture2D(edgesTex, coords.xy + vec2(-1.0, 0.0) * SMAA_PIXEL_SIZE.xy).rg;
        c.zw = texture2D(edgesTex, coords.zw + vec2( 1.0, 0.0) * SMAA_PIXEL_SIZE.xy).rg;
        c.yxwz = SMAADecodeDiagBilinearAccess(c.xyzw);
        vec2 cc = mad(vec2(2.0), c.xz, c.yw);
        SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));
        weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.z);
    }
    d.xz = SMAASearchDiag2(edgesTex, texcoord, vec2(-1.0,-1.0), end);
    if (texture2D(edgesTex, texcoord + vec2(1.0, 0.0) * SMAA_PIXEL_SIZE.xy).r > 0.0) {
        d.yw = SMAASearchDiag2(edgesTex, texcoord, vec2(1.0), end);
        d.y += float(end.y > 0.9);
    } else {
        d.yw = vec2(0.0);
    }
    if (d.x + d.y > 2.0) {
        vec4 coords = mad(vec4(-d.x, API_V_DIR(-d.x), d.y, API_V_DIR(d.y)), SMAA_PIXEL_SIZE.xyxy, texcoord.xyxy);
        vec4 c;
        c.x  = texture2D(edgesTex, coords.xy + vec2(-1.0, 0.0) * SMAA_PIXEL_SIZE.xy).g;
        c.y  = texture2D(edgesTex, coords.xy + vec2( 0.0, API_V_DIR(-1.0)) * SMAA_PIXEL_SIZE.xy).r;
        c.zw = texture2D(edgesTex, coords.zw + vec2( 1.0, 0.0) * SMAA_PIXEL_SIZE.xy).gr;
        vec2 cc = mad(vec2(2.0), c.xz, c.yw);
        SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0));
        weights += SMAAAreaDiag(areaTex, d.xy, cc, subsampleIndices.w).gr;
    }
    return weights;
}
vec2 SMAAArea(sampler2D inAreaTexture, vec2 inDist, float inE1, float inE2, float offset) {
    vec2 uv = mad(vec2(SMAAInfo2Ints.y, SMAAInfo2Ints.y), round(4.0 * vec2(inE1, inE2)), inDist);
    uv      = mad(SMAAInfo2Floats.xy, uv, 0.5 * SMAAInfo2Floats.xy);
    uv.y    = mad(SMAAInfo2Floats.z, offset, uv.y);
    uv.y    = API_V_COORD(uv.y);
    return texture2D(USE_SAMPLER_2D(inAreaTexture), uv).rg;
}
void SMAADetectHorizontalCornerPattern(sampler2D inEdgeTexture, inout vec2 inWeights, vec4 inUV, vec2 inD) {
    if(SMAAInfo2Ints.w == 0) {
        return;
    }
    vec2 leftRight = step(inD.xy, inD.yx);
    vec2 rounding  = (1.0 - SMAAInfo2Floats.w) * leftRight;
    rounding      /= leftRight.x + leftRight.y;
    vec2 factor    = vec2(1.0, 1.0);
    factor.x      -= rounding.x * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.xy + vec2(0.0, API_V_DIR(1.0))  * SMAA_PIXEL_SIZE.xy).r;
    factor.x      -= rounding.y * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.zw + vec2(1.0, API_V_DIR(1.0))  * SMAA_PIXEL_SIZE.xy).r;
    factor.y      -= rounding.x * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.xy + vec2(0.0, API_V_DIR(-2.0)) * SMAA_PIXEL_SIZE.xy).r;
    factor.y      -= rounding.y * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.zw + vec2(1.0, API_V_DIR(-2.0)) * SMAA_PIXEL_SIZE.xy).r;
    inWeights     *= saturate(factor);
}
void SMAADetectVerticalCornerPattern(sampler2D inEdgeTexture, inout vec2 inWeights, vec4 inUV, vec2 inD) {
    if(SMAAInfo2Ints.w == 0) {
        return;
    }
    vec2 leftRight = step(inD.xy, inD.yx);
    vec2 rounding  = (1.0 - SMAAInfo2Floats.w) * leftRight;
    rounding      /= leftRight.x + leftRight.y;
    vec2 factor    = vec2(1.0, 1.0);
    factor.x      -= rounding.x * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.xy + vec2( 1.0, 0.0)            * SMAA_PIXEL_SIZE.xy).g;
    factor.x      -= rounding.y * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.zw + vec2( 1.0, API_V_DIR(1.0)) * SMAA_PIXEL_SIZE.xy).g;
    factor.y      -= rounding.x * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.xy + vec2(-2.0, 0.0)            * SMAA_PIXEL_SIZE.xy).g;
    factor.y      -= rounding.y * texture2D(USE_SAMPLER_2D(inEdgeTexture), inUV.zw + vec2(-2.0, API_V_DIR(1.0)) * SMAA_PIXEL_SIZE.xy).g;
    inWeights     *= saturate(factor);
}
vec4 SMAABlendingWeightCalculationPS(vec2 inUV, vec2 inPixcoord, vec4 inOffset[3], sampler2D inEdgesTex, sampler2D inAreaTex, sampler2D inSearchTex, vec4 inSubsampleIndices) {
    vec4 weights = vec4(0.0);
    vec2 e       = texture2D(USE_SAMPLER_2D(inEdgesTex), inUV).rg;
    if (e.g > 0.0) {
        if (SMAAInfo2Ints.x > 0) {
            weights.rg = SMAACalculateDiagWeights(USE_SAMPLER_2D(inEdgesTex), USE_SAMPLER_2D(inAreaTex), inUV, e, inSubsampleIndices);
            if (weights.r == -weights.g) {
                vec2 d;
                vec3 coords;
                coords.x    = SMAASearchXLeft(USE_SAMPLER_2D(inEdgesTex), USE_SAMPLER_2D(inSearchTex), inOffset[0].xy, inOffset[2].x);
                coords.y    = inOffset[1].y;
                d.x         = coords.x;
                float e1    = texture2D(inEdgesTex, coords.xy).r;
                coords.z    = SMAASearchXRight(USE_SAMPLER_2D(inEdgesTex), USE_SAMPLER_2D(inSearchTex), inOffset[0].zw, inOffset[2].y);
                d.y         = coords.z;
                d           = abs(round(mad(SMAA_PIXEL_SIZE.zz, d, -inPixcoord.xx)));
                vec2 sqrt_d = sqrt(d);
                float e2    = texture2D(USE_SAMPLER_2D(inEdgesTex), coords.zy + vec2(1.0, 0.0) * SMAA_PIXEL_SIZE.xy).r;
                weights.rg  = SMAAArea(USE_SAMPLER_2D(inAreaTex), sqrt_d, e1, e2, inSubsampleIndices.y);
                coords.y    = inUV.y;
                SMAADetectHorizontalCornerPattern(USE_SAMPLER_2D(inEdgesTex), weights.rg, coords.xyzy, d);
            } else { 
                e.r = 0.0; 
            }
        } else {
           vec2 d;
           vec3 coords;
           coords.x    = SMAASearchXLeft(inEdgesTex, inSearchTex, inOffset[0].xy, inOffset[2].x);
           coords.y    = inOffset[1].y;
           d.x         = coords.x;
           float e1    = texture2D(inEdgesTex, coords.xy).r;
           coords.z    = SMAASearchXRight(inEdgesTex, inSearchTex, inOffset[0].zw, inOffset[2].y);
           d.y         = coords.z;
           d           = abs(round(mad(SMAA_PIXEL_SIZE.zz, d, -inPixcoord.xx)));
           vec2 sqrt_d = sqrt(d);
           float e2    = texture2D(inEdgesTex, coords.zy + vec2(1.0, 0.0) * SMAA_PIXEL_SIZE.xy).r;
           weights.rg  = SMAAArea(inAreaTex, sqrt_d, e1, e2, inSubsampleIndices.y);
           coords.y    = inUV.y;
           SMAADetectHorizontalCornerPattern(inEdgesTex, weights.rg, coords.xyzy, d);
        }
    }
    if (e.r > 0.0) {
        vec2 d;
        vec3 coords;
        coords.y    = SMAASearchYUp(inEdgesTex, inSearchTex, inOffset[1].xy, inOffset[2].z);
        coords.x    = inOffset[0].x;
        d.x         = coords.y;
        float e1    = texture2D(inEdgesTex, coords.xy).g;
        coords.z    = SMAASearchYDown(inEdgesTex, inSearchTex, inOffset[1].zw, inOffset[2].w);
        d.y         = coords.z;
        d           = abs(round(mad(SMAA_PIXEL_SIZE.ww, d, -inPixcoord.yy)));
        vec2 sqrt_d = sqrt(d);
        float e2    = texture2D(inEdgesTex, coords.xz + vec2(0.0, API_V_DIR(1.0)) * SMAA_PIXEL_SIZE.xy).g;
        weights.ba  = SMAAArea(inAreaTex, sqrt_d, e1, e2, inSubsampleIndices.x);
        coords.x    = inUV.x;
        SMAADetectVerticalCornerPattern(inEdgesTex, weights.ba, coords.xyxz, d);
    }
    return weights;
}
void main() {
    vec4 subSamples = vec4(0.0);
    vec2 uvs = varUV;
    gl_FragColor = SMAABlendingWeightCalculationPS(uvs, varPixCoord, varOffset, USE_SAMPLER_2D(edge_tex), USE_SAMPLER_2D(area_tex), USE_SAMPLER_2D(search_tex), subSamples);
}
)";
}
void Engine::priv::SMAA::internal_init_neighbor_vertex_code(const std::string& common) {
    STATIC_SMAA.m_Vertex_Shaders_Code[PassStage::Neighbor] = common + R"(
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 UV;

uniform mat4 Model;
uniform mat4 VP;
uniform vec4 SMAA_PIXEL_SIZE;

varying vec2 varUV;
varying vec4 varOffset;

void main() {
    varUV       = ViewportUVCalculation(UV);
    varOffset   = mad(SMAA_PIXEL_SIZE.xyxy, vec4(1.0, 0.0, 0.0, API_V_DIR(1.0)), varUV.xyxy);
    gl_Position = VP * Model * vec4(position, 1.0);
}
    )";
}
void Engine::priv::SMAA::internal_init_neighbor_fragment_code(const std::string& common) {
    STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Neighbor] = common + R"(
uniform SAMPLER_TYPE_2D textureMap;
uniform SAMPLER_TYPE_2D blend_tex;
uniform vec4 SMAA_PIXEL_SIZE;

varying vec2 varUV;
varying vec4 varOffset;

vec4 SMAANeighborhoodBlendingPS(vec2 inUV, vec4 inOffset, sampler2D inColorTexture, sampler2D inBlendTexture) {
    vec4 a;
    a.x  = texture2D(USE_SAMPLER_2D(inBlendTexture), inOffset.xy).a;
    a.y  = texture2D(USE_SAMPLER_2D(inBlendTexture), inOffset.zw).g;
    a.wz = texture2D(USE_SAMPLER_2D(inBlendTexture), inUV).xz;
    if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) <= 1e-5) {
        vec4 color = texture2DLod(USE_SAMPLER_2D(inColorTexture), inUV, 0.0);
        return color;
    } else {
        bool h = max(a.x, a.z) > max(a.y, a.w);
        vec4 blendingOffset = vec4(0.0, API_V_DIR(a.y), 0.0, API_V_DIR(a.w));
        vec2 blendingWeight = a.yw;
        SMAAMovc(bvec4(h, h, h, h), blendingOffset, vec4(a.x, 0.0, a.z, 0.0));
        SMAAMovc(bvec2(h, h), blendingWeight, a.xz);
        blendingWeight     /= dot(blendingWeight, vec2(1.0, 1.0));
        vec4 blendingCoord  = mad(blendingOffset, vec4(SMAA_PIXEL_SIZE.xy, -SMAA_PIXEL_SIZE.xy), inUV.xyxy);
        vec4 color          = blendingWeight.x * texture2D(USE_SAMPLER_2D(inColorTexture), blendingCoord.xy);
        color              += blendingWeight.y * texture2D(USE_SAMPLER_2D(inColorTexture), blendingCoord.zw);
        return color;
    }
}
void main() {
    vec2 uvs = varUV;
    gl_FragColor = SMAANeighborhoodBlendingPS(uvs, varOffset, USE_SAMPLER_2D(textureMap), USE_SAMPLER_2D(blend_tex));
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

varying vec2 varUV;

void SMAAResolveVS(inout vec2 inUV) {}
void SMAASeparateVS(inout vec2 inUV) {}

void main() {
    varUV       = UV;
    gl_Position = VP * Model * vec4(position, 1.0);
}
    )";
#pragma endregion

#pragma region FragFinal
        STATIC_SMAA.m_Fragment_Shaders_Code[PassStage::Final] = smaa_common + R"(
varying vec2 varUV;
vec4 SMAAResolvePS(vec2 inUV, sampler2D inCurrentColorTex, sampler2D inPreviousColorTex) {
    vec4 current  = texture2D(USE_SAMPLER_2D(inCurrentColorTex), inUV);
    vec4 previous = texture2D(USE_SAMPLER_2D(inPreviousColorTex), inUV);
    return mix(current, previous, 0.5);
}
void main() {
    gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
    )";
#pragma endregion

    for (uint32_t i = 0; i < PassStage::_TOTAL; ++i) {
        auto lambda_part_a = [i]() {
            STATIC_SMAA.m_Vertex_Shaders[i]   = Engine::Resources::addResource<Shader>(STATIC_SMAA.m_Vertex_Shaders_Code[i], ShaderType::Vertex);
            STATIC_SMAA.m_Fragment_Shaders[i] = Engine::Resources::addResource<Shader>(STATIC_SMAA.m_Fragment_Shaders_Code[i], ShaderType::Fragment);
        };
        auto lambda_part_b = [i]() {
            STATIC_SMAA.m_Shader_Programs[i]  = Engine::Resources::addResource<ShaderProgram>("SMAA_" + std::to_string(i), STATIC_SMAA.m_Vertex_Shaders[i], STATIC_SMAA.m_Fragment_Shaders[i]);
        };
        Engine::priv::threading::addJobWithPostCallback(lambda_part_a, lambda_part_b);
    }

    const TextureType textureType = TextureType::Texture2D;
    Engine::Renderer::genAndBindTexture(textureType, STATIC_SMAA.AreaTexture);
    glTexImage2D(textureType.toGLType(), 0, GL_RG8, 160, 560, 0, GL_RG, GL_UNSIGNED_BYTE, Engine::priv::SMAA_AreaTextureBytes);
    Texture::setFilter(textureType, TextureFilter::Linear);
    TextureBaseClass::setWrapping(textureType, TextureWrap::ClampToBorder);

    Engine::Renderer::genAndBindTexture(textureType, STATIC_SMAA.SearchTexture);
    glTexImage2D(textureType.toGLType(), 0, GL_R8, 64, 16, 0, GL_RED, GL_UNSIGNED_BYTE, Engine::priv::SMAA_SearchTextureBytes);
    Texture::setFilter(textureType, TextureFilter::Linear);
    TextureBaseClass::setWrapping(textureType, TextureWrap::ClampToBorder);

    return true;
}

constexpr uint32_t TEXTURE_INTERMEDIARY = Engine::priv::GBufferType::Normal;

void Engine::priv::SMAA::passEdge(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t sceneTexture, uint32_t outTexture, const Engine::priv::RenderModule& renderer) {
    gbuffer.bindFramebuffers({ outTexture }, "RGBA"); //probably the lighting buffer
    renderer.bind(m_Shader_Programs[PassStage::Edge].get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::Settings::clear(true, false, true);//lighting rgba, stencil is completely filled with 0's

    Engine::Renderer::GLEnable(GL_STENCIL_TEST);
    Engine::Renderer::stencilMask(0b1111'1111);

    //fragments written during the edge test always pass. the edge fragment pass will discard bad pixels, leaving their stencil value in the buffer at zero.
    Engine::Renderer::stencilFunc(GL_ALWAYS, 0b1111'1111, 0b1111'1111);

    Engine::Renderer::stencilOp(GL_KEEP, GL_INCR, GL_INCR);
    
    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::Renderer::sendUniform4Safe("SMAAInfo1Floats", THRESHOLD, DEPTH_THRESHOLD, LOCAL_CONTRAST_ADAPTATION_FACTOR, PREDICATION_THRESHOLD);
    Engine::Renderer::sendUniform2Safe("SMAAInfo1FloatsA", PREDICATION_SCALE, PREDICATION_STRENGTH);

    Engine::Renderer::sendUniform1Safe("SMAA_PREDICATION", int(PREDICATION));

    Engine::priv::OpenGLBindTextureRAII textureMap{ "textureMap", gbuffer.getTexture(sceneTexture), 0, false };
    Engine::priv::OpenGLBindTextureRAII texturePredication{ "texturePredication", gbuffer.getTexture(GBufferType::Diffuse), 1, true };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);

    Engine::Renderer::stencilMask(0b1111'1111);
    //only if the stored stencil value is 0b00000001 will the fragments be evaluated
    Engine::Renderer::stencilFunc(GL_EQUAL, 0b0000'0001, 0b1111'1111);
    Engine::Renderer::stencilOp(GL_KEEP, GL_KEEP, GL_KEEP); //Do not change stencil
}
void Engine::priv::SMAA::passBlend(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t outTexture, const Engine::priv::RenderModule& renderer) {
    gbuffer.bindFramebuffers({ TEXTURE_INTERMEDIARY }, "RGBA");
    Engine::Renderer::Settings::clear(true, false, false); //clear color only
    const auto& viewportDimensions = viewport.getViewportDimensions();
    renderer.bind(m_Shader_Programs[PassStage::Blend].get<ShaderProgram>());

    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::priv::OpenGLBindTextureRAII edge_tex{ "edge_tex", gbuffer.getTexture(outTexture), 0, false };
    Engine::priv::OpenGLBindTextureRAII area_tex{ "area_tex", AreaTexture, GL_TEXTURE_2D, 1, false };
    Engine::priv::OpenGLBindTextureRAII search_tex{ "search_tex", SearchTexture, GL_TEXTURE_2D, 2, false };

    Engine::Renderer::sendUniform1Safe("SMAA_MAX_SEARCH_STEPS", MAX_SEARCH_STEPS);

    Engine::Renderer::sendUniform4Safe("SMAAInfo2Ints", MAX_SEARCH_STEPS_DIAG, AREATEX_MAX_DISTANCE, AREATEX_MAX_DISTANCE_DIAG, CORNER_ROUNDING);
    Engine::Renderer::sendUniform4Safe("SMAAInfo2Floats", AREATEX_PIXEL_SIZE.x, AREATEX_PIXEL_SIZE.y, AREATEX_SUBTEX_SIZE, (float)CORNER_ROUNDING / 100.0f);

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);

    Engine::Renderer::GLDisable(GL_STENCIL_TEST);
}
void Engine::priv::SMAA::passNeighbor(Engine::priv::GBuffer& gbuffer, const glm::vec4& PIXEL_SIZE, const Viewport& viewport, uint32_t sceneTexture, const Engine::priv::RenderModule& renderer) {
    renderer.bind(m_Shader_Programs[PassStage::Neighbor].get<ShaderProgram>());
    const auto& viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::sendUniform4("SMAA_PIXEL_SIZE", PIXEL_SIZE);

    Engine::priv::OpenGLBindTextureRAII textureMap{ "textureMap", gbuffer.getTexture(sceneTexture), 0, true };
    Engine::priv::OpenGLBindTextureRAII blend_tex{ "blend_tex", gbuffer.getTexture(TEXTURE_INTERMEDIARY), 1, true };

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
}
void Engine::priv::SMAA::passFinal(Engine::priv::GBuffer& gbuffer, const Viewport& viewport, const Engine::priv::RenderModule& renderer) {
    /*
    //this pass is optional. lets skip it for now
    //gbuffer.bindFramebuffers(GBufferType::Lighting);
    const auto& viewportDimensions = viewport.getViewportDimensions();
    gbuffer.stop();

    renderer.bind(m_Shader_Programs[PassStage::Final]);

    Engine::Renderer::renderFullscreenQuadCentered(viewportDimensions.z, viewportDimensions.w);
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
        } case SMAAQualityLevel::Medium: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.1f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 8;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 0;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 0;
            break;
        } case SMAAQualityLevel::High: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.1f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 16;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 8;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 25;
            break;
        } case SMAAQualityLevel::Ultra: {
            Engine::priv::SMAA::STATIC_SMAA.THRESHOLD = 0.05f;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS = 32;
            Engine::priv::SMAA::STATIC_SMAA.MAX_SEARCH_STEPS_DIAG = 16;
            Engine::priv::SMAA::STATIC_SMAA.CORNER_ROUNDING = 25;
            break;
        }
    }
}
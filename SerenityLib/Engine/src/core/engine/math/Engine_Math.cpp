#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>

#include <btBulletDynamicsCommon.h>
#include <boost/math/interpolators/cubic_b_spline.hpp>

using namespace Engine;
using namespace std;

constexpr float ROTATION_THRESHOLD = 0.00001f;

glm_vec3 Math::rotate_vec3(const glm_quat& rotation, const glm_vec3& vec) {
    return rotation * vec;
}

//could use some fixing
glm::vec3 Math::polynomial_interpolate_linear(vector<glm::vec3>& points, float time) {
    glm::vec3 ret = glm::vec3(0.0f);
    auto n = points.size();
    assert(n >= 3);

    //first get the 3 points we need to work with based on time (0 <= time <= 1)
    glm::vec3 P0;
    glm::vec3 P1;
    glm::vec3 P2;
    if (n == 3) {
        P0 = points[0];
        P1 = points[1];
        P2 = points[2];
    }else{
        float indexFloat = glm::max(0.01f, (n * time) - 1.0f);
        size_t index1 = static_cast<size_t>(glm::floor(static_cast<size_t>(indexFloat)));
        size_t index2;
        size_t index3;
        if (index1 >= n - 2) {
            index1 = n - 3;
            index2 = n - 2;
            index3 = n - 1;
        }else{
            index2 = index1 + 1;
            index3 = index1 + 2;
        }
        P0 = points[index1];
        P1 = points[index2];
        P2 = points[index3];
    }
    //ok we have the three points
    auto timeSquared = time * time;
    auto a2 = (P1 - P0) - time * (P2 - P0) / time * (time - 1.0f);
    auto a1 = P2 - P0 - a2;
    auto a0 = P0;
    ret = a2 * timeSquared + a1 * time + a0;
    return ret;
}
//this works perfectly
glm::vec3 Math::polynomial_interpolate_cubic(vector<glm::vec3>& points, float time) {
    glm::vec3 ret = glm::vec3(0.0f);
    auto n = points.size();
    vector<float> xs; xs.reserve(n);
    vector<float> ys; ys.reserve(n);
    vector<float> zs; zs.reserve(n);
    for (auto& pt : points) {
        xs.push_back(pt.x);
        ys.push_back(pt.y);
        zs.push_back(pt.z);
    }
    auto step = 1.0f / static_cast<float>(n-1);
    boost::math::cubic_b_spline<float> x_spline(xs.data(), n, 0.0f, step);
    boost::math::cubic_b_spline<float> y_spline(ys.data(), n, 0.0f, step);
    boost::math::cubic_b_spline<float> z_spline(zs.data(), n, 0.0f, step);

    return glm::vec3(x_spline(time), y_spline(time), z_spline(time));
}


void Math::Float32From16(float* out, const uint16_t in) {
    //#if defined(ENGINE_SIMD_SUPPORTED) && !defined(_DEBUG)
    //    _mm256_storeu_ps(out, _mm256_cvtph_ps(_mm_loadu_si128((__m128i*)&in)));
    //#else
        uint32_t t1 = in & 0x7fff;              // Non-sign bits
        uint32_t t2 = in & 0x8000;              // Sign bit
        uint32_t t3 = in & 0x7c00;              // Exponent
        t1 <<= 13;                              // Align mantissa on MSB
        t2 <<= 16;                              // Shift sign bit into position
        t1 += 0x38000000;                       // Adjust bias
        t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero
        t1 |= t2;                               // Re-insert sign bit
        *(uint32_t*)out = t1;
    //#endif
}
void Math::Float16From32(uint16_t* out, const float in) {
    //#if defined(ENGINE_SIMD_SUPPORTED) && !defined(_DEBUG)
    //    _mm_storeu_si128((__m128i*)out, _mm256_cvtps_ph(_mm256_loadu_ps(&in), 0));
        //__m128  V1 = _mm_set_ss(in);
        //__m128i V2 = _mm_cvtps_ph(V1, 0);
        //(*out) = static_cast<uint16_t>(_mm_cvtsi128_si32(V2));
    //#else
        uint32_t inu = *((uint32_t*)& in);
        uint32_t t1 = inu & 0x7fffffff;        // Non-sign bits
        uint32_t t2 = inu & 0x80000000;        // Sign bit
        uint32_t t3 = inu & 0x7f800000;        // Exponent
        t1 >>= 13;                             // Align mantissa on MSB
        t2 >>= 16;                             // Shift sign bit into position
        t1 -= 0x1c000;                         // Adjust bias
        t1 = (t3 < 0x38800000) ? 0 : t1;
        t1 = (t3 > 0x47000000) ? 0x7bff : t1;
        t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero
        t1 |= t2;                              // Re-insert sign bit
        *(uint16_t*)out = t1;
    //#endif
}


void Math::Float32From16(float*    out, const uint16_t* in, const uint arraySize) {
    for (unsigned i = 0; i < arraySize; ++i) {
        Math::Float32From16(&(out[i]), in[i]);
    }
}
void Math::Float16From32(uint16_t* out, const float*    in, const uint arraySize) {
    for (uint i = 0; i < arraySize; ++i) {
        Math::Float16From32(&(out[i]), in[i]);
    }
}
void Math::setFinalModelMatrix(glm_mat4& modelMatrix, const glm_vec3& position, const glm_quat& rotation, const glm_vec3& inScale) {
    modelMatrix            = glm_mat4(1.0);
    auto translationMatrix = glm::translate(position);
    auto rotationMatrix    = glm::mat4_cast(rotation);
    auto scaleMatrix       = glm::scale(inScale);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}
void Math::setFinalModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& inScale) {
    modelMatrix            = glm::mat4(1.0f);
    auto translationMatrix = glm::translate(position);
    auto rotationMatrix    = glm::mat4_cast(rotation);
    auto scaleMatrix       = glm::scale(inScale);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}
void Math::setRotation(glm_quat& orientation, decimal pitch, decimal yaw, decimal roll) {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation               = (glm::angleAxis(pitch, glm_vec3(-1, 0, 0)));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm_vec3(0, -1, 0)));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,   glm_vec3(0, 0, 1)));   //roll
}
void Math::setRotation(glm::quat& orientation, float pitch, float yaw, float roll) {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation               = (glm::angleAxis(pitch, glm::vec3(-1, 0, 0)));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw, glm::vec3(0, -1, 0)));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));   //roll
}
void Math::rotate(glm_quat& orientation, decimal pitch, decimal yaw, decimal roll) {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(pitch, glm_vec3(-1, 0, 0)));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm_vec3(0, -1, 0)));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,   glm_vec3(0, 0, 1)));   //roll
}
void Math::rotate(glm::quat& orientation, float pitch, float yaw, float roll) {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(pitch, glm::vec3(-1, 0, 0)));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw, glm::vec3(0, -1, 0)));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll, glm::vec3(0, 0, 1)));   //roll
}
glm::vec2 Math::rotate2DPoint(const glm::vec2& point, float angle, const glm::vec2& origin) {
    float s = glm::sin(angle);
    float c = glm::cos(angle);
    glm::vec2 ret;
    ret.x = c * (point.x - origin.x) - s * (point.y - origin.y) + origin.x;
    ret.y = s * (point.x - origin.x) + c * (point.y - origin.y) + origin.x;
    return ret;
}



void Math::extractViewFrustumPlanesHartmannGribbs(const glm::mat4& inViewProjection,glm::vec4* outPlanes){
    glm::vec4 rows[4];
    for(unsigned char i = 0; i < 4; ++i)
        rows[i] = glm::row(inViewProjection,i);
    //0 = left, 1 = right, 2 = top, 3 = bottom, 4 = near, 5 = far
    for(unsigned char i = 0; i < 3; ++i){
        unsigned char index = i * 2;
        outPlanes[index  ] = -(rows[3] + rows[i]);  //0,2,4
        outPlanes[index+1] = -(rows[3] - rows[i]);  //1,3,5
    }
    //https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
}
glm_quat Math::btToGLMQuat(const btQuaternion& q){
	return glm_quat(q.getW(),q.getX(),q.getY(),q.getZ()); 
}
btQuaternion Math::glmToBTQuat(const glm_quat& q){
	return btQuaternion(q.x,q.y,q.z,q.w); 
}
glm::vec3 Math::assimpToGLMVec3(const aiVector3D& n){
	return glm::vec3(n.x,n.y,n.z); 
}
glm::mat4 Math::assimpToGLMMat4(const aiMatrix4x4& n){
	return glm::mat4(n.a1,n.b1,n.c1,n.d1,n.a2,n.b2,n.c2,n.d2,n.a3,n.b3,n.c3,n.d3,n.a4,n.b4,n.c4,n.d4); 
}
glm::mat3 Math::assimpToGLMMat3(const aiMatrix3x3& n){
	return glm::mat3(n.a1,n.b1,n.c1,n.a2,n.b2,n.c2,n.a3,n.b3,n.c3);  
}

float Math::toRadians(float degrees){
	return degrees * 0.0174533f; 
}
float Math::toDegrees(float radians){
	return radians * 57.2958f; 
}
float Math::toRadians(double degrees){
	return Math::toRadians(static_cast<float>(degrees)); 
}
float Math::toDegrees(double radians){
	return Math::toDegrees(static_cast<float>(radians));
}
float Math::remainder(float x, float y){
	return x - (glm::round(x / y) * y);
}

glm_vec3 Math::btVectorToGLM(const btVector3& btvector){
	return glm_vec3(btvector.x(), btvector.y(), btvector.z());
}
btVector3 Math::btVectorFromGLM(const glm_vec3& vector){ 
	return btVector3(vector.x, vector.y, vector.z);
}
glm_vec3 Math::getMatrixPosition(const glm_mat4& matrix) {
    return glm_vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
}
glm::vec3 Math::getMatrixPosition(const glm::mat4& matrix) {
    return glm::vec3(matrix[3][0], matrix[3][1], matrix[3][2]);
}
void Math::removeMatrixPosition(glm::mat4& matrix){
	matrix[3][0] = 0.0f;
	matrix[3][1] = 0.0f;
	matrix[3][2] = 0.0f;
}

bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector, const glm::vec3& point, float fovRadians){
    glm::vec3 diff = glm::normalize(point - conePos);
    float t = glm::dot(coneVector, diff);
    return ( t >= glm::cos( fovRadians ) );
}
bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector, const glm::vec3& point, float fovRadians, float maxDistance){
    glm::vec3 diff = glm::normalize(point - conePos);
    float t = glm::dot(coneVector, diff);
    float length = glm::length(point - conePos);
    return (length > maxDistance) ? false : (t >= glm::cos(fovRadians));
}
glm::vec3 Math::getScreenCoordinates(const glm::vec3& position, const Camera& camera, const glm::mat4& view, const glm::mat4& projection, const glm::vec4& viewport, bool clampToEdge){
    using v2 = glm::vec2;
    auto getIntersection = [&](const v2& a1, const v2& a2, const v2& b1, const v2& b2){
        auto b = a2 - a1;
        auto d = b2 - b1;
        auto bDotDPerp = b.x * d.y - b.y * d.x;

        // if b dot d == 0, it means the lines are parallel so have infinite intersection points
        if (bDotDPerp == 0) {
            return glm::vec3(-1.0f);
        }
        auto c = b1 - a1;
        auto t = (c.x * d.y - c.y * d.x) / bDotDPerp;
        if (t < 0 || t > 1){
            return glm::vec3(-1.0f);
        }
        auto u = (c.x * b.y - c.y * b.x) / bDotDPerp;
        if (u < 0 || u > 1){
            return glm::vec3(-1.0f);
        }
        auto res = a1 + t * b;
        return glm::vec3(res.x, res.y, 1.0f);
    };
    auto getPerimiterIntersection = [&](const v2& p1, const v2& p2, const v2& r1, const v2& r2, const v2& r3, const v2& r4) {
        auto intersection = getIntersection(p1, p2, r1, r2);
        if (intersection.z > 0) {
            return v2(intersection.x, intersection.y); //left
        }
        intersection = getIntersection(p1, p2, r2, r3);
        if (intersection.z > 0) {
            return v2(intersection.x, intersection.y); //top
        }
        intersection = getIntersection(p1, p2, r3, r4);
        if (intersection.z > 0) {
            return v2(intersection.x, intersection.y); //right
        }
        intersection = getIntersection(p1, p2, r4, r1);
        if (intersection.z > 0) {
            return v2(intersection.x, intersection.y); //bottom
        }
        return v2(intersection.x, intersection.y);
    };
    unsigned int inBounds = 0;

    auto screen_pos       = glm::project(position, view, projection, viewport);
    float dot       = glm::dot(camera.getViewVector(), position - glm::vec3(camera.getPosition()));
    if (screen_pos.x >= viewport.x && screen_pos.x <= viewport.z) {
        if (screen_pos.y >= viewport.y && screen_pos.y <= viewport.w) {
            if (dot < 0.0f) { //negative dot means infront
                inBounds = 1;
            }
        }
    }
    if (inBounds) {
        return glm::vec3(screen_pos.x, screen_pos.y, inBounds);
    }
    if (clampToEdge) {
        v2 center(viewport.z / 2.0f, viewport.w / 2.0f);
        glm::vec3 res(0.0f, 0.0f, inBounds);
        v2 perm;
        if (dot >= 0.0f) {
            //reflect screen_pos along center
            auto xDiff = screen_pos.x - center.x;
            auto yDiff = screen_pos.y - center.y;
            v2 reflected(center.x - xDiff, center.y - yDiff);

            auto norm = reflected - center;
            norm = glm::normalize(norm);
            norm *= 9999999.0f;

            perm = getPerimiterIntersection(center, center + norm, v2(0, 0), v2(0, 0 + viewport.w), v2(0 + viewport.z, 0 + viewport.w), v2(0 + viewport.z, 0));
        }else{
            perm = getPerimiterIntersection(center, v2(screen_pos.x, screen_pos.y), v2(0, 0), v2(0, 0 + viewport.w), v2(0 + viewport.z, 0 + viewport.w), v2(0 + viewport.z, 0));
        }
        res.x = perm.x;
        res.y = perm.y;
        return res;
    }
    return glm::vec3(screen_pos.x, screen_pos.y, inBounds);
}
glm::vec3 Math::getScreenCoordinates(const glm::vec3& position, const Camera& camera, const glm::vec4& viewport, bool clampToEdge) {
    return Math::getScreenCoordinates(position, camera, camera.getView(), camera.getProjection(), viewport, clampToEdge);
}
glm::vec3 Math::getScreenCoordinates(const glm::vec3& objPos, const Camera& camera, bool clampToEdge) {
    glm::vec2 winSize  = glm::vec2(Resources::getWindowSize());
    glm::vec4 viewport = glm::vec4(0.0f, 0.0f, winSize.x, winSize.y);
    return Math::getScreenCoordinates(objPos, camera, viewport, clampToEdge);
}
float Math::Max(const glm::vec2& v){
	return glm::max(v.x,v.y); 
}
float Math::Max(const glm::vec3& v){
	return glm::max(v.x,glm::max(v.y,v.z)); 
}
float Math::Max(const glm::vec4& v){
	return glm::max(v.x,glm::max(v.y,glm::max(v.z,v.w))); 
}
float Math::Max(float x, float y){
	return glm::max(x,y); 
}
float Math::Max(float x, float y, float z){
	return glm::max(x,glm::max(y,z)); 
}
float Math::Max(float x, float y, float z, float w){
	return glm::max(x,glm::max(y,glm::max(z,w))); 
}
uint Math::Max(uint x, uint y){
	return glm::max(x,y); 
}
uint Math::Max(uint x, uint y, uint z){
	return glm::max(x,glm::max(y,z)); 
}
uint Math::Max(uint x, uint y, uint z, uint w){
	return glm::max(x,glm::max(y,glm::max(z,w))); 
}

glm::vec3 Math::unpack3NormalsFrom32Int(uint32_t data) {
    glm::vec3 conversions;
    glm::vec3 negatives = glm::vec3(1.0f);
    //X
    conversions.x = static_cast<float>(data & 1023 << 0);
    if (conversions.x >= 512.0f) { //2^9
        conversions.x = 1023 - conversions.x; //2^10
        negatives.x *= -1.0f;
    }
    conversions.x /= 511.0f * negatives.x; //(2^9) - 1
    //Y 
    conversions.y = static_cast<float>(data & 1023 << 10);
    if (conversions.y >= 524289.0f) { //2^19
        conversions.y = 1048575.0f - conversions.y; //2^20
        negatives.y *= -1.0f;
    }
    conversions.y /= 524288.0f * negatives.y; //(2^19) - 1
    //Z
    conversions.z = static_cast<float>(data & 1023 << 20);
    if (conversions.z >= 536870912.0f) { //2^29
        conversions.z = 1073741824.0f - conversions.z; //2^30
        negatives.z *= -1.0f;
    }
    conversions.z /= 536870911.0f * negatives.z; //(2^29) - 1
    return conversions;
}
uint32_t Math::pack3NormalsInto32Int(float x, float y, float z){
    uint32_t xsign = x < 0; //if x < 0, this = 1, else this = 0
    uint32_t ysign = y < 0; //if y < 0, this = 1, else this = 0
    uint32_t zsign = z < 0; //if z < 0, this = 1, else this = 0
    float w = 0.0f;         //2 bits left for w, should i ever want to use it
    uint32_t wsign = w < 0; //if w < 0, this = 1, else this = 0
    uint32_t intW = ((uint32_t)(w       + (wsign << 1)) & 1);
    uint32_t intZ = ((uint32_t)(z * 511 + (zsign << 9)) & 511);
    uint32_t intY = ((uint32_t)(y * 511 + (ysign << 9)) & 511);
    uint32_t intX = ((uint32_t)(x * 511 + (xsign << 9)) & 511);
    uint32_t data = 
        (wsign << 31 | intW << 30) |
        (zsign << 29 | intZ << 20) |
        (ysign << 19 | intY << 10) |
        (xsign << 9  | intX      );
    return data;
}
uint32_t Math::pack3NormalsInto32Int(const glm::vec3& v){
	return Math::pack3NormalsInto32Int(v.x,v.y,v.z); 
}


float Math::pack3FloatsInto1Float(float r, float g, float b){
    r = (r + 1.0f) * 0.5f;
    uchar _r = static_cast<uchar>(r * 255.0f);
    g = (g + 1.0f) * 0.5f;
    uchar _g = static_cast<uchar>(g * 255.0f);
    b = (b + 1.0f) * 0.5f;
    uchar _b = static_cast<uchar>(b * 255.0f);
    uint packedColor = (_r << 16) | (_g << 8) | _b;
    float packedFloat = static_cast<float>(static_cast<double>(packedColor) / static_cast<double>(1 << 24));
    return packedFloat;
}
float Math::pack3FloatsInto1Float(const glm::vec3& c){ 
	return Math::pack3FloatsInto1Float(c.r,c.g,c.b); 
}
glm::vec3 Math::unpack3FloatsInto1Float(float v){
    glm::vec3 ret = glm::vec3(
		static_cast<float>(fmod(v, 1.0f)),
		static_cast<float>(fmod(v * 256.0f, 1.0f)),
		static_cast<float>(fmod(v * 65536.0f, 1.0f))
	);
    //Unpack to the -1..1 range
    ret.r = (ret.r * 2.0f) - 1.0f;
    ret.g = (ret.g * 2.0f) - 1.0f;
    ret.b = (ret.b * 2.0f) - 1.0f;
    return ret;
}
float Math::pack3FloatsInto1FloatUnsigned(float r,float g,float b){
    uchar _r = static_cast<uchar>(r * 255.0f);
    uchar _g = static_cast<uchar>(g * 255.0f);
    uchar _b = static_cast<uchar>(b * 255.0f);
    uint packedColor = (_r << 16) | (_g << 8) | _b;
    float packedFloat = static_cast<float>(static_cast<double>(packedColor) / static_cast<double>(1 << 24) );
    return packedFloat;
}
float Math::pack3FloatsInto1FloatUnsigned(const glm::vec3& c){ 
	return Math::pack3FloatsInto1Float(c.r,c.g,c.b); 
}
glm::vec3 Math::unpack3FloatsInto1FloatUnsigned(float v){
    const glm::vec3 ret = glm::vec3(
		static_cast<float>(fmod(v, 1.0f)),
		static_cast<float>(fmod(v * 256.0f, 1.0f)),
		static_cast<float>(fmod(v * 65536.0f, 1.0f))
	);
    return ret;
}
uchar Math::pack2NibblesIntoChar(float x, float y) {
    uchar packedData = 0;
    int bits  = static_cast<int>(round(x / 0.066666666666f));
    int bits1 = static_cast<int>(round(y / 0.066666666666f));
	packedData |= bits & 15;
	packedData |= (bits1 << 4) & 240;
    return packedData;
}
glm::vec2 Math::unpack2NibblesFromChar(unsigned char _packedData) {
    int low  = _packedData & 15;
    int high = _packedData >> 4;
    return glm::vec2(static_cast<float>(low * 0.066666666666f), static_cast<float>(high * 0.066666666666f));
}
//attempt to do the above using non bitwise operations for glsl versions that do not support bitwise operations
float Math::pack2NibblesIntoCharBasic(float x, float y) {
    float lowEnd = (round(x / 0.066666666666f));
    float highEnd = (round(y / 0.066666666666f) * 16.0f);
    return (lowEnd + highEnd);
}
glm::vec2 Math::unpack2NibblesFromCharBasic(float _packedData) {
    float highEnd = (_packedData / 16.0f);
    highEnd = highEnd - glm::floor(highEnd);
    float lowEnd = static_cast<float>(_packedData - (highEnd * 16.0f));
    return glm::vec2(highEnd, static_cast<float>(lowEnd / 255.0));
}
float Math::pack2FloatsInto1Float(float x, float y){
    int _x = static_cast<int>((x + 1.0f) * 0.5f);
	int _y = static_cast<int>((y + 1.0f) * 0.5f);
    return glm::floor(_x * 1000.0f) + _y; 
}
float Math::pack2FloatsInto1Float(const glm::vec2& v){
	return Math::pack2FloatsInto1Float(v.x,v.y); 
}
glm::vec2 Math::unpack2FloatsInto1Float(float i){
    glm::vec2 res;
    res.y = i - glm::floor(i);
    res.x = (i - res.y) / 1000.0f;
    res.x = (res.x - 0.5f) * 2.0f;
    res.y = (res.y - 0.5f) * 2.0f;
    return res;
}
void Math::translate(const btRigidBody& body, btVector3& vec, bool local){
    if(local){
        btQuaternion q = body.getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(), q.getAngle());
    }
}
glm::vec3 Math::midpoint(const glm::vec3& a, const glm::vec3& b){ 
    return glm::vec3((a.x + b.x) / 2.0f, (a.y + b.y) / 2.0f, (a.z + b.z) / 2.0f);
}
glm::vec3 Math::direction(const glm::vec3& eye, const glm::vec3& target){ 
    return glm::normalize(eye - target);
}
glm::vec3 Math::getForward(const glm_quat& q){
    return glm::normalize(q * glm_vec3(0, 0, -1));
}
glm::vec3 Math::getRight(const glm_quat& q){
    return glm::normalize(q * glm_vec3(1, 0, 0));
}
glm::vec3 Math::getUp(const glm_quat& q){
    return glm::normalize(q * glm_vec3(0, 1, 0));
}
glm::vec3 Math::getColumnVector(const btRigidBody& b, unsigned int column){
    btTransform t;
    b.getMotionState()->getWorldTransform(t);
    btVector3 v = t.getBasis().getColumn(column);
    return glm::normalize(glm::vec3(v.x(), v.y(), v.z()));
}
glm::vec3 Math::getForward(const btRigidBody& b) {
    return Math::getColumnVector(b, 2);
}
glm::vec3 Math::getRight(const btRigidBody& b) {
    return Math::getColumnVector(b, 0);
}
glm::vec3 Math::getUp(const btRigidBody& b) {
    return Math::getColumnVector(b, 1);
}
void Math::recalculateForwardRightUp(const glm_quat& o,glm_vec3& f,glm_vec3& r,glm_vec3& u){
    f = Math::getForward(o);
	r = Math::getRight(o);
	u = Math::getUp(o);
}
void Math::recalculateForwardRightUp(const btRigidBody& b,glm_vec3& f,glm_vec3& r,glm_vec3& u){
    f = Math::getForward(b);
	r = Math::getRight(b);
	u = Math::getUp(b);
}
float Math::getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees){
	if (a == b) 
        return 0.0f;
	float angle = glm::acos(glm::dot(a, b));
    if(degrees) 
		angle *= 57.2958f;
    return angle;
}
void Math::alignTo(glm_quat& o, const glm_vec3& direction){ 
    o = glm::conjugate(glm::toQuat(glm::lookAt(glm_vec3(0.0f), -direction, glm_vec3(0, 1, 0))));
    o = glm::normalize(o);
}
void Math::setColor(glm::vec3& c, float r, float g, float b){
    if(r > 1.0f) r /= 255.0f;
    if(g > 1.0f) g /= 255.0f;
    if(b > 1.0f) b /= 255.0f;
    c.x = r;
	c.y = g;
	c.z = b;
}
void Math::setColor(glm::vec4& c, float r, float g, float b, float a){
    if(r > 1.0f) r /= 255.0f;
    if(g > 1.0f) g /= 255.0f;
    if(b > 1.0f) b /= 255.0f;
    if(a > 1.0f) a /= 255.0f;
    c.x = r;
	c.y = g;
	c.z = b;
	c.w = a; 
}
float Math::fade(float t){ 
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
double Math::fade(double t){ 
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
float Math::lerp(float t, float a, float b){
	return a + t * (b - a);
}
double Math::lerp(double t, double a, double b){
	return a + t * (b - a);
}
float Math::grad(int hash, float x, float y, float z){
	int h    = hash & 15;
	double u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return float(((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v));
}
double Math::grad(int hash, double x, double y, double z){
	int h    = hash & 15;
	double u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
glm::vec4 Math::PaintersAlgorithm(const glm::vec4& paint_color, const glm::vec4& canvas_color){
    glm::vec4 output(0.0f);
    float alpha = paint_color.a + canvas_color.a * (1.0f - paint_color.a);
    output      = ((paint_color * paint_color.a + canvas_color * canvas_color.a * (1.0f - paint_color.a)) / alpha);
    output.a    = alpha;
    return output;
}
sf::Color Math::PaintersAlgorithm(const sf::Color& paint_color, const sf::Color& canvas_color) {
    glm::vec4 cC = glm::vec4((float)canvas_color.r / 255.0f, (float)canvas_color.g / 255.0f, (float)canvas_color.b / 255.0f, (float)canvas_color.a / 255.0f);
    glm::vec4 pC = glm::vec4((float)paint_color.r / 255.0f, (float)paint_color.g / 255.0f, (float)paint_color.b / 255.0f, (float)paint_color.a / 255.0f);
    float alpha  = pC.a + cC.a * (1.0f - pC.a);
    glm::vec4 calculations(0.0f);
    calculations = (pC * pC.a + cC * cC.a * (1.0f - pC.a) / alpha);
    calculations.a = alpha;
    sf::Uint8 finalR = (sf::Uint8)(calculations.r * 255.0f);
    sf::Uint8 finalG = (sf::Uint8)(calculations.g * 255.0f);
    sf::Uint8 finalB = (sf::Uint8)(calculations.b * 255.0f);
    sf::Uint8 finalA = (sf::Uint8)(calculations.a * 255.0f);
    return sf::Color(finalR, finalG, finalB, finalA);
}
bool Math::rayIntersectSphere(const glm::vec3& C, float r, const glm::vec3& A, const glm::vec3& rayVector){
    glm::vec3 B = A + rayVector;
    float dot   = glm::dot(rayVector, C - A);
    if (dot >= 0.0f) { //check if point is behind
        return false;
    }
    float a = ((B.x - A.x) * (B.x - A.x)) + ((B.y - A.y) * (B.y - A.y)) + ((B.z - A.z) * (B.z - A.z));
    float b = 2.0f * ((B.x - A.x) * (A.x - C.x) + (B.y - A.y) * (A.y - C.y) + (B.z - A.z) * (A.z - C.z));
    float c = (((A.x - C.x) * (A.x - C.x)) + ((A.y - C.y) * (A.y - C.y)) + ((A.z - C.z) * (A.z - C.z))) - (r * r);
    float d = (b * b) - (4.0f * a * c);
    return !(d < 0.0f);
}
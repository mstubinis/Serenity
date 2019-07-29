#include <core/engine/math/Engine_Math.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/scene/Scene.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <btBulletDynamicsCommon.h>

using namespace Engine;
using namespace std;

typedef unsigned char uchar;

const float ROTATION_THRESHOLD = 0.00001f;

void Math::Float32From16(float*     out, const uint16_t in) {
    uint32_t t1, t2, t3;
    t1 = in & 0x7fff;                       // Non-sign bits
    t2 = in & 0x8000;                       // Sign bit
    t3 = in & 0x7c00;                       // Exponent
    t1 <<= 13;                              // Align mantissa on MSB
    t2 <<= 16;                              // Shift sign bit into position
    t1 += 0x38000000;                       // Adjust bias
    t1 = (t3 == 0 ? 0 : t1);                // Denormals-as-zero
    t1 |= t2;                               // Re-insert sign bit
    *(uint32_t*)out = t1;
}
void Math::Float16From32(uint16_t*  out, const float    in) {
    uint32_t inu = *((uint32_t*)&in);
    uint32_t t1, t2, t3;
    t1 = inu & 0x7fffffff;                 // Non-sign bits
    t2 = inu & 0x80000000;                 // Sign bit
    t3 = inu & 0x7f800000;                 // Exponent
    t1 >>= 13;                             // Align mantissa on MSB
    t2 >>= 16;                             // Shift sign bit into position
    t1 -= 0x1c000;                         // Adjust bias
    t1 = (t3 < 0x38800000) ? 0 : t1;
    t1 = (t3 > 0x47000000) ? 0x7bff : t1;
    t1 = (t3 == 0 ? 0 : t1);               // Denormals-as-zero
    t1 |= t2;                              // Re-insert sign bit
    *(uint16_t*)out = t1;
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
void Math::setFinalModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale) {
    modelMatrix = glm::mat4(1.0f);
    const glm::mat4 translationMatrix = glm::translate(position);
    const glm::mat4 rotationMatrix = glm::mat4_cast(rotation);
    const glm::mat4 scaleMatrix = glm::scale(scale);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix /*  * modelMatrix  */;
}
void Math::setRotation(glm::quat& orientation, const float& pitch, const float& yaw, const float& roll) {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation               = (glm::angleAxis(pitch, glm::vec3(-1, 0, 0)));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm::vec3(0, -1, 0)));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,   glm::vec3(0, 0, 1)));   //roll
}
void Math::rotate(glm::quat& orientation, const float& pitch, const float& yaw, const float& roll) {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(pitch, glm::vec3(-1, 0, 0)));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm::vec3(0, -1, 0)));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,   glm::vec3(0, 0, 1)));   //roll
}
glm::vec2 Math::rotate2DPoint(const glm::vec2& point, const float angle, const glm::vec2& origin) {
    float s = glm::sin(angle);
    float c = glm::cos(angle);
    glm::vec2 ret;
    ret.x = c * (point.x - origin.x) - s * (point.y - origin.y) + origin.x;
    ret.y = s * (point.x - origin.x) + c * (point.y - origin.y) + origin.x;
    return ret;
}



void Math::extractViewFrustumPlanesHartmannGribbs(const glm::mat4& inViewProjection,glm::vec4* outPlanes){
    glm::vec4 rows[4];
    for(ushort i = 0; i < 4; ++i)
        rows[i] = glm::row(inViewProjection,i);
    //0 = left, 1 = right, 2 = top, 3 = bottom, 4 = near, 5 = far
    for(ushort i = 0; i < 3; ++i){
        ushort index = i * 2;
        outPlanes[index  ] = -(rows[3] + rows[i]);  //0,2,4
        outPlanes[index+1] = -(rows[3] - rows[i]);  //1,3,5
    }
    //https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
    //avoiding normalization of the planes for now so we can uitilize halfspace and thus negatives (outside frust / inside frust / intersecting frust)
    /*
    for(ushort i = 0; i < 6; ++i){
        float mag = glm::sqrt(outPlanes[i].x * outPlanes[i].x + outPlanes[i].y * outPlanes[i].y + outPlanes[i].z * outPlanes[i].z);
        outPlanes[i].x = outPlanes[i].x / mag;
        outPlanes[i].y = outPlanes[i].y / mag;
        outPlanes[i].z = outPlanes[i].z / mag;
        outPlanes[i].w = outPlanes[i].w / mag;
    }
    */
}
glm::quat Math::btToGLMQuat(const btQuaternion& q){
	return glm::quat(q.getW(),q.getX(),q.getY(),q.getZ()); 
}
btQuaternion Math::glmToBTQuat(const glm::quat& q){
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

float Math::toRadians(const float degrees){
	return degrees * 0.0174533f; 
}
float Math::toDegrees(const float radians){
	return radians * 57.2958f; 
}
float Math::toRadians(const double degrees){
	return Math::toRadians(static_cast<float>(degrees)); 
}
float Math::toDegrees(const double radians){
	return Math::toDegrees(static_cast<float>(radians));
}
float Math::remainder(const float x, const float y){
	return x - (glm::round(x / y) * y);
}

glm::vec3 Math::btVectorToGLM(const btVector3& btvector){
	return glm::vec3(btvector.x(), btvector.y(), btvector.z());
}
btVector3 Math::btVectorFromGLM(const glm::vec3& vector){ 
	return btVector3(vector.x, vector.y, vector.z);
}

void Math::removeMatrixPosition(glm::mat4& matrix){
	matrix[3][0] = 0;
	matrix[3][1] = 0;
	matrix[3][2] = 0;
}

bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector, const glm::vec3& point,const float fovRadians){
    const glm::vec3& diff = glm::normalize(point - conePos);
    float t = glm::dot(coneVector,diff);
    return ( t >= glm::cos( fovRadians ) );
}
bool Math::isPointWithinCone(const glm::vec3& conePos,const glm::vec3& coneVector, const glm::vec3& point,const float fovRadians,const float maxDistance){
	const glm::vec3& diff = glm::normalize(point - conePos);
	const float t = glm::dot(coneVector,diff);
	const float length = glm::length(point-conePos);
    if ( length > maxDistance ){ return false; }
    return ( t >= glm::cos( fovRadians ) );
}

glm::vec3 Math::getScreenCoordinates(const glm::vec3& objPos,Camera& camera, const bool clampToEdge) {
    const glm::uvec2& winSizeInt = Resources::getWindowSize();
	const glm::vec2&  winSize    = glm::vec2(winSizeInt.x, winSizeInt.y);
	const glm::vec4&  viewport   = glm::vec4(0, 0, winSize.x, winSize.y);
	const glm::vec3&  screen     = glm::project(objPos, camera.getView(), camera.getProjection(), viewport);
    //check if point is behind
	const float dot = glm::dot(camera.getViewVector(), objPos - camera.getPosition());
    float resX = screen.x;
    float resY = screen.y;
    uint inBounds = 1;
    if (clampToEdge) {
        if (screen.x < 0.0f) { 
            resX = 0.0f; 
            inBounds = 0; 
        }else if (screen.x > winSize.x) { 
            resX = winSize.x; 
            inBounds = 0; 
        }
        if (resY < 0.0f) { 
            resY = 0.0f; 
            inBounds = 0; 
        }else if (resY > winSize.y) { 
            resY = winSize.y; 
            inBounds = 0; 
        }
    }
    if (dot < 0.0f) {
        return glm::vec3(resX, resY, inBounds);
    }
    inBounds = 0;
    resX = winSize.x - screen.x;
    resY = winSize.y - screen.y;
    if (clampToEdge) {
        if (resX < winSize.x / 2) {
            resX = 0.0f;
        }else if (resX > winSize.x / 2) {
            resX = winSize.x;
        }
        if (resY < winSize.y / 2) {
            resY = 0.0f;
        }else if (resY > winSize.y / 2) {
            resY = winSize.y;
        }
    }
    return glm::vec3(resX, resY, inBounds);
}
glm::vec3 Math::getScreenCoordinates(const glm::vec3& objPos, const bool clampToEdge){
    return Math::getScreenCoordinates(objPos,*Resources::getCurrentScene()->getActiveCamera(),clampToEdge);
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
float Math::Max(const float x, const float y){
	return glm::max(x,y); 
}
float Math::Max(const float x, const float y, const float z){
	return glm::max(x,glm::max(y,z)); 
}
float Math::Max(const float x, const float y, const float z, const float w){
	return glm::max(x,glm::max(y,glm::max(z,w))); 
}
uint Math::Max(const uint x, const uint y){
	return glm::max(x,y); 
}
uint Math::Max(const uint x, const uint y, const uint z){
	return glm::max(x,glm::max(y,z)); 
}
uint Math::Max(const uint x, const uint y, const uint z, const uint w){
	return glm::max(x,glm::max(y,glm::max(z,w))); 
}

GLuint Math::pack3NormalsInto32Int(const float x, const float y, const float z){
   const int _X = static_cast<int>(x * 511.0f);
   const int _Y = static_cast<int>(y * 511.0f);
   const int _Z = static_cast<int>(z * 511.0f);
   return (_X & 0x3FF) | ((_Y & 0x3FF) << 10) | ((_Z & 0x3FF) << 20);
}
GLuint Math::pack3NormalsInto32Int(const glm::vec3& v){ 
	return Math::pack3NormalsInto32Int(v.x,v.y,v.z); 
}


float Math::pack3FloatsInto1Float(float r,float g,float b){
    //Scale and bias
    r = (r + 1.0f) * 0.5f;
	const uchar _r = static_cast<uchar>(r * 255.0f);
    g = (g + 1.0f) * 0.5f;
	const uchar _g = static_cast<uchar>(g * 255.0f);
    b = (b + 1.0f) * 0.5f;
	const uchar _b = static_cast<uchar>(b * 255.0f);
	const uint packedColor = (_r << 16) | (_g << 8) | _b;
	const float packedFloat = static_cast<float>(static_cast<double>(packedColor) / static_cast<double>(1 << 24));
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
	const uchar _r = static_cast<uchar>(r * 255.0f);
	const uchar _g = static_cast<uchar>(g * 255.0f);
	const uchar _b = static_cast<uchar>(b * 255.0f);
	const uint packedColor = (_r << 16) | (_g << 8) | _b;
	const float packedFloat = static_cast<float>(static_cast<double>(packedColor) / static_cast<double>(1 << 24) );
    return packedFloat;
}
float Math::pack3FloatsInto1FloatUnsigned(const glm::vec3& c){ 
	return Math::pack3FloatsInto1Float(c.r,c.g,c.b); 
}
glm::vec3 Math::unpack3FloatsInto1FloatUnsigned(float v){
    const glm::vec3& ret = glm::vec3(
		static_cast<float>(fmod(v, 1.0f)),
		static_cast<float>(fmod(v * 256.0f, 1.0f)),
		static_cast<float>(fmod(v * 65536.0f, 1.0f))
	);
    return ret;
}
uchar Math::pack2NibblesIntoChar(const float x, const float y) {
    uchar packedData = 0x00;
    int bits  = static_cast<int>(round(x / 0.0666f));
    int bits1 = static_cast<int>(round(y / 0.0666f));
	packedData |= bits & 0x0F;
	packedData |= (bits1 << 4) & 0xF0;
    return packedData;
}
glm::vec2 Math::unpack2NibblesFromChar(const uchar _packedData) {
    int low  = _packedData & 0x0F; /* binary 00000001 */
    int high = _packedData >> 4;   /* binary 10000000 */
    return glm::vec2(static_cast<float>(low * 0.0666f), static_cast<float>(high * 0.0666f));
}
//attempt to do the above using non bitwise operations for glsl versions that do not support bitwise operations
float Math::pack2NibblesIntoCharBasic(const float x, const float y) {
    float lowEnd = static_cast<float>(round(x / 0.0666f));
    float highEnd = static_cast<float>(round(y / 0.0666f) * 16.0f);
    return static_cast<float>(lowEnd + highEnd);
}
glm::vec2 Math::unpack2NibblesFromCharBasic(const float _packedData) {
    float highEnd = (_packedData / 16.0f);
    highEnd = highEnd - glm::floor(highEnd);
    float lowEnd = static_cast<float>(_packedData - (highEnd * 16.0f));
    return glm::vec2(highEnd, static_cast<float>(lowEnd / 255.0));
}
float Math::pack2FloatsInto1Float(const float x, const float y){
    int _x = static_cast<int>((x + 1.0f) * 0.5f);
	int _y = static_cast<int>((y + 1.0f) * 0.5f);
    return glm::floor(_x * 1000.0f) + _y; 
}
float Math::pack2FloatsInto1Float(const glm::vec2& v){
	return Math::pack2FloatsInto1Float(v.x,v.y); 
}
glm::vec2 Math::unpack2FloatsInto1Float(const float i){
    glm::vec2 res;
    res.y = i - glm::floor(i);
    res.x = (i - res.y) / 1000.0f;
    res.x = (res.x - 0.5f) * 2.0f;
    res.y = (res.y - 0.5f) * 2.0f;
    return res;
}
void Math::translate(const btRigidBody& body,btVector3& vec,bool local){
    if(local){
        const btQuaternion& q = body.getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(), q.getAngle());
    }
}

glm::vec3 Math::midpoint(const glm::vec3& a, const glm::vec3& b){ return glm::vec3((a.x+b.x)/2.0f,(a.y+b.y)/2.0f,(a.z+b.z)/2.0f); }
glm::vec3 Math::direction(const glm::vec3& eye, const glm::vec3& target){ return glm::normalize(eye-target); }
glm::vec3 Math::getForward(const glm::quat& q){return glm::normalize(q * glm::vec3(0.0f,0.0f,-1.0f));}
glm::vec3 Math::getRight(const glm::quat& q){return glm::normalize(q * glm::vec3(1.0f,0.0f,0.0f));}
glm::vec3 Math::getUp(const glm::quat& q){return glm::normalize(q * glm::vec3(0.0f,1.0f,0.0f));}
glm::vec3 Math::getColumnVector(const btRigidBody& b, const uint& column){
    btTransform t;
    b.getMotionState()->getWorldTransform(t);
    btVector3 v = t.getBasis().getColumn(column);
    return glm::normalize(glm::vec3(v.x(),v.y(),v.z()));
}
glm::vec3 Math::getForward(const btRigidBody& b){ 
	return Math::getColumnVector(b,2); 
}
glm::vec3 Math::getRight(const btRigidBody& b){ 
	return Math::getColumnVector(b,0); 
}
glm::vec3 Math::getUp(const btRigidBody& b){ 
	return Math::getColumnVector(b,1); 
}
void Math::recalculateForwardRightUp(const glm::quat& o,glm::vec3& f,glm::vec3& r,glm::vec3& u){
    f = Math::getForward(o);
	r = Math::getRight(o);
	u = Math::getUp(o);
}
void Math::recalculateForwardRightUp(const btRigidBody& b,glm::vec3& f,glm::vec3& r,glm::vec3& u){
    f = Math::getForward(b);
	r = Math::getRight(b);
	u = Math::getUp(b);
}
float Math::getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees){
	if (a == b) return 0;
    //float angle = glm::acos( glm::dot(glm::normalize(a),glm::normalize(b)) );
	float angle = glm::acos(glm::dot(a, b));
    if(degrees) 
		angle *= 57.2958f;
    return angle;
}
void Math::alignTo(glm::quat& o, const glm::vec3& direction,float speed){
	const glm::vec3& _direction = glm::normalize(direction);
	const glm::quat original(o);
    const glm::vec3 xaxis = glm::normalize(glm::cross(glm::vec3(0.0f,1.0f,0.0f), _direction));
	const glm::vec3 yaxis = glm::normalize(glm::cross(_direction, xaxis));
    glm::mat3 rot;
    rot[0][0] = float(xaxis.x);      rot[0][1] = float(xaxis.y);      rot[0][2] = float(xaxis.z);
    rot[1][0] = float(yaxis.x);      rot[1][1] = float(yaxis.y);      rot[1][2] = float(yaxis.z);
    rot[2][0] = float(_direction.x); rot[2][1] = float(_direction.y); rot[2][2] = float(_direction.z);
    o = glm::quat_cast(rot);
    if(speed != 0.0f){
        const float& angle = Math::getAngleBetweenTwoVectors(_direction, getForward(original) ,true); // degrees
        speed *= 1.0f / angle;
        speed *= (float)Resources::dt();
		o = glm::mix(original, o, speed * 5.0f);
    }
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
float Math::fade(const float t){ 
	return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}
double Math::fade(const double t){ 
	return t * t * t * (t * (t * 6.0 - 15.0) + 10.0);
}
float Math::lerp(const float t, const float a, const float b){
	return a + t * (b - a);
}
double Math::lerp(const double t, const double a, const double b){
	return a + t * (b - a);
}
float Math::grad(const int hash, const float x, const float y, const float z){
	const int h = hash & 15;
	const double u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return float(((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v));
}
double Math::grad(const int hash, const double x, const double y, const double z){
	const int h = hash & 15;
	const double u = h < 8 ? x : y, v = h < 4 ? y : h == 12 || h == 14 ? x : z;
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
glm::vec4 Math::PaintersAlgorithm(const glm::vec4& paint_color, const glm::vec4& canvas_color){
	const float& alpha = paint_color.a + canvas_color.a * (1.0f - paint_color.a);
    glm::vec4 ret(0.0f);
    ret   = ((paint_color * paint_color.a + canvas_color * canvas_color.a * (1.0f - paint_color.a)) / alpha);
    ret.a = alpha;
    return ret;
}
sf::Color Math::PaintersAlgorithm(const sf::Color& paint_color, const sf::Color& canvas_color) {
    const glm::vec4 cC = glm::vec4(static_cast<float>(canvas_color.r) / 255.0f, static_cast<float>(canvas_color.g) / 255.0f, static_cast<float>(canvas_color.b) / 255.0f, static_cast<float>(canvas_color.a) / 255.0f);
    const glm::vec4 pC = glm::vec4(static_cast<float>(paint_color.r) / 255.0f, static_cast<float>(paint_color.g) / 255.0f, static_cast<float>(paint_color.b) / 255.0f, static_cast<float>(paint_color.a) / 255.0f);
    const float full = 1.0f;
    const float alpha = pC.a + cC.a * (full - pC.a);
    glm::vec4 ret(0.0f);
    ret = (pC * pC.a + cC * cC.a * (full - pC.a) / alpha);
    ret.a = alpha;
    const sf::Uint8 finalR = static_cast<sf::Uint8>(ret.r * 255.0f);
    const sf::Uint8 finalG = static_cast<sf::Uint8>(ret.g * 255.0f);
    const sf::Uint8 finalB = static_cast<sf::Uint8>(ret.b * 255.0f);
    const sf::Uint8 finalA = static_cast<sf::Uint8>(ret.a * 255.0f);
    return sf::Color(finalR, finalG, finalB, finalA);
}
bool Math::rayIntersectSphere(const glm::vec3& C, const float r, const glm::vec3& A, const glm::vec3& rayVector){
	const glm::vec3& B = A + rayVector;
    const float& dot = glm::dot(rayVector, C - A);
    if(dot >= 0.0f) //check if point is behind
        return false;
	const float& a = ((B.x - A.x) * (B.x - A.x)) + ((B.y - A.y) * (B.y - A.y)) + ((B.z - A.z) * (B.z - A.z));
	const float& b = 2.0f * ((B.x - A.x) * (A.x - C.x) + (B.y - A.y) * (A.y - C.y) + (B.z - A.z) * (A.z - C.z));
	const float& c = (((A.x - C.x) * (A.x - C.x)) + ((A.y - C.y) * (A.y - C.y)) + ((A.z - C.z) * (A.z - C.z))) - (r * r);
	const float& d = (b * b) - (4.0f * a * c);
    if(d < 0.0f)
        return false;
    return true;
}
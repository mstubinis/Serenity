
#include <serenity/math/Engine_Math.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/scene/Camera.h>
#include <serenity/scene/Scene.h>

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/quaternion.hpp>

#include <btBulletDynamicsCommon.h>
#include <boost/math/interpolators/cubic_b_spline.hpp>

using namespace Engine;

constexpr float ROTATION_THRESHOLD = 0.00001f;

//could use some fixing
glm::vec3 Math::polynomial_interpolate_linear(const std::vector<glm::vec3>& points, float time) {
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
        size_t index1 = (size_t)glm::floor((size_t)indexFloat);
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
glm::vec3 Math::polynomial_interpolate_cubic(const std::vector<glm::vec3>& points, float time) {
    glm::vec3 ret{ 0.0f };
    uint32_t n  = (uint32_t)points.size();
    auto x_vals = Engine::create_and_reserve<std::vector<float>>(n);
    auto y_vals = Engine::create_and_reserve<std::vector<float>>(n);
    auto z_vals = Engine::create_and_reserve<std::vector<float>>(n);
    std::for_each(std::cbegin(points), std::cend(points), [&x_vals, &y_vals, &z_vals](const auto& pt) {
        x_vals.emplace_back(pt.x);
        y_vals.emplace_back(pt.y);
        z_vals.emplace_back(pt.z);
    });
    auto step = 1.0f / (float)(n - 1);
    boost::math::cubic_b_spline<float> x_spline{ x_vals.data(), n, 0.0f, step };
    boost::math::cubic_b_spline<float> y_spline{ y_vals.data(), n, 0.0f, step };
    boost::math::cubic_b_spline<float> z_spline{ z_vals.data(), n, 0.0f, step };
    return glm::vec3{ x_spline(time), y_spline(time), z_spline(time) };
}

bool Math::IsNear(float v1, float v2, float threshold) noexcept {
    return (std::abs(v1 - v2) < threshold);
}
bool Math::IsNear(glm::vec2& v1, glm::vec2& v2, float threshold) noexcept {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold);
}
bool Math::IsNear(glm::vec3& v1, glm::vec3& v2, float threshold) noexcept {
    return (std::abs(v1.x - v2.x) < threshold && std::abs(v1.y - v2.y) < threshold && std::abs(v1.z - v2.z) < threshold);
}
bool Math::IsSpecialFloat(float f) noexcept {
    if (boost::math::isnan(f)) return true;
    if (boost::math::isinf(f)) return true;
    return false;
}
bool Math::IsSpecialFloat(const glm::vec2& v) noexcept {
    if (boost::math::isnan(v.x) || boost::math::isnan(v.y)) return true;
    if (boost::math::isinf(v.x) || boost::math::isinf(v.y)) return true;
    return false;
}
bool Math::IsSpecialFloat(const glm::vec3& v) noexcept {
    if (boost::math::isnan(v.x) || boost::math::isnan(v.y) || boost::math::isnan(v.z)) return true;
    if (boost::math::isinf(v.x) || boost::math::isinf(v.y) || boost::math::isinf(v.z)) return true;
    return false;
}

bool Math::rect_fully_contained(const glm::vec4& bigger, const glm::vec4& smaller) noexcept {
    return ((smaller.x >= bigger.x && smaller.x + smaller.z <= bigger.x + bigger.z) && (smaller.y >= bigger.y && smaller.y + smaller.w <= bigger.y + bigger.w));
}
glm::vec4 Math::rect_union(const glm::vec4& bigger, const glm::vec4& smaller) noexcept {
    if (bigger == glm::vec4{ -1.0f }) {
        return smaller;
    }
    float x = std::max(bigger.x, smaller.x);
    float y = std::max(bigger.y, smaller.y);
    float z = std::min(bigger.x + bigger.z, smaller.x + smaller.z);
    float w = std::min(bigger.y + bigger.w, smaller.y + smaller.w);
    if (x > z || y > w) { //no intersection
        return bigger;
    }
    return glm::vec4{ x, y, std::abs(z - x), std::abs(w - y) };
}

void Math::Float32From16(float* out, const uint16_t* in, uint32_t arraySize) noexcept {
    for (uint32_t i = 0; i < arraySize; ++i) {
        Math::Float32From16(&(out[i]), in[i]);
    }
}
void Math::Float16From32(uint16_t* out, const float* in, uint32_t arraySize) noexcept {
    for (uint32_t i = 0; i < arraySize; ++i) {
        Math::Float16From32(&(out[i]), in[i]);
    }
}
void Math::setFinalModelMatrix(glm_mat4& modelMatrix, const glm_vec3& position, const glm_quat& rotation, const glm_vec3& inScale) noexcept {
    modelMatrix            = glm_mat4(1.0);
    auto translationMatrix = glm::translate(position);
    auto rotationMatrix    = glm::mat4_cast(rotation);
    auto scaleMatrix       = glm::scale(inScale);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}
void Math::setFinalModelMatrix(glm::mat4& modelMatrix, const glm::vec3& position, const glm::quat& rotation, const glm::vec3& inScale) noexcept {
    modelMatrix            = glm::mat4(1.0f);
    auto translationMatrix = glm::translate(position);
    auto rotationMatrix    = glm::mat4_cast(rotation);
    auto scaleMatrix       = glm::scale(inScale);
    modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
}
void Math::setRotation(glm_quat& orientation, decimal pitch, decimal yaw, decimal roll) noexcept {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation               = (glm::angleAxis(pitch, glm_vec3{ -1, 0, 0 }));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm_vec3{0, -1, 0}));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,  glm_vec3{ 0, 0, 1 }));   //roll
}
void Math::setRotation(glm::quat& orientation, float pitch, float yaw, float roll) noexcept {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation =               (glm::angleAxis(pitch, glm::vec3{ -1, 0, 0 }));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm::vec3{ 0, -1, 0 }));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,  glm::vec3{ 0, 0, 1 }));   //roll
}
void Math::rotate(glm_quat& orientation, decimal pitch, decimal yaw, decimal roll) noexcept {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(pitch, glm_vec3{ -1, 0, 0 }));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm_vec3{ 0, -1, 0 }));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,  glm_vec3{ 0, 0, 1 }));   //roll
}
void Math::rotate(glm::quat& orientation, float pitch, float yaw, float roll) noexcept {
    if (abs(pitch) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(pitch, glm::vec3{ -1, 0, 0 }));   //pitch
    if (abs(yaw) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(yaw,   glm::vec3{ 0, -1, 0 }));   //yaw
    if (abs(roll) > ROTATION_THRESHOLD)
        orientation = orientation * (glm::angleAxis(roll,  glm::vec3{ 0, 0, 1 }));   //roll
}
glm::vec2 Math::rotate2DPoint(const glm::vec2& point, float angle, const glm::vec2& origin) {
    float s = glm::sin(angle);
    float c = glm::cos(angle);
    return {
        c* (point.x - origin.x) - s * (point.y - origin.y) + origin.x,
        s* (point.x - origin.x) + c * (point.y - origin.y) + origin.x
    };
}

void Math::extractViewFrustumPlanesHartmannGribbs(const glm::mat4& inViewProjection, std::array<glm::vec4, 6>& outPlanes){
    glm::vec4 rows[4];
    for (uint32_t i = 0; i < 4; ++i) {
        rows[i] = glm::row(inViewProjection, i);
    }
    //0 = left, 1 = right, 2 = top, 3 = bottom, 4 = near, 5 = far
    for (size_t i = 0; i < outPlanes.size() / 2; ++i) {
        size_t index         = i * 2;
        outPlanes[index]     = -(rows[3] + rows[i]);  //0,2,4
        outPlanes[index + 1] = -(rows[3] - rows[i]);  //1,3,5
    }
    //https://www.gamedevs.org/uploads/fast-extraction-viewing-frustum-planes-from-world-view-projection-matrix.pdf
}
glm_quat Math::toGLM(const btQuaternion& q){
    return glm_quat{ q.getW(), q.getX(), q.getY(), q.getZ() };
}
btQuaternion Math::toBT(const glm_quat& q){
    return btQuaternion{ q.x, q.y, q.z, q.w };
}
glm::vec3 Math::toGLM(const aiVector3D& n){
    return glm::vec3{ n.x, n.y, n.z };
}
glm::mat4 Math::toGLM(const aiMatrix4x4& n){
    return glm::mat4{ n.a1, n.b1, n.c1, n.d1, n.a2, n.b2, n.c2, n.d2, n.a3, n.b3, n.c3, n.d3, n.a4, n.b4, n.c4, n.d4 };
}
glm::mat3 Math::toGLM(const aiMatrix3x3& n){
    return glm::mat3{ n.a1, n.b1, n.c1, n.a2, n.b2, n.c2, n.a3, n.b3, n.c3 };
}
glm::quat Math::toGLM(const aiQuaternion& aiQuat) {
    return glm::quat{ aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z };
}

glm_vec3 Math::toGLM(const btVector3& btvector){
    return glm_vec3{ btvector.x(), btvector.y(), btvector.z() };
}
btVector3 Math::toBT(const glm_vec3& vector){ 
    return btVector3{ vector.x, vector.y, vector.z };
}
glm_vec3 Math::getMatrixPosition(const glm_mat4& matrix) {
    return glm_vec3{ matrix[3][0], matrix[3][1], matrix[3][2] };
}
glm::vec3 Math::getMatrixPosition(const glm::mat4& matrix) {
    return glm::vec3{ matrix[3][0], matrix[3][1], matrix[3][2] };
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
glm::vec3 Math::getScreenCoordinates(
const glm::vec3& position, 
const Camera& camera, 
const glm::mat4& view, 
const glm::mat4& projection, 
const glm::vec4& viewport, 
bool clampToEdge
){
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

    auto screen_pos = glm::project(position, view, projection, viewport);
    float dot       = glm::dot(camera.getViewVector(), position - glm::vec3(camera.getPosition()));
    if (clampToEdge) {
        if (screen_pos.x >= viewport.x && screen_pos.x <= viewport.z) {
            if (screen_pos.y >= viewport.y && screen_pos.y <= viewport.w) {
                if (dot < 0.0f) { //negative dot means infront
                    inBounds = 1;
                }
            }
        }
    }else{
        if (dot < 0.0f) { //negative dot means infront
            inBounds = 1;
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
    glm::vec2 winSize{ Engine::Resources::getWindowSize() };
    glm::vec4 viewport{ 0.0f, 0.0f, winSize.x, winSize.y };
    return Math::getScreenCoordinates(objPos, camera, viewport, clampToEdge);
}



void Math::translate(const btRigidBody& body, btVector3& vec, bool local) noexcept {
    if (local) {
        btQuaternion q = body.getWorldTransform().getRotation().normalize();
        vec = vec.rotate(q.getAxis(), q.getAngle());
    }
}
glm::vec3 Math::midpoint(const glm::vec3& a, const glm::vec3& b) {
    return glm::vec3{ (a.x + b.x) / 2.0f, (a.y + b.y) / 2.0f, (a.z + b.z) / 2.0f };
}
glm::vec3 Math::direction(const glm::vec3& eye, const glm::vec3& target) {
    return glm::normalize(eye - target);
}
glm::vec3 Math::getForward(const glm_quat& q) {
    return glm::normalize(q * glm_vec3{ 0, 0, -1 });
}
glm::vec3 Math::getRight(const glm_quat& q) {
    return glm::normalize(q * glm_vec3{ 1, 0, 0 });
}
glm::vec3 Math::getUp(const glm_quat& q) {
    return glm::normalize(q * glm_vec3{ 0, 1, 0 });
}
glm::vec3 Math::getColumnVector(const btRigidBody& b, unsigned int column) {
    btTransform t;
    b.getMotionState()->getWorldTransform(t);
    btVector3 v = t.getBasis().getColumn(column);
    return glm::normalize(glm::vec3{ v.x(), v.y(), v.z() });
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
void Math::recalculateForwardRightUp(const glm_quat& o, glm_vec3& f, glm_vec3& r, glm_vec3& u) {
    f = Math::getForward(o);
    r = Math::getRight(o);
    u = Math::getUp(o);
}
void Math::recalculateForwardRightUp(const btRigidBody& b, glm_vec3& f, glm_vec3& r, glm_vec3& u) {
    f = Math::getForward(b);
    r = Math::getRight(b);
    u = Math::getUp(b);
}
float Math::getAngleBetweenTwoVectors(const glm::vec3& a, const glm::vec3& b, bool degrees) {
    if (a == b)
        return 0.0f;
    float angle = glm::acos(glm::dot(a, b));
    if (degrees)
        angle *= 57.2958f;
    return angle;
}
glm_quat Math::alignTo(decimal x, decimal y, decimal z) noexcept {
    glm_quat o{ 1.0, 0.0, 0.0, 0.0 };
    o = glm::conjugate(glm::toQuat(glm::lookAt(glm_vec3{ 0.0f }, -glm_vec3{ x, y, z }, glm_vec3{ 0, 1, 0 })));
    o = glm::normalize(o);
    return o;
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
    glm::vec4 output{ 0.0f };
    float alpha = paint_color.a + canvas_color.a * (1.0f - paint_color.a);
    output      = ((paint_color * paint_color.a + canvas_color * canvas_color.a * (1.0f - paint_color.a)) / alpha);
    output.a    = alpha;
    return output;
}
sf::Color Math::PaintersAlgorithm(const sf::Color& paint_color, const sf::Color& canvas_color) {
    glm::vec4 cC{ (float)canvas_color.r / 255.0f, (float)canvas_color.g / 255.0f, (float)canvas_color.b / 255.0f, (float)canvas_color.a / 255.0f };
    glm::vec4 pC{ (float)paint_color.r / 255.0f, (float)paint_color.g / 255.0f, (float)paint_color.b / 255.0f, (float)paint_color.a / 255.0f };
    float alpha  = pC.a + cC.a * (1.0f - pC.a);
    glm::vec4 calculations(0.0f);
    calculations = (pC * pC.a + cC * cC.a * (1.0f - pC.a) / alpha);
    calculations.a = alpha;
    sf::Uint8 finalR = (sf::Uint8)(calculations.r * 255.0f);
    sf::Uint8 finalG = (sf::Uint8)(calculations.g * 255.0f);
    sf::Uint8 finalB = (sf::Uint8)(calculations.b * 255.0f);
    sf::Uint8 finalA = (sf::Uint8)(calculations.a * 255.0f);
    return sf::Color{ finalR, finalG, finalB, finalA };
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
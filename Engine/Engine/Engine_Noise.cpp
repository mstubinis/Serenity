#include "Engine_Noise.h"
#include <cstdint>

using namespace Engine::Math;
using namespace std;

const double Noise::Detail::MathNoiseManagement::STRETCH_CONSTANT_2D = -0.211324865405187;
const double Noise::Detail::MathNoiseManagement::SQUISH_CONSTANT_2D = 0.366025403784439;
const double Noise::Detail::MathNoiseManagement::STRETCH_CONSTANT_3D = -1.0 / 6.0;
const double Noise::Detail::MathNoiseManagement::SQUISH_CONSTANT_3D = 1.0 / 3.0;
const double Noise::Detail::MathNoiseManagement::STRETCH_CONSTANT_4D = -0.138196601125011;
const double Noise::Detail::MathNoiseManagement::SQUISH_CONSTANT_4D = 0.309016994374947;

const double Noise::Detail::MathNoiseManagement::NORM_CONSTANT_2D = 47.0;
const double Noise::Detail::MathNoiseManagement::NORM_CONSTANT_3D = 103.0;
const double Noise::Detail::MathNoiseManagement::NORM_CONSTANT_4D = 30.0;

vector<short> Noise::Detail::MathNoiseManagement::perm;
vector<short> Noise::Detail::MathNoiseManagement::permGradIndex3D;

vector<glm::ivec2> Noise::Detail::MathNoiseManagement::grad2 = [](){
    vector<glm::ivec2> _v;
    _v.push_back(glm::ivec2(5,2)); _v.push_back(glm::ivec2(2,5));
    _v.push_back(glm::ivec2(-5,2)); _v.push_back(glm::ivec2(-2,5));
    _v.push_back(glm::ivec2(5,-2)); _v.push_back(glm::ivec2(2,-5));
    _v.push_back(glm::ivec2(-5,-2)); _v.push_back(glm::ivec2(-2,-5));
    return _v;
}();
vector<glm::ivec3> Noise::Detail::MathNoiseManagement::grad3 = [](){
    vector<glm::ivec3> _v;
    _v.push_back(glm::ivec3(-11,  4,  4)); _v.push_back(glm::ivec3(-4,  11,  4));
    _v.push_back(glm::ivec3(-4,  4,  11)); _v.push_back(glm::ivec3(11,  4,  4));
    _v.push_back(glm::ivec3(4,  11,  4)); _v.push_back(glm::ivec3(4,  4,  11));
    _v.push_back(glm::ivec3(-11, -4,  4)); _v.push_back(glm::ivec3(-4, -11,  4));
    _v.push_back(glm::ivec3(-4, -4,  11)); _v.push_back(glm::ivec3(11, -4,  4));
    _v.push_back(glm::ivec3(4, -11,  4)); _v.push_back(glm::ivec3(4, -4,  11));
    _v.push_back(glm::ivec3(-11,  4, -4)); _v.push_back(glm::ivec3(-4,  11, -4));
    _v.push_back(glm::ivec3(-4,  4, -11)); _v.push_back(glm::ivec3(11,  4, -4));
    _v.push_back(glm::ivec3(4,  11, -4)); _v.push_back(glm::ivec3(4,  4, -11));
    _v.push_back(glm::ivec3(-11, -4, -4)); _v.push_back(glm::ivec3(-4, -11, -4));
    _v.push_back(glm::ivec3(-4, -4, -11)); _v.push_back(glm::ivec3(11, -4, -4));
    _v.push_back(glm::ivec3(4, -11, -4)); _v.push_back(glm::ivec3(4, -4, -11));
    return _v;
}();
vector<glm::ivec4> Noise::Detail::MathNoiseManagement::grad4 = [](){
    vector<glm::ivec4> _v;
    _v.push_back(glm::ivec4(3,  1,  1,  1)); _v.push_back(glm::ivec4(1,  3,  1,  1));
    _v.push_back(glm::ivec4(1,  1,  3,  1)); _v.push_back(glm::ivec4(1,  1,  1,  3));
    _v.push_back(glm::ivec4(-3,  1,  1,  1)); _v.push_back(glm::ivec4(-1,  3,  1,  1));
    _v.push_back(glm::ivec4(-1,  1,  3,  1)); _v.push_back(glm::ivec4(-1,  1,  1,  3));
    _v.push_back(glm::ivec4(3, -1,  1,  1)); _v.push_back(glm::ivec4(1, -3,  1,  1));
    _v.push_back(glm::ivec4(1, -1,  3,  1)); _v.push_back(glm::ivec4(1, -1,  1,  3));
    _v.push_back(glm::ivec4(-3, -1,  1,  1)); _v.push_back(glm::ivec4(-1, -3,  1,  1));
    _v.push_back(glm::ivec4(-1, -1,  3,  1)); _v.push_back(glm::ivec4(-1, -1,  1,  3));
    _v.push_back(glm::ivec4(3,  1, -1,  1)); _v.push_back(glm::ivec4(1,  3, -1,  1));
    _v.push_back(glm::ivec4(1,  1, -3,  1)); _v.push_back(glm::ivec4(1,  1, -1,  3));
    _v.push_back(glm::ivec4(-3,  1, -1,  1)); _v.push_back(glm::ivec4(-1,  3, -1,  1));
    _v.push_back(glm::ivec4(-1,  1, -3,  1)); _v.push_back(glm::ivec4(-1,  1, -1,  3));
    _v.push_back(glm::ivec4(3, -1, -1,  1)); _v.push_back(glm::ivec4(1, -3, -1,  1));
    _v.push_back(glm::ivec4(1, -1, -3,  1)); _v.push_back(glm::ivec4(1, -1, -1,  3));
    _v.push_back(glm::ivec4(-3, -1, -1,  1)); _v.push_back(glm::ivec4(-1, -3, -1,  1));
    _v.push_back(glm::ivec4(-1, -1, -3,  1)); _v.push_back(glm::ivec4(-1, -1, -1,  3));
    _v.push_back(glm::ivec4(3,  1,  1, -1)); _v.push_back(glm::ivec4(1,  3,  1, -1));
    _v.push_back(glm::ivec4(1,  1,  3, -1)); _v.push_back(glm::ivec4(1,  1,  1, -3));
    _v.push_back(glm::ivec4(-3,  1,  1, -1)); _v.push_back(glm::ivec4(-1,  3,  1, -1));
    _v.push_back(glm::ivec4(-1,  1,  3, -1)); _v.push_back(glm::ivec4(-1,  1,  1, -3));
    _v.push_back(glm::ivec4(3, -1,  1, -1)); _v.push_back(glm::ivec4(1, -3,  1, -1));
    _v.push_back(glm::ivec4(1, -1,  3, -1)); _v.push_back(glm::ivec4(1, -1,  1, -3));
    _v.push_back(glm::ivec4(-3, -1,  1, -1)); _v.push_back(glm::ivec4(-1, -3,  1, -1));
    _v.push_back(glm::ivec4(-1, -1,  3, -1)); _v.push_back(glm::ivec4(-1, -1,  1, -3));
    _v.push_back(glm::ivec4(3,  1, -1, -1)); _v.push_back(glm::ivec4(1,  3, -1, -1));
    _v.push_back(glm::ivec4(1,  1, -3, -1)); _v.push_back(glm::ivec4(1,  1, -1, -3));
    _v.push_back(glm::ivec4(-3,  1, -1, -1)); _v.push_back(glm::ivec4(-1,  3, -1, -1));
    _v.push_back(glm::ivec4(-1,  1, -3, -1)); _v.push_back(glm::ivec4(-1,  1, -1, -3));
    _v.push_back(glm::ivec4(3, -1, -1, -1)); _v.push_back(glm::ivec4(1, -3, -1, -1));
    _v.push_back(glm::ivec4(1, -1, -3, -1)); _v.push_back(glm::ivec4(1, -1, -1, -3));
    _v.push_back(glm::ivec4(-3, -1, -1, -1)); _v.push_back(glm::ivec4(-1, -3, -1, -1));
    _v.push_back(glm::ivec4(-1, -1, -3, -1)); _v.push_back(glm::ivec4(-1, -1, -1, -3));
    return _v;
}();

void Noise::Detail::MathNoiseManagement::_initFromSeed(unsigned long long seed){
    Noise::Detail::MathNoiseManagement::perm.clear(); Noise::Detail::MathNoiseManagement::perm.resize(256);
    Noise::Detail::MathNoiseManagement::permGradIndex3D.clear(); Noise::Detail::MathNoiseManagement::permGradIndex3D.resize(256);
    vector<short> source; source.resize(256);
    for (short i = 0; i < 256; ++i)
        source[i] = i;
    seed = seed * (6364136223846793005LL) + (1442695040888963407LL);
    seed = seed * (6364136223846793005LL) + (1442695040888963407LL);
    seed = seed * (6364136223846793005LL) + (1442695040888963407LL);
    for (int i = 255; i >= 0; i--) {
        seed = seed * (6364136223846793005LL) + (1442695040888963407LL);
        int r = (int)((seed + 31) % (i + 1));
        if (r < 0)
            r += (i + 1);
        Noise::Detail::MathNoiseManagement::perm[i] = source[r];
        Noise::Detail::MathNoiseManagement::permGradIndex3D[i] = (short)((Noise::Detail::MathNoiseManagement::perm[i] % (Noise::Detail::MathNoiseManagement::grad3.size() / 3)) * 3);
        source[r] = source[i];
    }
}

double Noise::extrapolate(int xsb, int ysb, double dx, double dy){
    int index = Noise::Detail::MathNoiseManagement::perm[(Noise::Detail::MathNoiseManagement::perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
    return Detail::MathNoiseManagement::grad2[index].x * dx + Detail::MathNoiseManagement::grad2[index].y * dy;
}   
double Noise::extrapolate(int xsb, int ysb, int zsb, double dx, double dy, double dz){
    int index = Noise::Detail::MathNoiseManagement::permGradIndex3D[(Noise::Detail::MathNoiseManagement::perm[(Noise::Detail::MathNoiseManagement::perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF];
    return Detail::MathNoiseManagement::grad3[index].x * dx + Detail::MathNoiseManagement::grad3[index].y * dy + Detail::MathNoiseManagement::grad3[index].z * dz;
}   
double Noise::extrapolate(int xsb, int ysb, int zsb, int wsb, double dx, double dy, double dz, double dw){
    int index = Noise::Detail::MathNoiseManagement::perm[(Noise::Detail::MathNoiseManagement::perm[(Noise::Detail::MathNoiseManagement::perm[(Noise::Detail::MathNoiseManagement::perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF] + wsb) & 0xFF] & 0xFC;
    return Detail::MathNoiseManagement::grad4[index].x * dx + Detail::MathNoiseManagement::grad4[index].y * dy + Detail::MathNoiseManagement::grad4[index].z * dz + Detail::MathNoiseManagement::grad4[index].w * dw;
}

double Noise::_2D::noiseOpenSimplex2D(double x, double y){
    double stretchOffset = (x + y) * Detail::MathNoiseManagement::STRETCH_CONSTANT_2D;
    double xs = x + stretchOffset;
    double ys = y + stretchOffset; 
    int xsb = int(glm::floor(xs));
    int ysb = int(glm::floor(ys));
    double squishOffset = (xsb + ysb) * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
    double xb = xsb + squishOffset;
    double yb = ysb + squishOffset;
    double xins = xs - xsb;
    double yins = ys - ysb;
    double inSum = xins + yins;
    double dx0 = x - xb;
    double dy0 = y - yb;
    double dx_ext, dy_ext;
    int xsv_ext, ysv_ext;    
    double value = 0;
    double dx1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
    double dy1 = dy0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
    double attn1 = 2 - dx1 * dx1 - dy1 * dy1;
    if (attn1 > 0) {
        attn1 *= attn1;
        value += attn1 * attn1 * extrapolate(xsb + 1, ysb + 0, dx1, dy1);
    }
    double dx2 = dx0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
    double dy2 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
    double attn2 = 2 - dx2 * dx2 - dy2 * dy2;
    if (attn2 > 0) {
        attn2 *= attn2;
        value += attn2 * attn2 * extrapolate(xsb + 0, ysb + 1, dx2, dy2);
    }    
    if (inSum <= 1) {
        double zins = 1 - inSum;
        if (zins > xins || zins > yins) {
            if (xins > yins) {
                xsv_ext = xsb + 1;
                ysv_ext = ysb - 1;
                dx_ext = dx0 - 1;
                dy_ext = dy0 + 1;
            } else {
                xsv_ext = xsb - 1;
                ysv_ext = ysb + 1;
                dx_ext = dx0 + 1;
                dy_ext = dy0 - 1;
            }
        } else {
            xsv_ext = xsb + 1;
            ysv_ext = ysb + 1;
            dx_ext = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
            dy_ext = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
        }
    } else {
        double zins = 2 - inSum;
        if (zins < xins || zins < yins) {
            if (xins > yins) {
                xsv_ext = xsb + 2;
                ysv_ext = ysb + 0;
                dx_ext = dx0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
                dy_ext = dy0 + 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
            } else {
                xsv_ext = xsb + 0;
                ysv_ext = ysb + 2;
                dx_ext = dx0 + 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
                dy_ext = dy0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
            }
        } else {
            dx_ext = dx0;
            dy_ext = dy0;
            xsv_ext = xsb;
            ysv_ext = ysb;
        }
        xsb += 1;
        ysb += 1;
        dx0 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
        dy0 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_2D;
    }
    double attn0 = 2 - dx0 * dx0 - dy0 * dy0;
    if (attn0 > 0) {
        attn0 *= attn0;
        value += attn0 * attn0 * extrapolate(xsb, ysb, dx0, dy0);
    }
    double attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
    if (attn_ext > 0) {
        attn_ext *= attn_ext;
        value += attn_ext * attn_ext * extrapolate(xsv_ext, ysv_ext, dx_ext, dy_ext);
    }    
    return value / Detail::MathNoiseManagement::NORM_CONSTANT_2D;
}
double Noise::_3D::noiseOpenSimplex3D(double x, double y, double z){
    double stretchOffset = (x + y + z) * Detail::MathNoiseManagement::STRETCH_CONSTANT_3D;
    double xs = x + stretchOffset;
    double ys = y + stretchOffset;
    double zs = z + stretchOffset;
    int xsb = int(glm::floor(xs));
    int ysb = int(glm::floor(ys));
    int zsb = int(glm::floor(zs));
    double squishOffset = (xsb + ysb + zsb) * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
    double xb = xsb + squishOffset;
    double yb = ysb + squishOffset;
    double zb = zsb + squishOffset;
    double xins = xs - xsb;
    double yins = ys - ysb;
    double zins = zs - zsb;
    double inSum = xins + yins + zins;
    double dx0 = x - xb;
    double dy0 = y - yb;
    double dz0 = z - zb;
    double dx_ext0, dy_ext0, dz_ext0;
    double dx_ext1, dy_ext1, dz_ext1;
    int xsv_ext0, ysv_ext0, zsv_ext0;
    int xsv_ext1, ysv_ext1, zsv_ext1;  
    double value = 0;
    if (inSum <= 1) {
        int8_t aPoint = 0x01;
        double aScore = xins;
        int8_t bPoint = 0x02;
        double bScore = yins;
        if (aScore >= bScore && zins > bScore) {
            bScore = zins;
            bPoint = 0x04;
        } else if (aScore < bScore && zins > aScore) {
            aScore = zins;
            aPoint = 0x04;
        }
        double wins = 1 - inSum;
        if (wins > aScore || wins > bScore) {
            int8_t c = (bScore > aScore ? bPoint : aPoint);
            if ((c & 0x01) == 0) {
                xsv_ext0 = xsb - 1;
                xsv_ext1 = xsb;
                dx_ext0 = dx0 + 1;
                dx_ext1 = dx0;
            } else {
                xsv_ext0 = xsv_ext1 = xsb + 1;
                dx_ext0 = dx_ext1 = dx0 - 1;
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0;
                if ((c & 0x01) == 0) {
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                } else {
                    ysv_ext0 -= 1;
                    dy_ext0 += 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1;
            }
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsb;
                zsv_ext1 = zsb - 1;
                dz_ext0 = dz0;
                dz_ext1 = dz0 + 1;
            } else {
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz_ext1 = dz0 - 1;
            }
        } else {
            int8_t c = (int8_t)(aPoint | bPoint);     
            if ((c & 0x01) == 0) {
                xsv_ext0 = xsb;
                xsv_ext1 = xsb - 1;
                dx_ext0 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dx_ext1 = dx0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                xsv_ext0 = xsv_ext1 = xsb + 1;
                dx_ext0 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dx_ext1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysb;
                ysv_ext1 = ysb - 1;
                dy_ext0 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext1 = dy0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext1 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsb;
                zsv_ext1 = zsb - 1;
                dz_ext0 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext1 = dz0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext1 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
        }
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * extrapolate(xsb + 0, ysb + 0, zsb + 0, dx0, dy0, dz0);
        }
        double dx1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy1 = dy0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz1 = dz0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 1, ysb + 0, zsb + 0, dx1, dy1, dz1);
        }
        double dx2 = dx0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy2 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz2 = dz1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 0, ysb + 1, zsb + 0, dx2, dy2, dz2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 0, ysb + 0, zsb + 1, dx3, dy3, dz3);
        }
    } else if (inSum >= 2) {
        int8_t aPoint = 0x06;
        double aScore = xins;
        int8_t bPoint = 0x05;
        double bScore = yins;
        if (aScore <= bScore && zins < bScore) {
            bScore = zins;
            bPoint = 0x03;
        } else if (aScore > bScore && zins < aScore) {
            aScore = zins;
            aPoint = 0x03;
        }
        double wins = 3 - inSum;
        if (wins < aScore || wins < bScore) {
            int8_t c = (bScore < aScore ? bPoint : aPoint);  
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsb + 2;
                xsv_ext1 = xsb + 1;
                dx_ext0 = dx0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dx_ext1 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                xsv_ext0 = xsv_ext1 = xsb;
                dx_ext0 = dx_ext1 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                if ((c & 0x01) != 0) {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                } else {
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsb + 1;
                zsv_ext1 = zsb + 2;
                dz_ext0 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext1 = dz0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz_ext1 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
        } else {
            int8_t c = (int8_t)(aPoint & bPoint); 
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsb + 1;
                xsv_ext1 = xsb + 2;
                dx_ext0 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dx_ext1 = dx0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                xsv_ext0 = xsv_ext1 = xsb;
                dx_ext0 = dx0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dx_ext1 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysb + 1;
                ysv_ext1 = ysb + 2;
                dy_ext0 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext1 = dy0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext1 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsb + 1;
                zsv_ext1 = zsb + 2;
                dz_ext0 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext1 = dz0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            } else {
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext1 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            }
        }
        double dx3 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy3 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz3 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 1, ysb + 1, zsb + 0, dx3, dy3, dz3);
        }
        double dx2 = dx3;
        double dy2 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz2 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 1, ysb + 0, zsb + 1, dx2, dy2, dz2);
        }
        double dx1 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy1 = dy3;
        double dz1 = dz2;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 0, ysb + 1, zsb + 1, dx1, dy1, dz1);
        }
        dx0 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        dy0 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        dz0 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * extrapolate(xsb + 1, ysb + 1, zsb + 1, dx0, dy0, dz0);
        }
    } else {
        double aScore;
        int8_t aPoint;
        bool aIsFurtherSide;
        double bScore;
        int8_t bPoint;
        bool bIsFurtherSide;
        double p1 = xins + yins;
        if (p1 > 1) {
            aScore = p1 - 1;
            aPoint = 0x03;
            aIsFurtherSide = true;
        } else {
            aScore = 1 - p1;
            aPoint = 0x04;
            aIsFurtherSide = false;
        }
        double p2 = xins + zins;
        if (p2 > 1) {
            bScore = p2 - 1;
            bPoint = 0x05;
            bIsFurtherSide = true;
        } else {
            bScore = 1 - p2;
            bPoint = 0x02;
            bIsFurtherSide = false;
        }
        double p3 = yins + zins;
        if (p3 > 1) {
            double score = p3 - 1;
            if (aScore <= bScore && aScore < score) {
                aScore = score;
                aPoint = 0x06;
                aIsFurtherSide = true;
            } else if (aScore > bScore && bScore < score) {
                bScore = score;
                bPoint = 0x06;
                bIsFurtherSide = true;
            }
        } else {
            double score = 1 - p3;
            if (aScore <= bScore && aScore < score) {
                aScore = score;
                aPoint = 0x01;
                aIsFurtherSide = false;
            } else if (aScore > bScore && bScore < score) {
                bScore = score;
                bPoint = 0x01;
                bIsFurtherSide = false;
            }
        }
        if (aIsFurtherSide == bIsFurtherSide) {
            if (aIsFurtherSide) {
                dx_ext0 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext0 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext0 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                xsv_ext0 = xsb + 1;
                ysv_ext0 = ysb + 1;
                zsv_ext0 = zsb + 1;
                int8_t c = (int8_t)(aPoint & bPoint);
                if ((c & 0x01) != 0) {
                    dx_ext1 = dx0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dy_ext1 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dz_ext1 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    xsv_ext1 = xsb + 2;
                    ysv_ext1 = ysb;
                    zsv_ext1 = zsb;
                } else if ((c & 0x02) != 0) {
                    dx_ext1 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dy_ext1 = dy0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dz_ext1 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    xsv_ext1 = xsb;
                    ysv_ext1 = ysb + 2;
                    zsv_ext1 = zsb;
                } else {
                    dx_ext1 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dy_ext1 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dz_ext1 = dz0 - 2 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    xsv_ext1 = xsb;
                    ysv_ext1 = ysb;
                    zsv_ext1 = zsb + 2;
                }
            } else {
                dx_ext0 = dx0;
                dy_ext0 = dy0;
                dz_ext0 = dz0;
                xsv_ext0 = xsb;
                ysv_ext0 = ysb;
                zsv_ext0 = zsb;
                int8_t c = (int8_t)(aPoint | bPoint);
                if ((c & 0x01) == 0) {
                    dx_ext1 = dx0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dy_ext1 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dz_ext1 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    xsv_ext1 = xsb - 1;
                    ysv_ext1 = ysb + 1;
                    zsv_ext1 = zsb + 1;
                } else if ((c & 0x02) == 0) {
                    dx_ext1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dy_ext1 = dy0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dz_ext1 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    xsv_ext1 = xsb + 1;
                    ysv_ext1 = ysb - 1;
                    zsv_ext1 = zsb + 1;
                } else {
                    dx_ext1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dy_ext1 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    dz_ext1 = dz0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                    xsv_ext1 = xsb + 1;
                    ysv_ext1 = ysb + 1;
                    zsv_ext1 = zsb - 1;
                }
            }
        } else {
            int8_t c1, c2;
            if (aIsFurtherSide) {
                c1 = aPoint;
                c2 = bPoint;
            } else {
                c1 = bPoint;
                c2 = aPoint;
            }
            if ((c1 & 0x01) == 0) {
                dx_ext0 = dx0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext0 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext0 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                xsv_ext0 = xsb - 1;
                ysv_ext0 = ysb + 1;
                zsv_ext0 = zsb + 1;
            } else if ((c1 & 0x02) == 0) {
                dx_ext0 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext0 = dy0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext0 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                xsv_ext0 = xsb + 1;
                ysv_ext0 = ysb - 1;
                zsv_ext0 = zsb + 1;
            } else {
                dx_ext0 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dy_ext0 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                dz_ext0 = dz0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
                xsv_ext0 = xsb + 1;
                ysv_ext0 = ysb + 1;
                zsv_ext0 = zsb - 1;
            }
            dx_ext1 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            dy_ext1 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            dz_ext1 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
            xsv_ext1 = xsb;
            ysv_ext1 = ysb;
            zsv_ext1 = zsb;
            if ((c2 & 0x01) != 0) {
                dx_ext1 -= 2;
                xsv_ext1 += 2;
            } else if ((c2 & 0x02) != 0) {
                dy_ext1 -= 2;
                ysv_ext1 += 2;
            } else {
                dz_ext1 -= 2;
                zsv_ext1 += 2;
            }
        }
        double dx1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy1 = dy0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz1 = dz0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 1, ysb + 0, zsb + 0, dx1, dy1, dz1);
        }
        double dx2 = dx0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy2 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz2 = dz1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 0, ysb + 1, zsb + 0, dx2, dy2, dz2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 0, ysb + 0, zsb + 1, dx3, dy3, dz3);
        }
        double dx4 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy4 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz4 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * extrapolate(xsb + 1, ysb + 1, zsb + 0, dx4, dy4, dz4);
        }
        double dx5 = dx4;
        double dy5 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dz5 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double attn5 = 2 - dx5 * dx5 - dy5 * dy5 - dz5 * dz5;
        if (attn5 > 0) {
            attn5 *= attn5;
            value += attn5 * attn5 * extrapolate(xsb + 1, ysb + 0, zsb + 1, dx5, dy5, dz5);
        }
        double dx6 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_3D;
        double dy6 = dy4;
        double dz6 = dz5;
        double attn6 = 2 - dx6 * dx6 - dy6 * dy6 - dz6 * dz6;
        if (attn6 > 0) {
            attn6 *= attn6;
            value += attn6 * attn6 * extrapolate(xsb + 0, ysb + 1, zsb + 1, dx6, dy6, dz6);
        }
    }
    double attn_ext0 = 2 - dx_ext0 * dx_ext0 - dy_ext0 * dy_ext0 - dz_ext0 * dz_ext0;
    if (attn_ext0 > 0){
        attn_ext0 *= attn_ext0;
        value += attn_ext0 * attn_ext0 * extrapolate(xsv_ext0, ysv_ext0, zsv_ext0, dx_ext0, dy_ext0, dz_ext0);
    }
    double attn_ext1 = 2 - dx_ext1 * dx_ext1 - dy_ext1 * dy_ext1 - dz_ext1 * dz_ext1;
    if (attn_ext1 > 0){
        attn_ext1 *= attn_ext1;
        value += attn_ext1 * attn_ext1 * extrapolate(xsv_ext1, ysv_ext1, zsv_ext1, dx_ext1, dy_ext1, dz_ext1);
    }   
    return value / Detail::MathNoiseManagement::NORM_CONSTANT_3D;
}
double Noise::_4D::noiseOpenSimplex4D(double x, double y, double z, double w){
    double stretchOffset = (x + y + z + w) * Detail::MathNoiseManagement::STRETCH_CONSTANT_4D;
    double xs = x + stretchOffset;
    double ys = y + stretchOffset;
    double zs = z + stretchOffset;
    double ws = w + stretchOffset;
    int xsb = int(glm::floor(xs));
    int ysb = int(glm::floor(ys));
    int zsb = int(glm::floor(zs));
    int wsb = int(glm::floor(ws));
    double squishOffset = (xsb + ysb + zsb + wsb) * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
    double xb = xsb + squishOffset;
    double yb = ysb + squishOffset;
    double zb = zsb + squishOffset;
    double wb = wsb + squishOffset;
    double xins = xs - xsb;
    double yins = ys - ysb;
    double zins = zs - zsb;
    double wins = ws - wsb;
    double inSum = xins + yins + zins + wins;
    double dx0 = x - xb;
    double dy0 = y - yb;
    double dz0 = z - zb;
    double dw0 = w - wb;
    double dx_ext0, dy_ext0, dz_ext0, dw_ext0;
    double dx_ext1, dy_ext1, dz_ext1, dw_ext1;
    double dx_ext2, dy_ext2, dz_ext2, dw_ext2;
    int xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0;
    int xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1;
    int xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2;       
    double value = 0;
    if (inSum <= 1) {
        int8_t aPoint = 0x01;
        double aScore = xins;
        int8_t bPoint = 0x02;
        double bScore = yins;
        if (aScore >= bScore && zins > bScore) {
            bScore = zins;
            bPoint = 0x04;
        } else if (aScore < bScore && zins > aScore) {
            aScore = zins;
            aPoint = 0x04;
        }
        if (aScore >= bScore && wins > bScore) {
            bScore = wins;
            bPoint = 0x08;
        } else if (aScore < bScore && wins > aScore) {
            aScore = wins;
            aPoint = 0x08;
        }
        double uins = 1 - inSum;
        if (uins > aScore || uins > bScore) {
            int8_t c = (bScore > aScore ? bPoint : aPoint);
            if ((c & 0x01) == 0) {
                xsv_ext0 = xsb - 1;
                xsv_ext1 = xsv_ext2 = xsb;
                dx_ext0 = dx0 + 1;
                dx_ext1 = dx_ext2 = dx0;
            } else {
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb + 1;
                dx_ext0 = dx_ext1 = dx_ext2 = dx0 - 1;
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0;
                if ((c & 0x01) == 0x01) {
                    ysv_ext0 -= 1;
                    dy_ext0 += 1;
                } else {
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0 - 1;
            }   
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0;
                if ((c & 0x03) != 0) {
                    if ((c & 0x03) == 0x03) {
                        zsv_ext0 -= 1;
                        dz_ext0 += 1;
                    } else {
                        zsv_ext1 -= 1;
                        dz_ext1 += 1;
                    }
                } else {
                    zsv_ext2 -= 1;
                    dz_ext2 += 1;
                }
            } else {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0 - 1;
            }  
            if ((c & 0x08) == 0) {
                wsv_ext0 = wsv_ext1 = wsb;
                wsv_ext2 = wsb - 1;
                dw_ext0 = dw_ext1 = dw0;
                dw_ext2 = dw0 + 1;
            } else {
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb + 1;
                dw_ext0 = dw_ext1 = dw_ext2 = dw0 - 1;
            }
        } else {
            int8_t c = (int8_t)(aPoint | bPoint);
            if ((c & 0x01) == 0) {
                xsv_ext0 = xsv_ext2 = xsb;
                xsv_ext1 = xsb - 1;
                dx_ext0 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext2 = dx0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb + 1;
                dx_ext0 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx_ext2 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext1 = dy_ext2 = dy0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c & 0x01) == 0x01) {
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                } else {
                    ysv_ext2 -= 1;
                    dy_ext2 += 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext1 = dy_ext2 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext1 = dz_ext2 = dz0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c & 0x03) == 0x03) {
                    zsv_ext1 -= 1;
                    dz_ext1 += 1;
                } else {
                    zsv_ext2 -= 1;
                    dz_ext2 += 1;
                }
            } else {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext1 = dz_ext2 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c & 0x08) == 0) {
                wsv_ext0 = wsv_ext1 = wsb;
                wsv_ext2 = wsb - 1;
                dw_ext0 = dw0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext2 = dw0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb + 1;
                dw_ext0 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw_ext2 = dw0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
        }
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0 - dw0 * dw0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * extrapolate(xsb + 0, ysb + 0, zsb + 0, wsb + 0, dx0, dy0, dz0, dw0);
        }
        double dx1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy1 = dy0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz1 = dz0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw1 = dw0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 0, dx1, dy1, dz1, dw1);
        }
        double dx2 = dx0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy2 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz2 = dz1;
        double dw2 = dw1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 0, dx2, dy2, dz2, dw2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw3 = dw1;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 0, dx3, dy3, dz3, dw3);
        }
        double dx4 = dx2;
        double dy4 = dy1;
        double dz4 = dz1;
        double dw4 = dw0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * extrapolate(xsb + 0, ysb + 0, zsb + 0, wsb + 1, dx4, dy4, dz4, dw4);
        }
    } else if (inSum >= 3) {
        int8_t aPoint = 0x0E;
        double aScore = xins;
        int8_t bPoint = 0x0D;
        double bScore = yins;
        if (aScore <= bScore && zins < bScore) {
            bScore = zins;
            bPoint = 0x0B;
        } else if (aScore > bScore && zins < aScore) {
            aScore = zins;
            aPoint = 0x0B;
        }
        if (aScore <= bScore && wins < bScore) {
            bScore = wins;
            bPoint = 0x07;
        } else if (aScore > bScore && wins < aScore) {
            aScore = wins;
            aPoint = 0x07;
        }
        double uins = 4 - inSum;
        if (uins < aScore || uins < bScore) {
            int8_t c = (bScore < aScore ? bPoint : aPoint); 
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsb + 2;
                xsv_ext1 = xsv_ext2 = xsb + 1;
                dx_ext0 = dx0 - 2 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx_ext2 = dx0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb;
                dx_ext0 = dx_ext1 = dx_ext2 = dx0 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c & 0x01) != 0) {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                } else {
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }   
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c & 0x03) != 0x03) {
                    if ((c & 0x03) == 0) {
                        zsv_ext0 += 1;
                        dz_ext0 -= 1;
                    } else {
                        zsv_ext1 += 1;
                        dz_ext1 -= 1;
                    }
                } else {
                    zsv_ext2 += 1;
                    dz_ext2 -= 1;
                }
            } else {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c & 0x08) != 0) {
                wsv_ext0 = wsv_ext1 = wsb + 1;
                wsv_ext2 = wsb + 2;
                dw_ext0 = dw_ext1 = dw0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext2 = dw0 - 2 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb;
                dw_ext0 = dw_ext1 = dw_ext2 = dw0 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
        } else {
            int8_t c = (int8_t)(aPoint & bPoint); 
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsv_ext2 = xsb + 1;
                xsv_ext1 = xsb + 2;
                dx_ext0 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext2 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb;
                dx_ext0 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx_ext2 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext1 = dy_ext2 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c & 0x01) != 0) {
                    ysv_ext2 += 1;
                    dy_ext2 -= 1;
                } else {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext1 = dy_ext2 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext1 = dz_ext2 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c & 0x03) != 0) {
                    zsv_ext2 += 1;
                    dz_ext2 -= 1;
                } else {
                    zsv_ext1 += 1;
                    dz_ext1 -= 1;
                }
            } else {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext1 = dz_ext2 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c & 0x08) != 0) {
                wsv_ext0 = wsv_ext1 = wsb + 1;
                wsv_ext2 = wsb + 2;
                dw_ext0 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext2 = dw0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb;
                dw_ext0 = dw0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw_ext2 = dw0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
        }
        double dx4 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy4 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz4 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw4 = dw0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * extrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 0, dx4, dy4, dz4, dw4);
        }
        double dx3 = dx4;
        double dy3 = dy4;
        double dz3 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw3 = dw0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 1, dx3, dy3, dz3, dw3);
        }
        double dx2 = dx4;
        double dy2 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz2 = dz4;
        double dw2 = dw3;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 1, dx2, dy2, dz2, dw2);
        }
        double dx1 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz1 = dz4;
        double dy1 = dy4;
        double dw1 = dw3;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 1, dx1, dy1, dz1, dw1);
        }
        dx0 = dx0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        dy0 = dy0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        dz0 = dz0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        dw0 = dw0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0 - dw0 * dw0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * extrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 1, dx0, dy0, dz0, dw0);
        }
    } else if (inSum <= 2) {
        double aScore;
        int8_t aPoint;
        bool aIsBiggerSide = true;
        double bScore;
        int8_t bPoint;
        bool bIsBiggerSide = true;
        if (xins + yins > zins + wins) {
            aScore = xins + yins;
            aPoint = 0x03;
        } else {
            aScore = zins + wins;
            aPoint = 0x0C;
        }
        if (xins + zins > yins + wins) {
            bScore = xins + zins;
            bPoint = 0x05;
        } else {
            bScore = yins + wins;
            bPoint = 0x0A;
        }
        if (xins + wins > yins + zins) {
            double score = xins + wins;
            if (aScore >= bScore && score > bScore) {
                bScore = score;
                bPoint = 0x09;
            } else if (aScore < bScore && score > aScore) {
                aScore = score;
                aPoint = 0x09;
            }
        } else {
            double score = yins + zins;
            if (aScore >= bScore && score > bScore) {
                bScore = score;
                bPoint = 0x06;
            } else if (aScore < bScore && score > aScore) {
                aScore = score;
                aPoint = 0x06;
            }
        }
        double p1 = 2 - inSum + xins;
        if (aScore >= bScore && p1 > bScore) {
            bScore = p1;
            bPoint = 0x01;
            bIsBiggerSide = false;
        } else if (aScore < bScore && p1 > aScore) {
            aScore = p1;
            aPoint = 0x01;
            aIsBiggerSide = false;
        }
        double p2 = 2 - inSum + yins;
        if (aScore >= bScore && p2 > bScore) {
            bScore = p2;
            bPoint = 0x02;
            bIsBiggerSide = false;
        } else if (aScore < bScore && p2 > aScore) {
            aScore = p2;
            aPoint = 0x02;
            aIsBiggerSide = false;
        }
        double p3 = 2 - inSum + zins;
        if (aScore >= bScore && p3 > bScore) {
            bScore = p3;
            bPoint = 0x04;
            bIsBiggerSide = false;
        } else if (aScore < bScore && p3 > aScore) {
            aScore = p3;
            aPoint = 0x04;
            aIsBiggerSide = false;
        }
        double p4 = 2 - inSum + wins;
        if (aScore >= bScore && p4 > bScore) {
            bScore = p4;
            bPoint = 0x08;
            bIsBiggerSide = false;
        } else if (aScore < bScore && p4 > aScore) {
            aScore = p4;
            aPoint = 0x08;
            aIsBiggerSide = false;
        }
        if (aIsBiggerSide == bIsBiggerSide) {
            if (aIsBiggerSide) {
                int8_t c1 = (int8_t)(aPoint | bPoint);
                int8_t c2 = (int8_t)(aPoint & bPoint);
                if ((c1 & 0x01) == 0) {
                    xsv_ext0 = xsb;
                    xsv_ext1 = xsb - 1;
                    dx_ext0 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dx_ext1 = dx0 + 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    xsv_ext0 = xsv_ext1 = xsb + 1;
                    dx_ext0 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dx_ext1 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }    
                if ((c1 & 0x02) == 0) {
                    ysv_ext0 = ysb;
                    ysv_ext1 = ysb - 1;
                    dy_ext0 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dy_ext1 = dy0 + 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    ysv_ext0 = ysv_ext1 = ysb + 1;
                    dy_ext0 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dy_ext1 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }   
                if ((c1 & 0x04) == 0) {
                    zsv_ext0 = zsb;
                    zsv_ext1 = zsb - 1;
                    dz_ext0 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dz_ext1 = dz0 + 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    zsv_ext0 = zsv_ext1 = zsb + 1;
                    dz_ext0 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dz_ext1 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }
                if ((c1 & 0x08) == 0) {
                    wsv_ext0 = wsb;
                    wsv_ext1 = wsb - 1;
                    dw_ext0 = dw0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dw_ext1 = dw0 + 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    wsv_ext0 = wsv_ext1 = wsb + 1;
                    dw_ext0 = dw0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dw_ext1 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }
                xsv_ext2 = xsb;
                ysv_ext2 = ysb;
                zsv_ext2 = zsb;
                wsv_ext2 = wsb;
                dx_ext2 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext2 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext2 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext2 = dw0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c2 & 0x01) != 0) {
                    xsv_ext2 += 2;
                    dx_ext2 -= 2;
                } else if ((c2 & 0x02) != 0) {
                    ysv_ext2 += 2;
                    dy_ext2 -= 2;
                } else if ((c2 & 0x04) != 0) {
                    zsv_ext2 += 2;
                    dz_ext2 -= 2;
                } else {
                    wsv_ext2 += 2;
                    dw_ext2 -= 2;
                }
                    
            } else {
                xsv_ext2 = xsb;
                ysv_ext2 = ysb;
                zsv_ext2 = zsb;
                wsv_ext2 = wsb;
                dx_ext2 = dx0;
                dy_ext2 = dy0;
                dz_ext2 = dz0;
                dw_ext2 = dw0;
                int8_t c = (int8_t)(aPoint | bPoint);
                if ((c & 0x01) == 0) {
                    xsv_ext0 = xsb - 1;
                    xsv_ext1 = xsb;
                    dx_ext0 = dx0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dx_ext1 = dx0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    xsv_ext0 = xsv_ext1 = xsb + 1;
                    dx_ext0 = dx_ext1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }
                if ((c & 0x02) == 0) {
                    ysv_ext0 = ysv_ext1 = ysb;
                    dy_ext0 = dy_ext1 = dy0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    if ((c & 0x01) == 0x01)
                    {
                        ysv_ext0 -= 1;
                        dy_ext0 += 1;
                    } else {
                        ysv_ext1 -= 1;
                        dy_ext1 += 1;
                    }
                } else {
                    ysv_ext0 = ysv_ext1 = ysb + 1;
                    dy_ext0 = dy_ext1 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }    
                if ((c & 0x04) == 0) {
                    zsv_ext0 = zsv_ext1 = zsb;
                    dz_ext0 = dz_ext1 = dz0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    if ((c & 0x03) == 0x03){
                        zsv_ext0 -= 1;
                        dz_ext0 += 1;
                    } else {
                        zsv_ext1 -= 1;
                        dz_ext1 += 1;
                    }
                } else {
                    zsv_ext0 = zsv_ext1 = zsb + 1;
                    dz_ext0 = dz_ext1 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } 
                if ((c & 0x08) == 0){
                    wsv_ext0 = wsb;
                    wsv_ext1 = wsb - 1;
                    dw_ext0 = dw0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dw_ext1 = dw0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    wsv_ext0 = wsv_ext1 = wsb + 1;
                    dw_ext0 = dw_ext1 = dw0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } 
            }
        } else {
            int8_t c1, c2;
            if (aIsBiggerSide) {
                c1 = aPoint;
                c2 = bPoint;
            } else {
                c1 = bPoint;
                c2 = aPoint;
            }
            if ((c1 & 0x01) == 0) {
                xsv_ext0 = xsb - 1;
                xsv_ext1 = xsb;
                dx_ext0 = dx0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                xsv_ext0 = xsv_ext1 = xsb + 1;
                dx_ext0 = dx_ext1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }   
            if ((c1 & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c1 & 0x01) == 0x01) {
                    ysv_ext0 -= 1;
                    dy_ext0 += 1;
                } else {
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            if ((c1 & 0x04) == 0) {
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz_ext1 = dz0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c1 & 0x03) == 0x03) {
                    zsv_ext0 -= 1;
                    dz_ext0 += 1;
                } else {
                    zsv_ext1 -= 1;
                    dz_ext1 += 1;
                }
            } else {
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz_ext1 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            if ((c1 & 0x08) == 0) {
                wsv_ext0 = wsb;
                wsv_ext1 = wsb - 1;
                dw_ext0 = dw0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw0 + 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                wsv_ext0 = wsv_ext1 = wsb + 1;
                dw_ext0 = dw_ext1 = dw0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            xsv_ext2 = xsb;
            ysv_ext2 = ysb;
            zsv_ext2 = zsb;
            wsv_ext2 = wsb;
            dx_ext2 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            dy_ext2 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            dz_ext2 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            dw_ext2 = dw0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            if ((c2 & 0x01) != 0) {
                xsv_ext2 += 2;
                dx_ext2 -= 2;
            } else if ((c2 & 0x02) != 0) {
                ysv_ext2 += 2;
                dy_ext2 -= 2;
            } else if ((c2 & 0x04) != 0) {
                zsv_ext2 += 2;
                dz_ext2 -= 2;
            } else {
                wsv_ext2 += 2;
                dw_ext2 -= 2;
            }
        }
        double dx1 = dx0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy1 = dy0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz1 = dz0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw1 = dw0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 0, dx1, dy1, dz1, dw1);
        }
        double dx2 = dx0 - 0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy2 = dy0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz2 = dz1;
        double dw2 = dw1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 0, dx2, dy2, dz2, dw2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw3 = dw1;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 0, dx3, dy3, dz3, dw3);
        }
        double dx4 = dx2;
        double dy4 = dy1;
        double dz4 = dz1;
        double dw4 = dw0 - 1 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * extrapolate(xsb + 0, ysb + 0, zsb + 0, wsb + 1, dx4, dy4, dz4, dw4);
        }
        double dx5 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy5 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz5 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw5 = dw0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn5 = 2 - dx5 * dx5 - dy5 * dy5 - dz5 * dz5 - dw5 * dw5;
        if (attn5 > 0) {
            attn5 *= attn5;
            value += attn5 * attn5 * extrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 0, dx5, dy5, dz5, dw5);
        }
        double dx6 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy6 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz6 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw6 = dw0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn6 = 2 - dx6 * dx6 - dy6 * dy6 - dz6 * dz6 - dw6 * dw6;
        if (attn6 > 0) {
            attn6 *= attn6;
            value += attn6 * attn6 * extrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 0, dx6, dy6, dz6, dw6);
        }
        double dx7 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy7 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz7 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw7 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn7 = 2 - dx7 * dx7 - dy7 * dy7 - dz7 * dz7 - dw7 * dw7;
        if (attn7 > 0) {
            attn7 *= attn7;
            value += attn7 * attn7 * extrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 1, dx7, dy7, dz7, dw7);
        }
        double dx8 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy8 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz8 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw8 = dw0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn8 = 2 - dx8 * dx8 - dy8 * dy8 - dz8 * dz8 - dw8 * dw8;
        if (attn8 > 0) {
            attn8 *= attn8;
            value += attn8 * attn8 * extrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 0, dx8, dy8, dz8, dw8);
        }
        double dx9 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy9 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz9 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw9 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn9 = 2 - dx9 * dx9 - dy9 * dy9 - dz9 * dz9 - dw9 * dw9;
        if (attn9 > 0) {
            attn9 *= attn9;
            value += attn9 * attn9 * extrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 1, dx9, dy9, dz9, dw9);
        }
        double dx10 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy10 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz10 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw10 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn10 = 2 - dx10 * dx10 - dy10 * dy10 - dz10 * dz10 - dw10 * dw10;
        if (attn10 > 0) {
            attn10 *= attn10;
            value += attn10 * attn10 * extrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 1, dx10, dy10, dz10, dw10);
        }
    } else {
        double aScore;
        int8_t aPoint;
        bool aIsBiggerSide = true;
        double bScore;
        int8_t bPoint;
        bool bIsBiggerSide = true;
        if (xins + yins < zins + wins) {
            aScore = xins + yins;
            aPoint = 0x0C;
        } else {
            aScore = zins + wins;
            aPoint = 0x03;
        }
        if (xins + zins < yins + wins) {
            bScore = xins + zins;
            bPoint = 0x0A;
        } else {
            bScore = yins + wins;
            bPoint = 0x05;
        }
        if (xins + wins < yins + zins) {
            double score = xins + wins;
            if (aScore <= bScore && score < bScore) {
                bScore = score;
                bPoint = 0x06;
            } else if (aScore > bScore && score < aScore) {
                aScore = score;
                aPoint = 0x06;
            }
        } else {
            double score = yins + zins;
            if (aScore <= bScore && score < bScore) {
                bScore = score;
                bPoint = 0x09;
            } else if (aScore > bScore && score < aScore) {
                aScore = score;
                aPoint = 0x09;
            }
        }
        double p1 = 3 - inSum + xins;
        if (aScore <= bScore && p1 < bScore) {
            bScore = p1;
            bPoint = 0x0E;
            bIsBiggerSide = false;
        } else if (aScore > bScore && p1 < aScore) {
            aScore = p1;
            aPoint = 0x0E;
            aIsBiggerSide = false;
        }
        double p2 = 3 - inSum + yins;
        if (aScore <= bScore && p2 < bScore) {
            bScore = p2;
            bPoint = 0x0D;
            bIsBiggerSide = false;
        } else if (aScore > bScore && p2 < aScore) {
            aScore = p2;
            aPoint = 0x0D;
            aIsBiggerSide = false;
        }
        double p3 = 3 - inSum + zins;
        if (aScore <= bScore && p3 < bScore) {
            bScore = p3;
            bPoint = 0x0B;
            bIsBiggerSide = false;
        } else if (aScore > bScore && p3 < aScore) {
            aScore = p3;
            aPoint = 0x0B;
            aIsBiggerSide = false;
        }
        double p4 = 3 - inSum + wins;
        if (aScore <= bScore && p4 < bScore) {
            bScore = p4;
            bPoint = 0x07;
            bIsBiggerSide = false;
        } else if (aScore > bScore && p4 < aScore) {
            aScore = p4;
            aPoint = 0x07;
            aIsBiggerSide = false;
        }
        if (aIsBiggerSide == bIsBiggerSide) {
            if (aIsBiggerSide) {
                int8_t c1 = (int8_t)(aPoint & bPoint);
                int8_t c2 = (int8_t)(aPoint | bPoint);
                xsv_ext0 = xsv_ext1 = xsb;
                ysv_ext0 = ysv_ext1 = ysb;
                zsv_ext0 = zsv_ext1 = zsb;
                wsv_ext0 = wsv_ext1 = wsb;
                dx_ext0 = dx0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext0 = dy0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext0 = dz0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext0 = dw0 - Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext1 = dy0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext1 = dz0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c1 & 0x01) != 0) {
                    xsv_ext0 += 1;
                    dx_ext0 -= 1;
                    xsv_ext1 += 2;
                    dx_ext1 -= 2;
                } else if ((c1 & 0x02) != 0) {
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                    ysv_ext1 += 2;
                    dy_ext1 -= 2;
                } else if ((c1 & 0x04) != 0) {
                    zsv_ext0 += 1;
                    dz_ext0 -= 1;
                    zsv_ext1 += 2;
                    dz_ext1 -= 2;
                } else {
                    wsv_ext0 += 1;
                    dw_ext0 -= 1;
                    wsv_ext1 += 2;
                    dw_ext1 -= 2;
                }
                xsv_ext2 = xsb + 1;
                ysv_ext2 = ysb + 1;
                zsv_ext2 = zsb + 1;
                wsv_ext2 = wsb + 1;
                dx_ext2 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext2 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext2 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext2 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c2 & 0x01) == 0) {
                    xsv_ext2 -= 2;
                    dx_ext2 += 2;
                } else if ((c2 & 0x02) == 0) {
                    ysv_ext2 -= 2;
                    dy_ext2 += 2;
                } else if ((c2 & 0x04) == 0) {
                    zsv_ext2 -= 2;
                    dz_ext2 += 2;
                } else {
                    wsv_ext2 -= 2;
                    dw_ext2 += 2;
                }
            } else {
                xsv_ext2 = xsb + 1;
                ysv_ext2 = ysb + 1;
                zsv_ext2 = zsb + 1;
                wsv_ext2 = wsb + 1;
                dx_ext2 = dx0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dy_ext2 = dy0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dz_ext2 = dz0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext2 = dw0 - 1 - 4 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                int8_t c = (int8_t)(aPoint & bPoint); 
                if ((c & 0x01) != 0) {
                    xsv_ext0 = xsb + 2;
                    xsv_ext1 = xsb + 1;
                    dx_ext0 = dx0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dx_ext1 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    xsv_ext0 = xsv_ext1 = xsb;
                    dx_ext0 = dx_ext1 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }
                if ((c & 0x02) != 0) {
                    ysv_ext0 = ysv_ext1 = ysb + 1;
                    dy_ext0 = dy_ext1 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    if ((c & 0x01) == 0){
                        ysv_ext0 += 1;
                        dy_ext0 -= 1;
                    } else {
                        ysv_ext1 += 1;
                        dy_ext1 -= 1;
                    }
                } else {
                    ysv_ext0 = ysv_ext1 = ysb;
                    dy_ext0 = dy_ext1 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } 
                if ((c & 0x04) != 0) {
                    zsv_ext0 = zsv_ext1 = zsb + 1;
                    dz_ext0 = dz_ext1 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    if ((c & 0x03) == 0){
                        zsv_ext0 += 1;
                        dz_ext0 -= 1;
                    } else {
                        zsv_ext1 += 1;
                        dz_ext1 -= 1;
                    }
                } else {
                    zsv_ext0 = zsv_ext1 = zsb;
                    dz_ext0 = dz_ext1 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }  
                if ((c & 0x08) != 0){
                    wsv_ext0 = wsb + 1;
                    wsv_ext1 = wsb + 2;
                    dw_ext0 = dw0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                    dw_ext1 = dw0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                } else {
                    wsv_ext0 = wsv_ext1 = wsb;
                    dw_ext0 = dw_ext1 = dw0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                }
            }
        } else {
            int8_t c1, c2;
            if (aIsBiggerSide) {
                c1 = aPoint;
                c2 = bPoint;
            } else {
                c1 = bPoint;
                c2 = aPoint;
            }   
            if ((c1 & 0x01) != 0) {
                xsv_ext0 = xsb + 2;
                xsv_ext1 = xsb + 1;
                dx_ext0 = dx0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dx_ext1 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                xsv_ext0 = xsv_ext1 = xsb;
                dx_ext0 = dx_ext1 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }  
            if ((c1 & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c1 & 0x01) == 0) {
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                } else {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                }
            } else {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            if ((c1 & 0x04) != 0) {
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz_ext1 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                if ((c1 & 0x03) == 0) {
                    zsv_ext0 += 1;
                    dz_ext0 -= 1;
                } else {
                    zsv_ext1 += 1;
                    dz_ext1 -= 1;
                }
            } else {
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz_ext1 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            if ((c1 & 0x08) != 0) {
                wsv_ext0 = wsb + 1;
                wsv_ext1 = wsb + 2;
                dw_ext0 = dw0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
                dw_ext1 = dw0 - 2 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            } else {
                wsv_ext0 = wsv_ext1 = wsb;
                dw_ext0 = dw_ext1 = dw0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            }
            xsv_ext2 = xsb + 1;
            ysv_ext2 = ysb + 1;
            zsv_ext2 = zsb + 1;
            wsv_ext2 = wsb + 1;
            dx_ext2 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            dy_ext2 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            dz_ext2 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            dw_ext2 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
            if ((c2 & 0x01) == 0) {
                xsv_ext2 -= 2;
                dx_ext2 += 2;
            } else if ((c2 & 0x02) == 0) {
                ysv_ext2 -= 2;
                dy_ext2 += 2;
            } else if ((c2 & 0x04) == 0) {
                zsv_ext2 -= 2;
                dz_ext2 += 2;
            } else {
                wsv_ext2 -= 2;
                dw_ext2 += 2;
            }
        }
        double dx4 = dx0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy4 = dy0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz4 = dz0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw4 = dw0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * extrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 0, dx4, dy4, dz4, dw4);
        }
        double dx3 = dx4;
        double dy3 = dy4;
        double dz3 = dz0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw3 = dw0 - 1 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * extrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 1, dx3, dy3, dz3, dw3);
        }
        double dx2 = dx4;
        double dy2 = dy0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz2 = dz4;
        double dw2 = dw3;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * extrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 1, dx2, dy2, dz2, dw2);
        }
        double dx1 = dx0 - 3 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz1 = dz4;
        double dy1 = dy4;
        double dw1 = dw3;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * extrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 1, dx1, dy1, dz1, dw1);
        }
        double dx5 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy5 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz5 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw5 = dw0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn5 = 2 - dx5 * dx5 - dy5 * dy5 - dz5 * dz5 - dw5 * dw5;
        if (attn5 > 0) {
            attn5 *= attn5;
            value += attn5 * attn5 * extrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 0, dx5, dy5, dz5, dw5);
        }
        double dx6 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy6 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz6 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw6 = dw0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn6 = 2 - dx6 * dx6 - dy6 * dy6 - dz6 * dz6 - dw6 * dw6;
        if (attn6 > 0) {
            attn6 *= attn6;
            value += attn6 * attn6 * extrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 0, dx6, dy6, dz6, dw6);
        }
        double dx7 = dx0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy7 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz7 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw7 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn7 = 2 - dx7 * dx7 - dy7 * dy7 - dz7 * dz7 - dw7 * dw7;
        if (attn7 > 0) {
            attn7 *= attn7;
            value += attn7 * attn7 * extrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 1, dx7, dy7, dz7, dw7);
        }
        double dx8 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy8 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz8 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw8 = dw0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn8 = 2 - dx8 * dx8 - dy8 * dy8 - dz8 * dz8 - dw8 * dw8;
        if (attn8 > 0) {
            attn8 *= attn8;
            value += attn8 * attn8 * extrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 0, dx8, dy8, dz8, dw8);
        }
        double dx9 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy9 = dy0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz9 = dz0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw9 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn9 = 2 - dx9 * dx9 - dy9 * dy9 - dz9 * dz9 - dw9 * dw9;
        if (attn9 > 0) {
            attn9 *= attn9;
            value += attn9 * attn9 * extrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 1, dx9, dy9, dz9, dw9);
        }
        double dx10 = dx0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dy10 = dy0 - 0 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dz10 = dz0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double dw10 = dw0 - 1 - 2 * Detail::MathNoiseManagement::SQUISH_CONSTANT_4D;
        double attn10 = 2 - dx10 * dx10 - dy10 * dy10 - dz10 * dz10 - dw10 * dw10;
        if (attn10 > 0) {
            attn10 *= attn10;
            value += attn10 * attn10 * extrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 1, dx10, dy10, dz10, dw10);
        }
    }
    double attn_ext0 = 2 - dx_ext0 * dx_ext0 - dy_ext0 * dy_ext0 - dz_ext0 * dz_ext0 - dw_ext0 * dw_ext0;
    if (attn_ext0 > 0){
        attn_ext0 *= attn_ext0;
        value += attn_ext0 * attn_ext0 * extrapolate(xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0, dx_ext0, dy_ext0, dz_ext0, dw_ext0);
    }
    double attn_ext1 = 2 - dx_ext1 * dx_ext1 - dy_ext1 * dy_ext1 - dz_ext1 * dz_ext1 - dw_ext1 * dw_ext1;
    if (attn_ext1 > 0){
        attn_ext1 *= attn_ext1;
        value += attn_ext1 * attn_ext1 * extrapolate(xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1, dx_ext1, dy_ext1, dz_ext1, dw_ext1);
    }
    double attn_ext2 = 2 - dx_ext2 * dx_ext2 - dy_ext2 * dy_ext2 - dz_ext2 * dz_ext2 - dw_ext2 * dw_ext2;
    if (attn_ext2 > 0){
        attn_ext2 *= attn_ext2;
        value += attn_ext2 * attn_ext2 * extrapolate(xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2, dx_ext2, dy_ext2, dz_ext2, dw_ext2);
    }
    return value / Detail::MathNoiseManagement::NORM_CONSTANT_4D;
}

double Noise::_2D::noiseOpenSimplex2D(double x, double y, unsigned long long seed){
    Noise::Detail::MathNoiseManagement::_initFromSeed(seed);
    return Noise::_2D::noiseOpenSimplex2D(x,y);
}
double Noise::_3D::noiseOpenSimplex3D(double x, double y, double z, unsigned long long seed){
    Noise::Detail::MathNoiseManagement::_initFromSeed(seed);
    return Noise::_3D::noiseOpenSimplex3D(x,y,z);
}
double Noise::_4D::noiseOpenSimplex4D(double x, double y, double z, double w, unsigned long long seed){
    Noise::Detail::MathNoiseManagement::_initFromSeed(seed);
    return Noise::_4D::noiseOpenSimplex4D(x,y,z,w);
}

#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/math/SimplexNoise.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

SimplexNoise noise;
SimplexNoise::SimplexNoise(){
    m_Constants[0] = -0.211324865405187; //STRETCH_CONSTANT_2D
    m_Constants[1] = 0.366025403784439;  //SQUISH_CONSTANT_2D
    m_Constants[2] = 0.166666666666666;  //STRETCH_CONSTANT_3D
    m_Constants[3] = 0.333333333333333;  //SQUISH_CONSTANT_3D
    m_Constants[4] = -0.138196601125011; //STRETCH_CONSTANT_4D
    m_Constants[5] = 0.309016994374947;  //SQUISH_CONSTANT_4D
    m_Constants[6] = 47.0;  //NORM_CONSTANT_2D
    m_Constants[7] = 103.0; //NORM_CONSTANT_3D
    m_Constants[8] = 30.0;  //NORM_CONSTANT_4D

    m_Grad2[0] = glm::ivec2(5, 2);
    m_Grad2[1] = glm::ivec2(2, 5);
    m_Grad2[2] = glm::ivec2(-5, 2);
    m_Grad2[3] = glm::ivec2(-2, 5);
    m_Grad2[4] = glm::ivec2(5, -2);
    m_Grad2[5] = glm::ivec2(2, -5);
    m_Grad2[6] = glm::ivec2(-5, -2);
    m_Grad2[7] = glm::ivec2(-2, -5);

    m_Grad3[0] = glm::ivec3(-11, 4, 4);
    m_Grad3[1] = glm::ivec3(-4, 11, 4);
    m_Grad3[2] = glm::ivec3(-4, 4, 11);
    m_Grad3[3] = glm::ivec3(11, 4, 4);
    m_Grad3[4] = glm::ivec3(4, 11, 4);
    m_Grad3[5] = glm::ivec3(4, 4, 11);
    m_Grad3[6] = glm::ivec3(-11, -4, 4);
    m_Grad3[7] = glm::ivec3(-4, -11, 4);
    m_Grad3[8] = glm::ivec3(-4, -4, 11);
    m_Grad3[9] = glm::ivec3(11, -4, 4);
    m_Grad3[10] = glm::ivec3(4, -11, 4);
    m_Grad3[11] = glm::ivec3(4, -4, 11);
    m_Grad3[12] = glm::ivec3(-11, 4, -4);
    m_Grad3[13] = glm::ivec3(-4, 11, -4);
    m_Grad3[14] = glm::ivec3(-4, 4, -11);
    m_Grad3[15] = glm::ivec3(11, 4, -4);
    m_Grad3[16] = glm::ivec3(4, 11, -4);
    m_Grad3[17] = glm::ivec3(4, 4, -11);
    m_Grad3[18] = glm::ivec3(-11, -4, -4);
    m_Grad3[19] = glm::ivec3(-4, -11, -4);
    m_Grad3[20] = glm::ivec3(-4, -4, -11);
    m_Grad3[21] = glm::ivec3(11, -4, -4);
    m_Grad3[22] = glm::ivec3(4, -11, -4);
    m_Grad3[23] = glm::ivec3(4, -4, -11);

    m_Grad4[0] = glm::ivec4(3, 1, 1, 1);
    m_Grad4[1] = glm::ivec4(1, 3, 1, 1);
    m_Grad4[2] = glm::ivec4(1, 1, 3, 1);
    m_Grad4[3] = glm::ivec4(1, 1, 1, 3);
    m_Grad4[4] = glm::ivec4(-3, 1, 1, 1);
    m_Grad4[5] = glm::ivec4(-1, 3, 1, 1);
    m_Grad4[6] = glm::ivec4(-1, 1, 3, 1);
    m_Grad4[7] = glm::ivec4(-1, 1, 1, 3);
    m_Grad4[8] = glm::ivec4(3, -1, 1, 1);
    m_Grad4[9] = glm::ivec4(1, -3, 1, 1);
    m_Grad4[10] = glm::ivec4(1, -1, 3, 1);
    m_Grad4[11] = glm::ivec4(1, -1, 1, 3);
    m_Grad4[12] = glm::ivec4(-3, -1, 1, 1);
    m_Grad4[13] = glm::ivec4(-1, -3, 1, 1);
    m_Grad4[14] = glm::ivec4(-1, -1, 3, 1);
    m_Grad4[15] = glm::ivec4(-1, -1, 1, 3);
    m_Grad4[16] = glm::ivec4(3, 1, -1, 1);
    m_Grad4[17] = glm::ivec4(1, 3, -1, 1);
    m_Grad4[18] = glm::ivec4(1, 1, -3, 1);
    m_Grad4[19] = glm::ivec4(1, 1, -1, 3);
    m_Grad4[20] = glm::ivec4(-3, 1, -1, 1);
    m_Grad4[21] = glm::ivec4(-1, 3, -1, 1);
    m_Grad4[22] = glm::ivec4(-1, 1, -3, 1);
    m_Grad4[23] = glm::ivec4(-1, 1, -1, 3);
    m_Grad4[24] = glm::ivec4(3, -1, -1, 1);
    m_Grad4[25] = glm::ivec4(1, -3, -1, 1);
    m_Grad4[26] = glm::ivec4(1, -1, -3, 1);
    m_Grad4[27] = glm::ivec4(1, -1, -1, 3);
    m_Grad4[28] = glm::ivec4(-3, -1, -1, 1);
    m_Grad4[29] = glm::ivec4(-1, -3, -1, 1);
    m_Grad4[30] = glm::ivec4(-1, -1, -3, 1);
    m_Grad4[31] = glm::ivec4(-1, -1, -1, 3);
    m_Grad4[32] = glm::ivec4(3, 1, 1, -1);
    m_Grad4[33] = glm::ivec4(1, 3, 1, -1);
    m_Grad4[34] = glm::ivec4(1, 1, 3, -1);
    m_Grad4[35] = glm::ivec4(1, 1, 1, -3);
    m_Grad4[36] = glm::ivec4(-3, 1, 1, -1);
    m_Grad4[37] = glm::ivec4(-1, 3, 1, -1);
    m_Grad4[38] = glm::ivec4(-1, 1, 3, -1);
    m_Grad4[39] = glm::ivec4(-1, 1, 1, -3);
    m_Grad4[40] = glm::ivec4(3, -1, 1, -1);
    m_Grad4[41] = glm::ivec4(1, -3, 1, -1);
    m_Grad4[42] = glm::ivec4(1, -1, 3, -1);
    m_Grad4[43] = glm::ivec4(1, -1, 1, -3);
    m_Grad4[44] = glm::ivec4(-3, -1, 1, -1);
    m_Grad4[45] = glm::ivec4(-1, -3, 1, -1);
    m_Grad4[46] = glm::ivec4(-1, -1, 3, -1);
    m_Grad4[47] = glm::ivec4(-1, -1, 1, -3);
    m_Grad4[48] = glm::ivec4(3, 1, -1, -1);
    m_Grad4[49] = glm::ivec4(1, 3, -1, -1);
    m_Grad4[50] = glm::ivec4(1, 1, -3, -1);
    m_Grad4[51] = glm::ivec4(1, 1, -1, -3);
    m_Grad4[52] = glm::ivec4(-3, 1, -1, -1);
    m_Grad4[53] = glm::ivec4(-1, 3, -1, -1);
    m_Grad4[54] = glm::ivec4(-1, 1, -3, -1);
    m_Grad4[55] = glm::ivec4(-1, 1, -1, -3);
    m_Grad4[56] = glm::ivec4(3, -1, -1, -1);
    m_Grad4[57] = glm::ivec4(1, -3, -1, -1);
    m_Grad4[58] = glm::ivec4(1, -1, -3, -1);
    m_Grad4[59] = glm::ivec4(1, -1, -1, -3);
    m_Grad4[60] = glm::ivec4(-3, -1, -1, -1);
    m_Grad4[61] = glm::ivec4(-1, -3, -1, -1);
    m_Grad4[62] = glm::ivec4(-1, -1, -3, -1);
    m_Grad4[63] = glm::ivec4(-1, -1, -1, -3);

    unsigned long long zero(time(0));
    internalInitFromSeed(zero);
}
SimplexNoise::~SimplexNoise(){
    m_Perm.clear();
    m_PermGradIndex3D.clear();
}
void SimplexNoise::internalInitFromSeed(const unsigned long long& seed) {
    unsigned long long _seed = seed;
    vector_clear(m_Perm);
    vector_clear(m_PermGradIndex3D);
    m_Perm.resize(256);
    m_PermGradIndex3D.resize(256);
    vector<short> source; source.resize(256);
    for (short i = 0; i < 256; ++i)
        source[i] = i;
    _seed = _seed * (6364136223846793005LL) + (1442695040888963407LL);
    _seed = _seed * (6364136223846793005LL) + (1442695040888963407LL);
    _seed = _seed * (6364136223846793005LL) + (1442695040888963407LL);
    for (int i = 255; i >= 0; i--) {
        _seed = _seed * (6364136223846793005LL) + (1442695040888963407LL);
        int r = (int)((_seed + 31) % (i + 1));
        if (r < 0)
            r += (i + 1);
        m_Perm[i] = source[r];
        m_PermGradIndex3D[i] = (short)((m_Perm[i] % 8) * 3);
        source[r] = source[i];
    }
}
double SimplexNoise::internalExtrapolate(const int xsb, const int ysb, const double& dx, const double& dy) {
    const int index = m_Perm[(m_Perm[xsb & 0xFF] + ysb) & 0xFF] & 0x0E;
    return m_Grad2[index].x * dx + m_Grad2[index].y * dy;
}
double SimplexNoise::internalExtrapolate(const int xsb, const int ysb, const int zsb, const double& dx, const double& dy, const double& dz) {
    const int index = m_PermGradIndex3D[(m_Perm[(m_Perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF];
    return m_Grad3[index].x * dx + m_Grad3[index].y * dy + m_Grad3[index].z * dz;
}
double SimplexNoise::internalExtrapolate(const int xsb, const int ysb, const int zsb, const int wsb, const double& dx, const double& dy, const double& dz, const double& dw) {
    const int index = m_Perm[(m_Perm[(m_Perm[(m_Perm[xsb & 0xFF] + ysb) & 0xFF] + zsb) & 0xFF] + wsb) & 0xFF] & 0xFC;
    return m_Grad4[index].x * dx + m_Grad4[index].y * dy + m_Grad4[index].z * dz + m_Grad4[index].w * dw;
}
double SimplexNoise::noiseOpenSimplex2D(const double& x, const double& y) {
    double stretchOffset = (x + y) * m_Constants[0];
    double xs = x + stretchOffset;
    double ys = y + stretchOffset;
    int xsb = int(glm::floor(xs));
    int ysb = int(glm::floor(ys));
    double squishOffset = (xsb + ysb) * m_Constants[1];
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
    double dx1 = dx0 - 1 - m_Constants[1];
    double dy1 = dy0 - 0 - m_Constants[1];
    double attn1 = 2 - dx1 * dx1 - dy1 * dy1;
    if (attn1 > 0) {
        attn1 *= attn1;
        value += attn1 * attn1 * internalExtrapolate(xsb + 1, ysb + 0, dx1, dy1);
    }
    double dx2 = dx0 - 0 - m_Constants[1];
    double dy2 = dy0 - 1 - m_Constants[1];
    double attn2 = 2 - dx2 * dx2 - dy2 * dy2;
    if (attn2 > 0) {
        attn2 *= attn2;
        value += attn2 * attn2 * internalExtrapolate(xsb + 0, ysb + 1, dx2, dy2);
    }
    if (inSum <= 1) {
        double zins = 1 - inSum;
        if (zins > xins || zins > yins) {
            if (xins > yins) {
                xsv_ext = xsb + 1;
                ysv_ext = ysb - 1;
                dx_ext = dx0 - 1;
                dy_ext = dy0 + 1;
            }else{
                xsv_ext = xsb - 1;
                ysv_ext = ysb + 1;
                dx_ext = dx0 + 1;
                dy_ext = dy0 - 1;
            }
        }else{
            xsv_ext = xsb + 1;
            ysv_ext = ysb + 1;
            dx_ext = dx0 - 1 - 2 * m_Constants[1];
            dy_ext = dy0 - 1 - 2 * m_Constants[1];
        }
    }else{
        double zins = 2 - inSum;
        if (zins < xins || zins < yins) {
            if (xins > yins) {
                xsv_ext = xsb + 2;
                ysv_ext = ysb + 0;
                dx_ext = dx0 - 2 - 2 * m_Constants[1];
                dy_ext = dy0 + 0 - 2 * m_Constants[1];
            }else{
                xsv_ext = xsb + 0;
                ysv_ext = ysb + 2;
                dx_ext = dx0 + 0 - 2 * m_Constants[1];
                dy_ext = dy0 - 2 - 2 * m_Constants[1];
            }
        }else{
            dx_ext = dx0;
            dy_ext = dy0;
            xsv_ext = xsb;
            ysv_ext = ysb;
        }
        xsb += 1;
        ysb += 1;
        dx0 = dx0 - 1 - 2 * m_Constants[1];
        dy0 = dy0 - 1 - 2 * m_Constants[1];
    }
    double attn0 = 2 - dx0 * dx0 - dy0 * dy0;
    if (attn0 > 0) {
        attn0 *= attn0;
        value += attn0 * attn0 * internalExtrapolate(xsb, ysb, dx0, dy0);
    }
    double attn_ext = 2 - dx_ext * dx_ext - dy_ext * dy_ext;
    if (attn_ext > 0) {
        attn_ext *= attn_ext;
        value += attn_ext * attn_ext * internalExtrapolate(xsv_ext, ysv_ext, dx_ext, dy_ext);
    }
    return value / m_Constants[6];
}
double SimplexNoise::noiseOpenSimplex3D(const double& x, const double& y, const double& z) {
    double stretchOffset = (x + y + z) * m_Constants[2];
    double xs = x + stretchOffset;
    double ys = y + stretchOffset;
    double zs = z + stretchOffset;
    int xsb = int(glm::floor(xs));
    int ysb = int(glm::floor(ys));
    int zsb = int(glm::floor(zs));
    double squishOffset = (xsb + ysb + zsb) * m_Constants[3];
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
        }else if (aScore < bScore && zins > aScore) {
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
            }else{
                xsv_ext0 = xsv_ext1 = xsb + 1;
                dx_ext0 = dx_ext1 = dx0 - 1;
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0;
                if ((c & 0x01) == 0) {
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                }else{
                    ysv_ext0 -= 1;
                    dy_ext0 += 1;
                }
            }else{
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1;
            }
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsb;
                zsv_ext1 = zsb - 1;
                dz_ext0 = dz0;
                dz_ext1 = dz0 + 1;
            }else{
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz_ext1 = dz0 - 1;
            }
        }else{
            int8_t c = (int8_t)(aPoint | bPoint);
            if ((c & 0x01) == 0) {
                xsv_ext0 = xsb;
                xsv_ext1 = xsb - 1;
                dx_ext0 = dx0 - 2 * m_Constants[3];
                dx_ext1 = dx0 + 1 - m_Constants[3];
            }else{
                xsv_ext0 = xsv_ext1 = xsb + 1;
                dx_ext0 = dx0 - 1 - 2 * m_Constants[3];
                dx_ext1 = dx0 - 1 - m_Constants[3];
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysb;
                ysv_ext1 = ysb - 1;
                dy_ext0 = dy0 - 2 * m_Constants[3];
                dy_ext1 = dy0 + 1 - m_Constants[3];
            }else{
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy0 - 1 - 2 * m_Constants[3];
                dy_ext1 = dy0 - 1 - m_Constants[3];
            }
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsb;
                zsv_ext1 = zsb - 1;
                dz_ext0 = dz0 - 2 * m_Constants[3];
                dz_ext1 = dz0 + 1 - m_Constants[3];
            }else{
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz0 - 1 - 2 * m_Constants[3];
                dz_ext1 = dz0 - 1 - m_Constants[3];
            }
        }
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 0, dx0, dy0, dz0);
        }
        double dx1 = dx0 - 1 - m_Constants[3];
        double dy1 = dy0 - 0 - m_Constants[3];
        double dz1 = dz0 - 0 - m_Constants[3];
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 0, dx1, dy1, dz1);
        }
        double dx2 = dx0 - 0 - m_Constants[3];
        double dy2 = dy0 - 1 - m_Constants[3];
        double dz2 = dz1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 0, dx2, dy2, dz2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - m_Constants[3];
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 1, dx3, dy3, dz3);
        }
    }else if (inSum >= 2){
        int8_t aPoint = 0x06;
        double aScore = xins;
        int8_t bPoint = 0x05;
        double bScore = yins;
        if (aScore <= bScore && zins < bScore) {
            bScore = zins;
            bPoint = 0x03;
        }else if (aScore > bScore && zins < aScore) {
            aScore = zins;
            aPoint = 0x03;
        }
        double wins = 3 - inSum;
        if (wins < aScore || wins < bScore) {
            int8_t c = (bScore < aScore ? bPoint : aPoint);
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsb + 2;
                xsv_ext1 = xsb + 1;
                dx_ext0 = dx0 - 2 - 3 * m_Constants[3];
                dx_ext1 = dx0 - 1 - 3 * m_Constants[3];
            }else{
                xsv_ext0 = xsv_ext1 = xsb;
                dx_ext0 = dx_ext1 = dx0 - 3 * m_Constants[3];
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1 - 3 * m_Constants[3];
                if ((c & 0x01) != 0) {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                }else{
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                }
            }else{
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0 - 3 * m_Constants[3];
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsb + 1;
                zsv_ext1 = zsb + 2;
                dz_ext0 = dz0 - 1 - 3 * m_Constants[3];
                dz_ext1 = dz0 - 2 - 3 * m_Constants[3];
            }else{
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz_ext1 = dz0 - 3 * m_Constants[3];
            }
        }else{
            int8_t c = (int8_t)(aPoint & bPoint);
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsb + 1;
                xsv_ext1 = xsb + 2;
                dx_ext0 = dx0 - 1 - m_Constants[3];
                dx_ext1 = dx0 - 2 - 2 * m_Constants[3];
            }else{
                xsv_ext0 = xsv_ext1 = xsb;
                dx_ext0 = dx0 - m_Constants[3];
                dx_ext1 = dx0 - 2 * m_Constants[3];
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysb + 1;
                ysv_ext1 = ysb + 2;
                dy_ext0 = dy0 - 1 - m_Constants[3];
                dy_ext1 = dy0 - 2 - 2 * m_Constants[3];
            }else{
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy0 - m_Constants[3];
                dy_ext1 = dy0 - 2 * m_Constants[3];
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsb + 1;
                zsv_ext1 = zsb + 2;
                dz_ext0 = dz0 - 1 - m_Constants[3];
                dz_ext1 = dz0 - 2 - 2 * m_Constants[3];
            }else{
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz0 - m_Constants[3];
                dz_ext1 = dz0 - 2 * m_Constants[3];
            }
        }
        double dx3 = dx0 - 1 - 2 * m_Constants[3];
        double dy3 = dy0 - 1 - 2 * m_Constants[3];
        double dz3 = dz0 - 0 - 2 * m_Constants[3];
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 0, dx3, dy3, dz3);
        }
        double dx2 = dx3;
        double dy2 = dy0 - 0 - 2 * m_Constants[3];
        double dz2 = dz0 - 1 - 2 * m_Constants[3];
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 1, dx2, dy2, dz2);
        }
        double dx1 = dx0 - 0 - 2 * m_Constants[3];
        double dy1 = dy3;
        double dz1 = dz2;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 1, dx1, dy1, dz1);
        }
        dx0 = dx0 - 1 - 3 * m_Constants[3];
        dy0 = dy0 - 1 - 3 * m_Constants[3];
        dz0 = dz0 - 1 - 3 * m_Constants[3];
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 1, dx0, dy0, dz0);
        }
    }else{
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
        }else{
            aScore = 1 - p1;
            aPoint = 0x04;
            aIsFurtherSide = false;
        }
        double p2 = xins + zins;
        if (p2 > 1) {
            bScore = p2 - 1;
            bPoint = 0x05;
            bIsFurtherSide = true;
        }else{
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
            }else if (aScore > bScore && bScore < score) {
                bScore = score;
                bPoint = 0x06;
                bIsFurtherSide = true;
            }
        }else{
            double score = 1 - p3;
            if (aScore <= bScore && aScore < score) {
                aScore = score;
                aPoint = 0x01;
                aIsFurtherSide = false;
            }else if (aScore > bScore && bScore < score) {
                bScore = score;
                bPoint = 0x01;
                bIsFurtherSide = false;
            }
        }
        if (aIsFurtherSide == bIsFurtherSide) {
            if (aIsFurtherSide) {
                dx_ext0 = dx0 - 1 - 3 * m_Constants[3];
                dy_ext0 = dy0 - 1 - 3 * m_Constants[3];
                dz_ext0 = dz0 - 1 - 3 * m_Constants[3];
                xsv_ext0 = xsb + 1;
                ysv_ext0 = ysb + 1;
                zsv_ext0 = zsb + 1;
                int8_t c = (int8_t)(aPoint & bPoint);
                if ((c & 0x01) != 0) {
                    dx_ext1 = dx0 - 2 - 2 * m_Constants[3];
                    dy_ext1 = dy0 - 2 * m_Constants[3];
                    dz_ext1 = dz0 - 2 * m_Constants[3];
                    xsv_ext1 = xsb + 2;
                    ysv_ext1 = ysb;
                    zsv_ext1 = zsb;
                }else if ((c & 0x02) != 0) {
                    dx_ext1 = dx0 - 2 * m_Constants[3];
                    dy_ext1 = dy0 - 2 - 2 * m_Constants[3];
                    dz_ext1 = dz0 - 2 * m_Constants[3];
                    xsv_ext1 = xsb;
                    ysv_ext1 = ysb + 2;
                    zsv_ext1 = zsb;
                }else{
                    dx_ext1 = dx0 - 2 * m_Constants[3];
                    dy_ext1 = dy0 - 2 * m_Constants[3];
                    dz_ext1 = dz0 - 2 - 2 * m_Constants[3];
                    xsv_ext1 = xsb;
                    ysv_ext1 = ysb;
                    zsv_ext1 = zsb + 2;
                }
            }else{
                dx_ext0 = dx0;
                dy_ext0 = dy0;
                dz_ext0 = dz0;
                xsv_ext0 = xsb;
                ysv_ext0 = ysb;
                zsv_ext0 = zsb;
                int8_t c = (int8_t)(aPoint | bPoint);
                if ((c & 0x01) == 0) {
                    dx_ext1 = dx0 + 1 - m_Constants[3];
                    dy_ext1 = dy0 - 1 - m_Constants[3];
                    dz_ext1 = dz0 - 1 - m_Constants[3];
                    xsv_ext1 = xsb - 1;
                    ysv_ext1 = ysb + 1;
                    zsv_ext1 = zsb + 1;
                }else if ((c & 0x02) == 0) {
                    dx_ext1 = dx0 - 1 - m_Constants[3];
                    dy_ext1 = dy0 + 1 - m_Constants[3];
                    dz_ext1 = dz0 - 1 - m_Constants[3];
                    xsv_ext1 = xsb + 1;
                    ysv_ext1 = ysb - 1;
                    zsv_ext1 = zsb + 1;
                }else{
                    dx_ext1 = dx0 - 1 - m_Constants[3];
                    dy_ext1 = dy0 - 1 - m_Constants[3];
                    dz_ext1 = dz0 + 1 - m_Constants[3];
                    xsv_ext1 = xsb + 1;
                    ysv_ext1 = ysb + 1;
                    zsv_ext1 = zsb - 1;
                }
            }
        }else{
            int8_t c1, c2;
            if (aIsFurtherSide) {
                c1 = aPoint;
                c2 = bPoint;
            }else{
                c1 = bPoint;
                c2 = aPoint;
            }
            if ((c1 & 0x01) == 0) {
                dx_ext0 = dx0 + 1 - m_Constants[3];
                dy_ext0 = dy0 - 1 - m_Constants[3];
                dz_ext0 = dz0 - 1 - m_Constants[3];
                xsv_ext0 = xsb - 1;
                ysv_ext0 = ysb + 1;
                zsv_ext0 = zsb + 1;
            }else if ((c1 & 0x02) == 0) {
                dx_ext0 = dx0 - 1 - m_Constants[3];
                dy_ext0 = dy0 + 1 - m_Constants[3];
                dz_ext0 = dz0 - 1 - m_Constants[3];
                xsv_ext0 = xsb + 1;
                ysv_ext0 = ysb - 1;
                zsv_ext0 = zsb + 1;
            }else{
                dx_ext0 = dx0 - 1 - m_Constants[3];
                dy_ext0 = dy0 - 1 - m_Constants[3];
                dz_ext0 = dz0 + 1 - m_Constants[3];
                xsv_ext0 = xsb + 1;
                ysv_ext0 = ysb + 1;
                zsv_ext0 = zsb - 1;
            }
            dx_ext1 = dx0 - 2 * m_Constants[3];
            dy_ext1 = dy0 - 2 * m_Constants[3];
            dz_ext1 = dz0 - 2 * m_Constants[3];
            xsv_ext1 = xsb;
            ysv_ext1 = ysb;
            zsv_ext1 = zsb;
            if ((c2 & 0x01) != 0) {
                dx_ext1 -= 2;
                xsv_ext1 += 2;
            }else if ((c2 & 0x02) != 0) {
                dy_ext1 -= 2;
                ysv_ext1 += 2;
            }else{
                dz_ext1 -= 2;
                zsv_ext1 += 2;
            }
        }
        double dx1 = dx0 - 1 - m_Constants[3];
        double dy1 = dy0 - 0 - m_Constants[3];
        double dz1 = dz0 - 0 - m_Constants[3];
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 0, dx1, dy1, dz1);
        }
        double dx2 = dx0 - 0 - m_Constants[3];
        double dy2 = dy0 - 1 - m_Constants[3];
        double dz2 = dz1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 0, dx2, dy2, dz2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - m_Constants[3];
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 1, dx3, dy3, dz3);
        }
        double dx4 = dx0 - 1 - 2 * m_Constants[3];
        double dy4 = dy0 - 1 - 2 * m_Constants[3];
        double dz4 = dz0 - 0 - 2 * m_Constants[3];
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 0, dx4, dy4, dz4);
        }
        double dx5 = dx4;
        double dy5 = dy0 - 0 - 2 * m_Constants[3];
        double dz5 = dz0 - 1 - 2 * m_Constants[3];
        double attn5 = 2 - dx5 * dx5 - dy5 * dy5 - dz5 * dz5;
        if (attn5 > 0) {
            attn5 *= attn5;
            value += attn5 * attn5 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 1, dx5, dy5, dz5);
        }
        double dx6 = dx0 - 0 - 2 * m_Constants[3];
        double dy6 = dy4;
        double dz6 = dz5;
        double attn6 = 2 - dx6 * dx6 - dy6 * dy6 - dz6 * dz6;
        if (attn6 > 0) {
            attn6 *= attn6;
            value += attn6 * attn6 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 1, dx6, dy6, dz6);
        }
    }
    double attn_ext0 = 2 - dx_ext0 * dx_ext0 - dy_ext0 * dy_ext0 - dz_ext0 * dz_ext0;
    if (attn_ext0 > 0) {
        attn_ext0 *= attn_ext0;
        value += attn_ext0 * attn_ext0 * internalExtrapolate(xsv_ext0, ysv_ext0, zsv_ext0, dx_ext0, dy_ext0, dz_ext0);
    }
    double attn_ext1 = 2 - dx_ext1 * dx_ext1 - dy_ext1 * dy_ext1 - dz_ext1 * dz_ext1;
    if (attn_ext1 > 0) {
        attn_ext1 *= attn_ext1;
        value += attn_ext1 * attn_ext1 * internalExtrapolate(xsv_ext1, ysv_ext1, zsv_ext1, dx_ext1, dy_ext1, dz_ext1);
    }
    return value / m_Constants[7];
}
double SimplexNoise::noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w) {
    double stretchOffset = (x + y + z + w) * m_Constants[4];
    double xs = x + stretchOffset;
    double ys = y + stretchOffset;
    double zs = z + stretchOffset;
    double ws = w + stretchOffset;
    int xsb = int(glm::floor(xs));
    int ysb = int(glm::floor(ys));
    int zsb = int(glm::floor(zs));
    int wsb = int(glm::floor(ws));
    double squishOffset = (xsb + ysb + zsb + wsb) * m_Constants[5];
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
        }else if (aScore < bScore && zins > aScore) {
            aScore = zins;
            aPoint = 0x04;
        }
        if (aScore >= bScore && wins > bScore) {
            bScore = wins;
            bPoint = 0x08;
        }else if (aScore < bScore && wins > aScore) {
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
            }else{
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb + 1;
                dx_ext0 = dx_ext1 = dx_ext2 = dx0 - 1;
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0;
                if ((c & 0x01) == 0x01) {
                    ysv_ext0 -= 1;
                    dy_ext0 += 1;
                }else{
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                }
            }else{
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
                    }else{
                        zsv_ext1 -= 1;
                        dz_ext1 += 1;
                    }
                }else{
                    zsv_ext2 -= 1;
                    dz_ext2 += 1;
                }
            }else{
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0 - 1;
            }
            if ((c & 0x08) == 0) {
                wsv_ext0 = wsv_ext1 = wsb;
                wsv_ext2 = wsb - 1;
                dw_ext0 = dw_ext1 = dw0;
                dw_ext2 = dw0 + 1;
            }else{
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb + 1;
                dw_ext0 = dw_ext1 = dw_ext2 = dw0 - 1;
            }
        }else{
            int8_t c = (int8_t)(aPoint | bPoint);
            if ((c & 0x01) == 0) {
                xsv_ext0 = xsv_ext2 = xsb;
                xsv_ext1 = xsb - 1;
                dx_ext0 = dx0 - 2 * m_Constants[5];
                dx_ext1 = dx0 + 1 - m_Constants[5];
                dx_ext2 = dx0 - m_Constants[5];
            }else{
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb + 1;
                dx_ext0 = dx0 - 1 - 2 * m_Constants[5];
                dx_ext1 = dx_ext2 = dx0 - 1 - m_Constants[5];
            }
            if ((c & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy0 - 2 * m_Constants[5];
                dy_ext1 = dy_ext2 = dy0 - m_Constants[5];
                if ((c & 0x01) == 0x01) {
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                }else{
                    ysv_ext2 -= 1;
                    dy_ext2 += 1;
                }
            }else{
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy0 - 1 - 2 * m_Constants[5];
                dy_ext1 = dy_ext2 = dy0 - 1 - m_Constants[5];
            }
            if ((c & 0x04) == 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz0 - 2 * m_Constants[5];
                dz_ext1 = dz_ext2 = dz0 - m_Constants[5];
                if ((c & 0x03) == 0x03) {
                    zsv_ext1 -= 1;
                    dz_ext1 += 1;
                }else{
                    zsv_ext2 -= 1;
                    dz_ext2 += 1;
                }
            }else{
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz0 - 1 - 2 * m_Constants[5];
                dz_ext1 = dz_ext2 = dz0 - 1 - m_Constants[5];
            }
            if ((c & 0x08) == 0) {
                wsv_ext0 = wsv_ext1 = wsb;
                wsv_ext2 = wsb - 1;
                dw_ext0 = dw0 - 2 * m_Constants[5];
                dw_ext1 = dw0 - m_Constants[5];
                dw_ext2 = dw0 + 1 - m_Constants[5];
            }else{
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb + 1;
                dw_ext0 = dw0 - 1 - 2 * m_Constants[5];
                dw_ext1 = dw_ext2 = dw0 - 1 - m_Constants[5];
            }
        }
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0 - dw0 * dw0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 0, wsb + 0, dx0, dy0, dz0, dw0);
        }
        double dx1 = dx0 - 1 - m_Constants[5];
        double dy1 = dy0 - 0 - m_Constants[5];
        double dz1 = dz0 - 0 - m_Constants[5];
        double dw1 = dw0 - 0 - m_Constants[5];
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 0, dx1, dy1, dz1, dw1);
        }
        double dx2 = dx0 - 0 - m_Constants[5];
        double dy2 = dy0 - 1 - m_Constants[5];
        double dz2 = dz1;
        double dw2 = dw1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 0, dx2, dy2, dz2, dw2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - m_Constants[5];
        double dw3 = dw1;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 0, dx3, dy3, dz3, dw3);
        }
        double dx4 = dx2;
        double dy4 = dy1;
        double dz4 = dz1;
        double dw4 = dw0 - 1 - m_Constants[5];
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 0, wsb + 1, dx4, dy4, dz4, dw4);
        }
    }else if (inSum >= 3) {
        int8_t aPoint = 0x0E;
        double aScore = xins;
        int8_t bPoint = 0x0D;
        double bScore = yins;
        if (aScore <= bScore && zins < bScore) {
            bScore = zins;
            bPoint = 0x0B;
        }else if (aScore > bScore && zins < aScore) {
            aScore = zins;
            aPoint = 0x0B;
        }
        if (aScore <= bScore && wins < bScore) {
            bScore = wins;
            bPoint = 0x07;
        }else if (aScore > bScore && wins < aScore) {
            aScore = wins;
            aPoint = 0x07;
        }
        double uins = 4 - inSum;
        if (uins < aScore || uins < bScore) {
            int8_t c = (bScore < aScore ? bPoint : aPoint);
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsb + 2;
                xsv_ext1 = xsv_ext2 = xsb + 1;
                dx_ext0 = dx0 - 2 - 4 * m_Constants[5];
                dx_ext1 = dx_ext2 = dx0 - 1 - 4 * m_Constants[5];
            }else{
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb;
                dx_ext0 = dx_ext1 = dx_ext2 = dx0 - 4 * m_Constants[5];
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0 - 1 - 4 * m_Constants[5];
                if ((c & 0x01) != 0) {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                }else{
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                }
            }else{
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy_ext1 = dy_ext2 = dy0 - 4 * m_Constants[5];
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0 - 1 - 4 * m_Constants[5];
                if ((c & 0x03) != 0x03) {
                    if ((c & 0x03) == 0) {
                        zsv_ext0 += 1;
                        dz_ext0 -= 1;
                    }else{
                        zsv_ext1 += 1;
                        dz_ext1 -= 1;
                    }
                }else{
                    zsv_ext2 += 1;
                    dz_ext2 -= 1;
                }
            }else{
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz_ext1 = dz_ext2 = dz0 - 4 * m_Constants[5];
            }
            if ((c & 0x08) != 0) {
                wsv_ext0 = wsv_ext1 = wsb + 1;
                wsv_ext2 = wsb + 2;
                dw_ext0 = dw_ext1 = dw0 - 1 - 4 * m_Constants[5];
                dw_ext2 = dw0 - 2 - 4 * m_Constants[5];
            }else{
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb;
                dw_ext0 = dw_ext1 = dw_ext2 = dw0 - 4 * m_Constants[5];
            }
        }else{
            int8_t c = (int8_t)(aPoint & bPoint);
            if ((c & 0x01) != 0) {
                xsv_ext0 = xsv_ext2 = xsb + 1;
                xsv_ext1 = xsb + 2;
                dx_ext0 = dx0 - 1 - 2 * m_Constants[5];
                dx_ext1 = dx0 - 2 - 3 * m_Constants[5];
                dx_ext2 = dx0 - 1 - 3 * m_Constants[5];
            }else{
                xsv_ext0 = xsv_ext1 = xsv_ext2 = xsb;
                dx_ext0 = dx0 - 2 * m_Constants[5];
                dx_ext1 = dx_ext2 = dx0 - 3 * m_Constants[5];
            }
            if ((c & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb + 1;
                dy_ext0 = dy0 - 1 - 2 * m_Constants[5];
                dy_ext1 = dy_ext2 = dy0 - 1 - 3 * m_Constants[5];
                if ((c & 0x01) != 0) {
                    ysv_ext2 += 1;
                    dy_ext2 -= 1;
                }else{
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                }
            }else{
                ysv_ext0 = ysv_ext1 = ysv_ext2 = ysb;
                dy_ext0 = dy0 - 2 * m_Constants[5];
                dy_ext1 = dy_ext2 = dy0 - 3 * m_Constants[5];
            }
            if ((c & 0x04) != 0) {
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb + 1;
                dz_ext0 = dz0 - 1 - 2 * m_Constants[5];
                dz_ext1 = dz_ext2 = dz0 - 1 - 3 * m_Constants[5];
                if ((c & 0x03) != 0) {
                    zsv_ext2 += 1;
                    dz_ext2 -= 1;
                }else{
                    zsv_ext1 += 1;
                    dz_ext1 -= 1;
                }
            }else{
                zsv_ext0 = zsv_ext1 = zsv_ext2 = zsb;
                dz_ext0 = dz0 - 2 * m_Constants[5];
                dz_ext1 = dz_ext2 = dz0 - 3 * m_Constants[5];
            }
            if ((c & 0x08) != 0) {
                wsv_ext0 = wsv_ext1 = wsb + 1;
                wsv_ext2 = wsb + 2;
                dw_ext0 = dw0 - 1 - 2 * m_Constants[5];
                dw_ext1 = dw0 - 1 - 3 * m_Constants[5];
                dw_ext2 = dw0 - 2 - 3 * m_Constants[5];
            }else{
                wsv_ext0 = wsv_ext1 = wsv_ext2 = wsb;
                dw_ext0 = dw0 - 2 * m_Constants[5];
                dw_ext1 = dw_ext2 = dw0 - 3 * m_Constants[5];
            }
        }
        double dx4 = dx0 - 1 - 3 * m_Constants[5];
        double dy4 = dy0 - 1 - 3 * m_Constants[5];
        double dz4 = dz0 - 1 - 3 * m_Constants[5];
        double dw4 = dw0 - 3 * m_Constants[5];
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 0, dx4, dy4, dz4, dw4);
        }
        double dx3 = dx4;
        double dy3 = dy4;
        double dz3 = dz0 - 3 * m_Constants[5];
        double dw3 = dw0 - 1 - 3 * m_Constants[5];
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 1, dx3, dy3, dz3, dw3);
        }
        double dx2 = dx4;
        double dy2 = dy0 - 3 * m_Constants[5];
        double dz2 = dz4;
        double dw2 = dw3;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 1, dx2, dy2, dz2, dw2);
        }
        double dx1 = dx0 - 3 * m_Constants[5];
        double dz1 = dz4;
        double dy1 = dy4;
        double dw1 = dw3;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 1, dx1, dy1, dz1, dw1);
        }
        dx0 = dx0 - 1 - 4 * m_Constants[5];
        dy0 = dy0 - 1 - 4 * m_Constants[5];
        dz0 = dz0 - 1 - 4 * m_Constants[5];
        dw0 = dw0 - 1 - 4 * m_Constants[5];
        double attn0 = 2 - dx0 * dx0 - dy0 * dy0 - dz0 * dz0 - dw0 * dw0;
        if (attn0 > 0) {
            attn0 *= attn0;
            value += attn0 * attn0 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 1, dx0, dy0, dz0, dw0);
        }
    }else if (inSum <= 2) {
        double aScore;
        int8_t aPoint;
        bool aIsBiggerSide = true;
        double bScore;
        int8_t bPoint;
        bool bIsBiggerSide = true;
        if (xins + yins > zins + wins) {
            aScore = xins + yins;
            aPoint = 0x03;
        }else{
            aScore = zins + wins;
            aPoint = 0x0C;
        }
        if (xins + zins > yins + wins) {
            bScore = xins + zins;
            bPoint = 0x05;
        }else{
            bScore = yins + wins;
            bPoint = 0x0A;
        }
        if (xins + wins > yins + zins) {
            double score = xins + wins;
            if (aScore >= bScore && score > bScore) {
                bScore = score;
                bPoint = 0x09;
            }else if (aScore < bScore && score > aScore) {
                aScore = score;
                aPoint = 0x09;
            }
        }else{
            double score = yins + zins;
            if (aScore >= bScore && score > bScore) {
                bScore = score;
                bPoint = 0x06;
            }
            else if (aScore < bScore && score > aScore) {
                aScore = score;
                aPoint = 0x06;
            }
        }
        double p1 = 2 - inSum + xins;
        if (aScore >= bScore && p1 > bScore) {
            bScore = p1;
            bPoint = 0x01;
            bIsBiggerSide = false;
        }else if (aScore < bScore && p1 > aScore) {
            aScore = p1;
            aPoint = 0x01;
            aIsBiggerSide = false;
        }
        double p2 = 2 - inSum + yins;
        if (aScore >= bScore && p2 > bScore) {
            bScore = p2;
            bPoint = 0x02;
            bIsBiggerSide = false;
        }else if (aScore < bScore && p2 > aScore) {
            aScore = p2;
            aPoint = 0x02;
            aIsBiggerSide = false;
        }
        double p3 = 2 - inSum + zins;
        if (aScore >= bScore && p3 > bScore) {
            bScore = p3;
            bPoint = 0x04;
            bIsBiggerSide = false;
        }else if (aScore < bScore && p3 > aScore) {
            aScore = p3;
            aPoint = 0x04;
            aIsBiggerSide = false;
        }
        double p4 = 2 - inSum + wins;
        if (aScore >= bScore && p4 > bScore) {
            bScore = p4;
            bPoint = 0x08;
            bIsBiggerSide = false;
        }else if (aScore < bScore && p4 > aScore) {
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
                    dx_ext0 = dx0 - 3 * m_Constants[5];
                    dx_ext1 = dx0 + 1 - 2 * m_Constants[5];
                }else{
                    xsv_ext0 = xsv_ext1 = xsb + 1;
                    dx_ext0 = dx0 - 1 - 3 * m_Constants[5];
                    dx_ext1 = dx0 - 1 - 2 * m_Constants[5];
                }
                if ((c1 & 0x02) == 0) {
                    ysv_ext0 = ysb;
                    ysv_ext1 = ysb - 1;
                    dy_ext0 = dy0 - 3 * m_Constants[5];
                    dy_ext1 = dy0 + 1 - 2 * m_Constants[5];
                }else{
                    ysv_ext0 = ysv_ext1 = ysb + 1;
                    dy_ext0 = dy0 - 1 - 3 * m_Constants[5];
                    dy_ext1 = dy0 - 1 - 2 * m_Constants[5];
                }
                if ((c1 & 0x04) == 0) {
                    zsv_ext0 = zsb;
                    zsv_ext1 = zsb - 1;
                    dz_ext0 = dz0 - 3 * m_Constants[5];
                    dz_ext1 = dz0 + 1 - 2 * m_Constants[5];
                }else{
                    zsv_ext0 = zsv_ext1 = zsb + 1;
                    dz_ext0 = dz0 - 1 - 3 * m_Constants[5];
                    dz_ext1 = dz0 - 1 - 2 * m_Constants[5];
                }
                if ((c1 & 0x08) == 0) {
                    wsv_ext0 = wsb;
                    wsv_ext1 = wsb - 1;
                    dw_ext0 = dw0 - 3 * m_Constants[5];
                    dw_ext1 = dw0 + 1 - 2 * m_Constants[5];
                }else{
                    wsv_ext0 = wsv_ext1 = wsb + 1;
                    dw_ext0 = dw0 - 1 - 3 * m_Constants[5];
                    dw_ext1 = dw0 - 1 - 2 * m_Constants[5];
                }
                xsv_ext2 = xsb;
                ysv_ext2 = ysb;
                zsv_ext2 = zsb;
                wsv_ext2 = wsb;
                dx_ext2 = dx0 - 2 * m_Constants[5];
                dy_ext2 = dy0 - 2 * m_Constants[5];
                dz_ext2 = dz0 - 2 * m_Constants[5];
                dw_ext2 = dw0 - 2 * m_Constants[5];
                if ((c2 & 0x01) != 0) {
                    xsv_ext2 += 2;
                    dx_ext2 -= 2;
                }else if ((c2 & 0x02) != 0) {
                    ysv_ext2 += 2;
                    dy_ext2 -= 2;
                }else if ((c2 & 0x04) != 0) {
                    zsv_ext2 += 2;
                    dz_ext2 -= 2;
                }else{
                    wsv_ext2 += 2;
                    dw_ext2 -= 2;
                }

            }else{
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
                    dx_ext0 = dx0 + 1 - m_Constants[5];
                    dx_ext1 = dx0 - m_Constants[5];
                }else{
                    xsv_ext0 = xsv_ext1 = xsb + 1;
                    dx_ext0 = dx_ext1 = dx0 - 1 - m_Constants[5];
                }
                if ((c & 0x02) == 0) {
                    ysv_ext0 = ysv_ext1 = ysb;
                    dy_ext0 = dy_ext1 = dy0 - m_Constants[5];
                    if ((c & 0x01) == 0x01){
                        ysv_ext0 -= 1;
                        dy_ext0 += 1;
                    }else{
                        ysv_ext1 -= 1;
                        dy_ext1 += 1;
                    }
                }else{
                    ysv_ext0 = ysv_ext1 = ysb + 1;
                    dy_ext0 = dy_ext1 = dy0 - 1 - m_Constants[5];
                }
                if ((c & 0x04) == 0) {
                    zsv_ext0 = zsv_ext1 = zsb;
                    dz_ext0 = dz_ext1 = dz0 - m_Constants[5];
                    if ((c & 0x03) == 0x03) {
                        zsv_ext0 -= 1;
                        dz_ext0 += 1;
                    }else{
                        zsv_ext1 -= 1;
                        dz_ext1 += 1;
                    }
                }else{
                    zsv_ext0 = zsv_ext1 = zsb + 1;
                    dz_ext0 = dz_ext1 = dz0 - 1 - m_Constants[5];
                }
                if ((c & 0x08) == 0) {
                    wsv_ext0 = wsb;
                    wsv_ext1 = wsb - 1;
                    dw_ext0 = dw0 - m_Constants[5];
                    dw_ext1 = dw0 + 1 - m_Constants[5];
                }else{
                    wsv_ext0 = wsv_ext1 = wsb + 1;
                    dw_ext0 = dw_ext1 = dw0 - 1 - m_Constants[5];
                }
            }
        }else{
            int8_t c1, c2;
            if (aIsBiggerSide) {
                c1 = aPoint;
                c2 = bPoint;
            }else{
                c1 = bPoint;
                c2 = aPoint;
            }
            if ((c1 & 0x01) == 0) {
                xsv_ext0 = xsb - 1;
                xsv_ext1 = xsb;
                dx_ext0 = dx0 + 1 - m_Constants[5];
                dx_ext1 = dx0 - m_Constants[5];
            }else{
                xsv_ext0 = xsv_ext1 = xsb + 1;
                dx_ext0 = dx_ext1 = dx0 - 1 - m_Constants[5];
            }
            if ((c1 & 0x02) == 0) {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0 - m_Constants[5];
                if ((c1 & 0x01) == 0x01) {
                    ysv_ext0 -= 1;
                    dy_ext0 += 1;
                }else{
                    ysv_ext1 -= 1;
                    dy_ext1 += 1;
                }
            }else{
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1 - m_Constants[5];
            }
            if ((c1 & 0x04) == 0) {
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz_ext1 = dz0 - m_Constants[5];
                if ((c1 & 0x03) == 0x03) {
                    zsv_ext0 -= 1;
                    dz_ext0 += 1;
                }else{
                    zsv_ext1 -= 1;
                    dz_ext1 += 1;
                }
            }else{
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz_ext1 = dz0 - 1 - m_Constants[5];
            }
            if ((c1 & 0x08) == 0) {
                wsv_ext0 = wsb;
                wsv_ext1 = wsb - 1;
                dw_ext0 = dw0 - m_Constants[5];
                dw_ext1 = dw0 + 1 - m_Constants[5];
            }else{
                wsv_ext0 = wsv_ext1 = wsb + 1;
                dw_ext0 = dw_ext1 = dw0 - 1 - m_Constants[5];
            }
            xsv_ext2 = xsb;
            ysv_ext2 = ysb;
            zsv_ext2 = zsb;
            wsv_ext2 = wsb;
            dx_ext2 = dx0 - 2 * m_Constants[5];
            dy_ext2 = dy0 - 2 * m_Constants[5];
            dz_ext2 = dz0 - 2 * m_Constants[5];
            dw_ext2 = dw0 - 2 * m_Constants[5];
            if ((c2 & 0x01) != 0) {
                xsv_ext2 += 2;
                dx_ext2 -= 2;
            }else if ((c2 & 0x02) != 0) {
                ysv_ext2 += 2;
                dy_ext2 -= 2;
            }else if ((c2 & 0x04) != 0) {
                zsv_ext2 += 2;
                dz_ext2 -= 2;
            }else{
                wsv_ext2 += 2;
                dw_ext2 -= 2;
            }
        }
        double dx1 = dx0 - 1 - m_Constants[5];
        double dy1 = dy0 - 0 - m_Constants[5];
        double dz1 = dz0 - 0 - m_Constants[5];
        double dw1 = dw0 - 0 - m_Constants[5];
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 0, dx1, dy1, dz1, dw1);
        }
        double dx2 = dx0 - 0 - m_Constants[5];
        double dy2 = dy0 - 1 - m_Constants[5];
        double dz2 = dz1;
        double dw2 = dw1;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 0, dx2, dy2, dz2, dw2);
        }
        double dx3 = dx2;
        double dy3 = dy1;
        double dz3 = dz0 - 1 - m_Constants[5];
        double dw3 = dw1;
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 0, dx3, dy3, dz3, dw3);
        }
        double dx4 = dx2;
        double dy4 = dy1;
        double dz4 = dz1;
        double dw4 = dw0 - 1 - m_Constants[5];
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 0, wsb + 1, dx4, dy4, dz4, dw4);
        }
        double dx5 = dx0 - 1 - 2 * m_Constants[5];
        double dy5 = dy0 - 1 - 2 * m_Constants[5];
        double dz5 = dz0 - 0 - 2 * m_Constants[5];
        double dw5 = dw0 - 0 - 2 * m_Constants[5];
        double attn5 = 2 - dx5 * dx5 - dy5 * dy5 - dz5 * dz5 - dw5 * dw5;
        if (attn5 > 0) {
            attn5 *= attn5;
            value += attn5 * attn5 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 0, dx5, dy5, dz5, dw5);
        }
        double dx6 = dx0 - 1 - 2 * m_Constants[5];
        double dy6 = dy0 - 0 - 2 * m_Constants[5];
        double dz6 = dz0 - 1 - 2 * m_Constants[5];
        double dw6 = dw0 - 0 - 2 * m_Constants[5];
        double attn6 = 2 - dx6 * dx6 - dy6 * dy6 - dz6 * dz6 - dw6 * dw6;
        if (attn6 > 0) {
            attn6 *= attn6;
            value += attn6 * attn6 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 0, dx6, dy6, dz6, dw6);
        }
        double dx7 = dx0 - 1 - 2 * m_Constants[5];
        double dy7 = dy0 - 0 - 2 * m_Constants[5];
        double dz7 = dz0 - 0 - 2 * m_Constants[5];
        double dw7 = dw0 - 1 - 2 * m_Constants[5];
        double attn7 = 2 - dx7 * dx7 - dy7 * dy7 - dz7 * dz7 - dw7 * dw7;
        if (attn7 > 0) {
            attn7 *= attn7;
            value += attn7 * attn7 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 1, dx7, dy7, dz7, dw7);
        }
        double dx8 = dx0 - 0 - 2 * m_Constants[5];
        double dy8 = dy0 - 1 - 2 * m_Constants[5];
        double dz8 = dz0 - 1 - 2 * m_Constants[5];
        double dw8 = dw0 - 0 - 2 * m_Constants[5];
        double attn8 = 2 - dx8 * dx8 - dy8 * dy8 - dz8 * dz8 - dw8 * dw8;
        if (attn8 > 0) {
            attn8 *= attn8;
            value += attn8 * attn8 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 0, dx8, dy8, dz8, dw8);
        }
        double dx9 = dx0 - 0 - 2 * m_Constants[5];
        double dy9 = dy0 - 1 - 2 * m_Constants[5];
        double dz9 = dz0 - 0 - 2 * m_Constants[5];
        double dw9 = dw0 - 1 - 2 * m_Constants[5];
        double attn9 = 2 - dx9 * dx9 - dy9 * dy9 - dz9 * dz9 - dw9 * dw9;
        if (attn9 > 0) {
            attn9 *= attn9;
            value += attn9 * attn9 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 1, dx9, dy9, dz9, dw9);
        }
        double dx10 = dx0 - 0 - 2 * m_Constants[5];
        double dy10 = dy0 - 0 - 2 * m_Constants[5];
        double dz10 = dz0 - 1 - 2 * m_Constants[5];
        double dw10 = dw0 - 1 - 2 * m_Constants[5];
        double attn10 = 2 - dx10 * dx10 - dy10 * dy10 - dz10 * dz10 - dw10 * dw10;
        if (attn10 > 0) {
            attn10 *= attn10;
            value += attn10 * attn10 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 1, dx10, dy10, dz10, dw10);
        }
    }else{
        double aScore;
        int8_t aPoint;
        bool aIsBiggerSide = true;
        double bScore;
        int8_t bPoint;
        bool bIsBiggerSide = true;
        if (xins + yins < zins + wins) {
            aScore = xins + yins;
            aPoint = 0x0C;
        }
        else {
            aScore = zins + wins;
            aPoint = 0x03;
        }
        if (xins + zins < yins + wins) {
            bScore = xins + zins;
            bPoint = 0x0A;
        }
        else {
            bScore = yins + wins;
            bPoint = 0x05;
        }
        if (xins + wins < yins + zins) {
            double score = xins + wins;
            if (aScore <= bScore && score < bScore) {
                bScore = score;
                bPoint = 0x06;
            }
            else if (aScore > bScore && score < aScore) {
                aScore = score;
                aPoint = 0x06;
            }
        }
        else {
            double score = yins + zins;
            if (aScore <= bScore && score < bScore) {
                bScore = score;
                bPoint = 0x09;
            }
            else if (aScore > bScore && score < aScore) {
                aScore = score;
                aPoint = 0x09;
            }
        }
        double p1 = 3 - inSum + xins;
        if (aScore <= bScore && p1 < bScore) {
            bScore = p1;
            bPoint = 0x0E;
            bIsBiggerSide = false;
        }
        else if (aScore > bScore && p1 < aScore) {
            aScore = p1;
            aPoint = 0x0E;
            aIsBiggerSide = false;
        }
        double p2 = 3 - inSum + yins;
        if (aScore <= bScore && p2 < bScore) {
            bScore = p2;
            bPoint = 0x0D;
            bIsBiggerSide = false;
        }
        else if (aScore > bScore && p2 < aScore) {
            aScore = p2;
            aPoint = 0x0D;
            aIsBiggerSide = false;
        }
        double p3 = 3 - inSum + zins;
        if (aScore <= bScore && p3 < bScore) {
            bScore = p3;
            bPoint = 0x0B;
            bIsBiggerSide = false;
        }
        else if (aScore > bScore && p3 < aScore) {
            aScore = p3;
            aPoint = 0x0B;
            aIsBiggerSide = false;
        }
        double p4 = 3 - inSum + wins;
        if (aScore <= bScore && p4 < bScore) {
            bScore = p4;
            bPoint = 0x07;
            bIsBiggerSide = false;
        }
        else if (aScore > bScore && p4 < aScore) {
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
                dx_ext0 = dx0 - m_Constants[5];
                dy_ext0 = dy0 - m_Constants[5];
                dz_ext0 = dz0 - m_Constants[5];
                dw_ext0 = dw0 - m_Constants[5];
                dx_ext1 = dx0 - 2 * m_Constants[5];
                dy_ext1 = dy0 - 2 * m_Constants[5];
                dz_ext1 = dz0 - 2 * m_Constants[5];
                dw_ext1 = dw0 - 2 * m_Constants[5];
                if ((c1 & 0x01) != 0) {
                    xsv_ext0 += 1;
                    dx_ext0 -= 1;
                    xsv_ext1 += 2;
                    dx_ext1 -= 2;
                }
                else if ((c1 & 0x02) != 0) {
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                    ysv_ext1 += 2;
                    dy_ext1 -= 2;
                }
                else if ((c1 & 0x04) != 0) {
                    zsv_ext0 += 1;
                    dz_ext0 -= 1;
                    zsv_ext1 += 2;
                    dz_ext1 -= 2;
                }
                else {
                    wsv_ext0 += 1;
                    dw_ext0 -= 1;
                    wsv_ext1 += 2;
                    dw_ext1 -= 2;
                }
                xsv_ext2 = xsb + 1;
                ysv_ext2 = ysb + 1;
                zsv_ext2 = zsb + 1;
                wsv_ext2 = wsb + 1;
                dx_ext2 = dx0 - 1 - 2 * m_Constants[5];
                dy_ext2 = dy0 - 1 - 2 * m_Constants[5];
                dz_ext2 = dz0 - 1 - 2 * m_Constants[5];
                dw_ext2 = dw0 - 1 - 2 * m_Constants[5];
                if ((c2 & 0x01) == 0) {
                    xsv_ext2 -= 2;
                    dx_ext2 += 2;
                }
                else if ((c2 & 0x02) == 0) {
                    ysv_ext2 -= 2;
                    dy_ext2 += 2;
                }
                else if ((c2 & 0x04) == 0) {
                    zsv_ext2 -= 2;
                    dz_ext2 += 2;
                }
                else {
                    wsv_ext2 -= 2;
                    dw_ext2 += 2;
                }
            }
            else {
                xsv_ext2 = xsb + 1;
                ysv_ext2 = ysb + 1;
                zsv_ext2 = zsb + 1;
                wsv_ext2 = wsb + 1;
                dx_ext2 = dx0 - 1 - 4 * m_Constants[5];
                dy_ext2 = dy0 - 1 - 4 * m_Constants[5];
                dz_ext2 = dz0 - 1 - 4 * m_Constants[5];
                dw_ext2 = dw0 - 1 - 4 * m_Constants[5];
                int8_t c = (int8_t)(aPoint & bPoint);
                if ((c & 0x01) != 0) {
                    xsv_ext0 = xsb + 2;
                    xsv_ext1 = xsb + 1;
                    dx_ext0 = dx0 - 2 - 3 * m_Constants[5];
                    dx_ext1 = dx0 - 1 - 3 * m_Constants[5];
                }
                else {
                    xsv_ext0 = xsv_ext1 = xsb;
                    dx_ext0 = dx_ext1 = dx0 - 3 * m_Constants[5];
                }
                if ((c & 0x02) != 0) {
                    ysv_ext0 = ysv_ext1 = ysb + 1;
                    dy_ext0 = dy_ext1 = dy0 - 1 - 3 * m_Constants[5];
                    if ((c & 0x01) == 0) {
                        ysv_ext0 += 1;
                        dy_ext0 -= 1;
                    }
                    else {
                        ysv_ext1 += 1;
                        dy_ext1 -= 1;
                    }
                }
                else {
                    ysv_ext0 = ysv_ext1 = ysb;
                    dy_ext0 = dy_ext1 = dy0 - 3 * m_Constants[5];
                }
                if ((c & 0x04) != 0) {
                    zsv_ext0 = zsv_ext1 = zsb + 1;
                    dz_ext0 = dz_ext1 = dz0 - 1 - 3 * m_Constants[5];
                    if ((c & 0x03) == 0) {
                        zsv_ext0 += 1;
                        dz_ext0 -= 1;
                    }
                    else {
                        zsv_ext1 += 1;
                        dz_ext1 -= 1;
                    }
                }
                else {
                    zsv_ext0 = zsv_ext1 = zsb;
                    dz_ext0 = dz_ext1 = dz0 - 3 * m_Constants[5];
                }
                if ((c & 0x08) != 0) {
                    wsv_ext0 = wsb + 1;
                    wsv_ext1 = wsb + 2;
                    dw_ext0 = dw0 - 1 - 3 * m_Constants[5];
                    dw_ext1 = dw0 - 2 - 3 * m_Constants[5];
                }
                else {
                    wsv_ext0 = wsv_ext1 = wsb;
                    dw_ext0 = dw_ext1 = dw0 - 3 * m_Constants[5];
                }
            }
        }
        else {
            int8_t c1, c2;
            if (aIsBiggerSide) {
                c1 = aPoint;
                c2 = bPoint;
            }
            else {
                c1 = bPoint;
                c2 = aPoint;
            }
            if ((c1 & 0x01) != 0) {
                xsv_ext0 = xsb + 2;
                xsv_ext1 = xsb + 1;
                dx_ext0 = dx0 - 2 - 3 * m_Constants[5];
                dx_ext1 = dx0 - 1 - 3 * m_Constants[5];
            }
            else {
                xsv_ext0 = xsv_ext1 = xsb;
                dx_ext0 = dx_ext1 = dx0 - 3 * m_Constants[5];
            }
            if ((c1 & 0x02) != 0) {
                ysv_ext0 = ysv_ext1 = ysb + 1;
                dy_ext0 = dy_ext1 = dy0 - 1 - 3 * m_Constants[5];
                if ((c1 & 0x01) == 0) {
                    ysv_ext0 += 1;
                    dy_ext0 -= 1;
                }
                else {
                    ysv_ext1 += 1;
                    dy_ext1 -= 1;
                }
            }
            else {
                ysv_ext0 = ysv_ext1 = ysb;
                dy_ext0 = dy_ext1 = dy0 - 3 * m_Constants[5];
            }
            if ((c1 & 0x04) != 0) {
                zsv_ext0 = zsv_ext1 = zsb + 1;
                dz_ext0 = dz_ext1 = dz0 - 1 - 3 * m_Constants[5];
                if ((c1 & 0x03) == 0) {
                    zsv_ext0 += 1;
                    dz_ext0 -= 1;
                }
                else {
                    zsv_ext1 += 1;
                    dz_ext1 -= 1;
                }
            }
            else {
                zsv_ext0 = zsv_ext1 = zsb;
                dz_ext0 = dz_ext1 = dz0 - 3 * m_Constants[5];
            }
            if ((c1 & 0x08) != 0) {
                wsv_ext0 = wsb + 1;
                wsv_ext1 = wsb + 2;
                dw_ext0 = dw0 - 1 - 3 * m_Constants[5];
                dw_ext1 = dw0 - 2 - 3 * m_Constants[5];
            }
            else {
                wsv_ext0 = wsv_ext1 = wsb;
                dw_ext0 = dw_ext1 = dw0 - 3 * m_Constants[5];
            }
            xsv_ext2 = xsb + 1;
            ysv_ext2 = ysb + 1;
            zsv_ext2 = zsb + 1;
            wsv_ext2 = wsb + 1;
            dx_ext2 = dx0 - 1 - 2 * m_Constants[5];
            dy_ext2 = dy0 - 1 - 2 * m_Constants[5];
            dz_ext2 = dz0 - 1 - 2 * m_Constants[5];
            dw_ext2 = dw0 - 1 - 2 * m_Constants[5];
            if ((c2 & 0x01) == 0) {
                xsv_ext2 -= 2;
                dx_ext2 += 2;
            }
            else if ((c2 & 0x02) == 0) {
                ysv_ext2 -= 2;
                dy_ext2 += 2;
            }
            else if ((c2 & 0x04) == 0) {
                zsv_ext2 -= 2;
                dz_ext2 += 2;
            }
            else {
                wsv_ext2 -= 2;
                dw_ext2 += 2;
            }
        }
        double dx4 = dx0 - 1 - 3 * m_Constants[5];
        double dy4 = dy0 - 1 - 3 * m_Constants[5];
        double dz4 = dz0 - 1 - 3 * m_Constants[5];
        double dw4 = dw0 - 3 * m_Constants[5];
        double attn4 = 2 - dx4 * dx4 - dy4 * dy4 - dz4 * dz4 - dw4 * dw4;
        if (attn4 > 0) {
            attn4 *= attn4;
            value += attn4 * attn4 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 1, wsb + 0, dx4, dy4, dz4, dw4);
        }
        double dx3 = dx4;
        double dy3 = dy4;
        double dz3 = dz0 - 3 * m_Constants[5];
        double dw3 = dw0 - 1 - 3 * m_Constants[5];
        double attn3 = 2 - dx3 * dx3 - dy3 * dy3 - dz3 * dz3 - dw3 * dw3;
        if (attn3 > 0) {
            attn3 *= attn3;
            value += attn3 * attn3 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 1, dx3, dy3, dz3, dw3);
        }
        double dx2 = dx4;
        double dy2 = dy0 - 3 * m_Constants[5];
        double dz2 = dz4;
        double dw2 = dw3;
        double attn2 = 2 - dx2 * dx2 - dy2 * dy2 - dz2 * dz2 - dw2 * dw2;
        if (attn2 > 0) {
            attn2 *= attn2;
            value += attn2 * attn2 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 1, dx2, dy2, dz2, dw2);
        }
        double dx1 = dx0 - 3 * m_Constants[5];
        double dz1 = dz4;
        double dy1 = dy4;
        double dw1 = dw3;
        double attn1 = 2 - dx1 * dx1 - dy1 * dy1 - dz1 * dz1 - dw1 * dw1;
        if (attn1 > 0) {
            attn1 *= attn1;
            value += attn1 * attn1 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 1, dx1, dy1, dz1, dw1);
        }
        double dx5 = dx0 - 1 - 2 * m_Constants[5];
        double dy5 = dy0 - 1 - 2 * m_Constants[5];
        double dz5 = dz0 - 0 - 2 * m_Constants[5];
        double dw5 = dw0 - 0 - 2 * m_Constants[5];
        double attn5 = 2 - dx5 * dx5 - dy5 * dy5 - dz5 * dz5 - dw5 * dw5;
        if (attn5 > 0) {
            attn5 *= attn5;
            value += attn5 * attn5 * internalExtrapolate(xsb + 1, ysb + 1, zsb + 0, wsb + 0, dx5, dy5, dz5, dw5);
        }
        double dx6 = dx0 - 1 - 2 * m_Constants[5];
        double dy6 = dy0 - 0 - 2 * m_Constants[5];
        double dz6 = dz0 - 1 - 2 * m_Constants[5];
        double dw6 = dw0 - 0 - 2 * m_Constants[5];
        double attn6 = 2 - dx6 * dx6 - dy6 * dy6 - dz6 * dz6 - dw6 * dw6;
        if (attn6 > 0) {
            attn6 *= attn6;
            value += attn6 * attn6 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 1, wsb + 0, dx6, dy6, dz6, dw6);
        }
        double dx7 = dx0 - 1 - 2 * m_Constants[5];
        double dy7 = dy0 - 0 - 2 * m_Constants[5];
        double dz7 = dz0 - 0 - 2 * m_Constants[5];
        double dw7 = dw0 - 1 - 2 * m_Constants[5];
        double attn7 = 2 - dx7 * dx7 - dy7 * dy7 - dz7 * dz7 - dw7 * dw7;
        if (attn7 > 0) {
            attn7 *= attn7;
            value += attn7 * attn7 * internalExtrapolate(xsb + 1, ysb + 0, zsb + 0, wsb + 1, dx7, dy7, dz7, dw7);
        }
        double dx8 = dx0 - 0 - 2 * m_Constants[5];
        double dy8 = dy0 - 1 - 2 * m_Constants[5];
        double dz8 = dz0 - 1 - 2 * m_Constants[5];
        double dw8 = dw0 - 0 - 2 * m_Constants[5];
        double attn8 = 2 - dx8 * dx8 - dy8 * dy8 - dz8 * dz8 - dw8 * dw8;
        if (attn8 > 0) {
            attn8 *= attn8;
            value += attn8 * attn8 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 1, wsb + 0, dx8, dy8, dz8, dw8);
        }
        double dx9 = dx0 - 0 - 2 * m_Constants[5];
        double dy9 = dy0 - 1 - 2 * m_Constants[5];
        double dz9 = dz0 - 0 - 2 * m_Constants[5];
        double dw9 = dw0 - 1 - 2 * m_Constants[5];
        double attn9 = 2 - dx9 * dx9 - dy9 * dy9 - dz9 * dz9 - dw9 * dw9;
        if (attn9 > 0) {
            attn9 *= attn9;
            value += attn9 * attn9 * internalExtrapolate(xsb + 0, ysb + 1, zsb + 0, wsb + 1, dx9, dy9, dz9, dw9);
        }
        double dx10 = dx0 - 0 - 2 * m_Constants[5];
        double dy10 = dy0 - 0 - 2 * m_Constants[5];
        double dz10 = dz0 - 1 - 2 * m_Constants[5];
        double dw10 = dw0 - 1 - 2 * m_Constants[5];
        double attn10 = 2 - dx10 * dx10 - dy10 * dy10 - dz10 * dz10 - dw10 * dw10;
        if (attn10 > 0) {
            attn10 *= attn10;
            value += attn10 * attn10 * internalExtrapolate(xsb + 0, ysb + 0, zsb + 1, wsb + 1, dx10, dy10, dz10, dw10);
        }
    }
    double attn_ext0 = 2 - dx_ext0 * dx_ext0 - dy_ext0 * dy_ext0 - dz_ext0 * dz_ext0 - dw_ext0 * dw_ext0;
    if (attn_ext0 > 0) {
        attn_ext0 *= attn_ext0;
        value += attn_ext0 * attn_ext0 * internalExtrapolate(xsv_ext0, ysv_ext0, zsv_ext0, wsv_ext0, dx_ext0, dy_ext0, dz_ext0, dw_ext0);
    }
    double attn_ext1 = 2 - dx_ext1 * dx_ext1 - dy_ext1 * dy_ext1 - dz_ext1 * dz_ext1 - dw_ext1 * dw_ext1;
    if (attn_ext1 > 0) {
        attn_ext1 *= attn_ext1;
        value += attn_ext1 * attn_ext1 * internalExtrapolate(xsv_ext1, ysv_ext1, zsv_ext1, wsv_ext1, dx_ext1, dy_ext1, dz_ext1, dw_ext1);
    }
    double attn_ext2 = 2 - dx_ext2 * dx_ext2 - dy_ext2 * dy_ext2 - dz_ext2 * dz_ext2 - dw_ext2 * dw_ext2;
    if (attn_ext2 > 0) {
        attn_ext2 *= attn_ext2;
        value += attn_ext2 * attn_ext2 * internalExtrapolate(xsv_ext2, ysv_ext2, zsv_ext2, wsv_ext2, dx_ext2, dy_ext2, dz_ext2, dw_ext2);
    }
    return value / m_Constants[8];
}
double SimplexNoise::noiseOpenSimplex2D(const double& x, const double& y, const unsigned long long& seed) {
    internalInitFromSeed(seed);
    return noiseOpenSimplex2D(x, y);
}
double SimplexNoise::noiseOpenSimplex3D(const double& x, const double& y, const double& z, const unsigned long long& seed) {
    internalInitFromSeed(seed);
    return noiseOpenSimplex3D(x, y, z);
}
double SimplexNoise::noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w, const unsigned long long& seed) {
    internalInitFromSeed(seed);
    return noiseOpenSimplex4D(x, y, z, w);
}




const double Noise::noiseOpenSimplex2D(const double& x, const double& y){
    return noise.noiseOpenSimplex2D(x,y); 
}
const double Noise::noiseOpenSimplex3D(const double& x, const double& y, const double& z){
    return noise.noiseOpenSimplex3D(x,y,z); 
}
const double Noise::noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w){
    return noise.noiseOpenSimplex4D(x,y,z,w); 
}
const double Noise::noiseOpenSimplex2D(const double& x, const double& y, const unsigned long long& seed){
    return noise.noiseOpenSimplex2D(x,y,seed); 
}
const double Noise::noiseOpenSimplex3D(const double& x, const double& y, const double& z, const unsigned long long& seed){ 
    return noise.noiseOpenSimplex3D(x,y,z,seed); 
}
const double Noise::noiseOpenSimplex4D(const double& x, const double& y, const double& z, const double& w, const unsigned long long& seed){ 
    return noise.noiseOpenSimplex4D(x,y,z,w,seed); 
}
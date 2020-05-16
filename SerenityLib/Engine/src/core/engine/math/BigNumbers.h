#pragma once
#ifndef ENGINE_MATH_BIG_NUMBERS_H
#define ENGINE_MATH_BIG_NUMBERS_H

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/multiprecision/cpp_dec_float.hpp>
using boost::multiprecision::cpp_dec_float;
using boost::multiprecision::cpp_int;

namespace Engine {
    using big_int       = boost::multiprecision::cpp_int;

    using big_float_40  = boost::multiprecision::number<cpp_dec_float<40U>>;
    using big_float_50  = boost::multiprecision::number<cpp_dec_float<50U>>;
    using big_float_60  = boost::multiprecision::number<cpp_dec_float<60U>>;
    using big_float_64  = boost::multiprecision::number<cpp_dec_float<64U>>;
    using big_float_70  = boost::multiprecision::number<cpp_dec_float<70U>>;
    using big_float_80  = boost::multiprecision::number<cpp_dec_float<80U>>;
    using big_float_90  = boost::multiprecision::number<cpp_dec_float<90U>>;
    using big_float_100 = boost::multiprecision::number<cpp_dec_float<100U>>;
    using big_float_128 = boost::multiprecision::number<cpp_dec_float<128U>>;
    using big_float_256 = boost::multiprecision::number<cpp_dec_float<256U>>;
    using big_float_512 = boost::multiprecision::number<cpp_dec_float<512U>>;
}

#endif
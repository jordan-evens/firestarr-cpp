/* SPDX-License-Identifier: AGPL-3.0-or-later */
#ifndef FS_UNSTABLE_H
#define FS_UNSTABLE_H
#define _USE_MATH_DEFINES
#include <cfloat>
#include <cmath>
// HACK: M_PI not showing up on Windows
#ifndef M_PI
#include <math.h>
#endif
namespace fs
{
// fixed-size float types aren't defined until C++23
#ifndef _Float32
#if __STDCPP_FLOAT32_T__
#define _Float32 float32_t
#elif 24 == FLT_MANT_DIG
#define _Float32 float
#elif 24 == DBL_MANT_DIG
#define _Float32 double
#else
#define _Float32 long double
#endif
#endif
#ifndef _Float64
#if __STDCPP_FLOAT64_T__
#define _Float64 float64_t
#elif 53 == FLT_MANT_DIG
#define _Float64 float
#elif 53 == DBL_MANT_DIG
#define _Float64 double
#else
#define _Float64 long double
#endif
#endif
static_assert(4 == sizeof(_Float32), "Could not determine 32-bit float type");
static_assert(8 == sizeof(_Float64), "Could not determine 64-bit float type");
// Type used for math operations
using MathSize = _Float64;
/**
 * \brief Return base raised to the power N
 * \tparam N Power to raise to
 * \tparam T Type passed and returned
 * \param base Base to raise to power N
 * \return base raised to power N
 */
template <unsigned int N, class T>
[[nodiscard]] constexpr T pow_int(const T& base)
{
  // https://stackoverflow.com/questions/16443682/c-power-of-integer-template-meta-programming
  return N == 0     ? 1
       : N % 2 == 0 ? pow_int<N / 2, T>(base) * pow_int<N / 2, T>(base)
                    : base * pow_int<(N - 1) / 2, T>(base) * pow_int<(N - 1) / 2, T>(base);
}
// HACK: force rounding so that precision matches across platforms
template <size_t Precision>
class PreciseValue
{
  static inline MathSize round_value(const MathSize value) noexcept
  {
    static constexpr auto round_by = pow_int<Precision>(10);
    const auto i = static_cast<size_t>(value * round_by);
    const auto v = static_cast<MathSize>(i) / static_cast<MathSize>(round_by);
    return v;
  }

public:
  MathSize value{};
  constexpr PreciseValue() = default;
  constexpr PreciseValue(const MathSize& v) noexcept : value{round_value(v)} { }
  constexpr operator double() const noexcept { return value; }
};
using RoundedValue = PreciseValue<8>;
// HACK: use for replicating old LookupTable rounding
using LookupValue = PreciseValue<1>;
// HACK: override these so unstable code doesn't change results of functions
// in debug vs release version
MathSize cos(const MathSize angle) noexcept;
MathSize sin(const MathSize angle) noexcept;
}
#endif

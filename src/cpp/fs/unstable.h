/* SPDX-License-Identifier: AGPL-3.0-or-later */
#ifndef FS_UNSTABLE_H
#define FS_UNSTABLE_H
#define CHECK_ROUNDING 1
// #undef CHECK_ROUNDING
#define DEBUG_ROUNDING 1
#undef DEBUG_ROUNDING
#if defined(DEBUG_ROUNDING) || defined(CHECK_ROUNDING)
#include <cinttypes>
#endif
#include <cstdint>
#include <cstdio>
#include <limits>
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
using MathConstantSize = _Float64;
/**
 * \brief Return base raised to the power N
 * \tparam N Power to raise to
 * \tparam T Type passed and returned
 * \param base Base to raise to power N
 * \return base raised to power N
 */
template <size_t N, class T>
[[nodiscard]] constexpr T pow_int(const T& base)
{
  // https://stackoverflow.com/questions/16443682/c-power-of-integer-template-meta-programming
  return N == 0     ? 1
       : N % 2 == 0 ? pow_int<N / 2, T>(base) * pow_int<N / 2, T>(base)
                    : base * pow_int<(N - 1) / 2, T>(base) * pow_int<(N - 1) / 2, T>(base);
}
// HACK: force rounding so that precision matches across platforms
// HACK: float values seem to deviate at 18 digits, but round to have MaxFloatValue not overflow
static constexpr size_t RoundedDigits{15};
static constexpr size_t RoundedMax{1'000};
// Can go 300k+ for S3. Don't want to use int but don't need high precision
static constexpr size_t IntensityDigits{12};
static constexpr size_t IntensityMax{999'999};
// HACK: use for replicating old LookupTable rounding
static constexpr size_t LookupDigits{1};
static constexpr size_t LookupMax{1'000};
template <size_t RoundByDigits, size_t MaxValueAtLeast>
class PreciseValue
{
  static inline MathSize round_value(const MathSize value) noexcept
  {
    static constexpr int64_t RoundBy{pow_int<RoundByDigits>(static_cast<int64_t>(10))};
    static_assert(0 == RoundBy % 10);
    static_assert(10 <= RoundBy);
    static_assert(std::numeric_limits<int64_t>::max() > RoundBy);
    // digits10 is maximum digits that are guaranteed to convert to/from type, so no need for buffer
    // stay a few decimals from max
    static constexpr size_t BufferDigits{0};
    static_assert(std::numeric_limits<int64_t>::digits10 > (RoundByDigits + BufferDigits));
    // static constexpr size_t Buffer = pow_int<BufferDigits>(10);
    static constexpr int64_t MaxFloatValue{
      pow_int<std::numeric_limits<int64_t>::digits10 - (RoundByDigits + BufferDigits)>(
        static_cast<int64_t>(10)
      )
    };
    static_assert(
      static_cast<size_t>(std::numeric_limits<int64_t>::max())
      > (static_cast<size_t>(RoundBy) * static_cast<size_t>(MaxFloatValue))
    );
    // at least same as maximum possible value
    static_assert(MaxFloatValue >= MaxValueAtLeast);
#if defined(DEBUG_ROUNDING) || defined(CHECK_ROUNDING)
    static constexpr auto MaxValue{MaxFloatValue};
    static constexpr auto MinValue{-MaxFloatValue};
    static constexpr auto MaxIntValue{
      static_cast<int64_t>(MaxValue * static_cast<MathSize>(RoundBy))
    };
    static constexpr auto MinIntValue{
      static_cast<int64_t>(MinValue * static_cast<MathSize>(RoundBy))
    };
    // ensure not out of bounds
    if (MaxFloatValue < value || value < -MaxFloatValue)
    {
      printf(
        "Rounding value %0.100g results in it going out of bounds (%" PRId64 ", %" PRId64 ")\n",
        value,
        MaxValue,
        MinValue
      );
      exit(-1);
    }
#endif
#if defined(DEBUG_ROUNDING)
    if constexpr (RoundByDigits > IntensityDigits)
    {
      static MathSize min_recorded{std::numeric_limits<MathSize>::max()};
      static MathSize max_recorded{std::numeric_limits<MathSize>::min()};
      static MathSize min_magnitude{std::numeric_limits<MathSize>::max()};
      static MathSize max_magnitude{std::numeric_limits<MathSize>::min()};
      if (value < min_recorded)
      {
        // if (min_magnitude > value * 10)
        if (value < -1)
        {
          printf(
            "Minimum recorded value is now %0.100g of possible %" PRId64 " from %0.100g\n",
            value,
            MinValue,
            +min_magnitude
          );
          min_magnitude = value;
        }
        min_recorded = value;
      }
      if (max_recorded < value)
      {
        // if (max_magnitude * 10 < value)
        // 16k -> rso = FuelType::criticalRos(spread.sfc_, critical_surface_intensity);
        // 388k -> spread.is_crown_ = FuelType::isCrown(critical_surface_intensity, sfi);
        if (value > 1000)
        {
          printf(
            "Maximum recorded value is now %0.100g of possible %" PRId64 " from %0.100g\n",
            value,
            MaxValue,
            +max_magnitude
          );
          max_magnitude = value;
        }
        max_recorded = value;
      }
    }
#endif
    const auto i = static_cast<int64_t>(value * static_cast<MathSize>(RoundBy));
#if defined(DEBUG_ROUNDING) || defined(CHECK_ROUNDING)
    // ensure not out of bounds
    if (MaxIntValue < i || i < MinIntValue)
    {
      printf(
        "Rounding value %0.100g -> %" PRId64 " results in it going out of bounds (%" PRId64
        ", %" PRId64 ")\n",
        value,
        i,
        MaxIntValue,
        MinIntValue
      );
      exit(-1);
    }
#endif
    const auto v = static_cast<MathSize>(i) / static_cast<MathSize>(RoundBy);
    return v;
  }

public:
  MathSize value{};
  constexpr PreciseValue() = default;
  PreciseValue(const MathSize& v) noexcept : value{round_value(v)} { }
  constexpr operator double() const noexcept { return value; }
};
using RoundedValue = PreciseValue<RoundedDigits, RoundedMax>;
using RoundedIntensity = PreciseValue<IntensityDigits, IntensityMax>;
using LookupValue = PreciseValue<LookupDigits, LookupMax>;
// HACK: override these so unstable code doesn't change results of functions
// in debug vs release version
MathSize cos(const MathSize angle) noexcept;
MathSize sin(const MathSize angle) noexcept;
}
#endif

/* SPDX-License-Identifier: AGPL-3.0-or-later */
#include "Duff.h"
#include "../Log.h"
namespace fs::testing
{
int compare_duff(const duff::Duff& a, const duff::Duff& b)
{
  static constexpr int RESOLUTION = 10000;
  static constexpr MathSize RANGE = 250.0;
  // check %, so 1 decimal is fine
  static constexpr auto EPSILON = static_cast<MathSize>(1e-1);
  logging::check_equal_verbose(logging::level::debug, a.ash, b.ash, "ash");
  logging::check_equal_verbose(logging::level::debug, a.rho, b.rho, "rho");
  logging::check_equal_verbose(logging::level::debug, a.b0, b.b0, "b0");
  logging::check_equal_verbose(logging::level::debug, a.b1, b.b1, "b1");
  logging::check_equal_verbose(logging::level::debug, a.b2, b.b2, "b2");
  logging::check_equal_verbose(logging::level::debug, a.b3, b.b3, "b3");
  for (auto i = 0; i < RESOLUTION; ++i)
  {
    const MathSize mc = RANGE * i / RESOLUTION;
    const auto msg = std::format("probability of survival (mc = {})", mc);
    logging::check_tolerance(
      EPSILON, a.probabilityOfSurvival(mc), b.probabilityOfSurvival(mc), msg.c_str()
    );
  }
  return 0;
}
// FIX: this was used to compare to the old template version, but doesn't work now
//      left for reference for now so idea could be used for more tests
int test_duff(const int argc, const char* const argv[])
{
  std::ignore = argc;
  std::ignore = argv;
  logging::info("Testing Duff");
  static auto cmp_duff = [](const string name, const duff::Duff& a, const duff::Duff& b) {
    logging::info("Checking {:s}", name);
    testing::compare_duff(a, b);
  };
  cmp_duff("SphagnumUpper", duff::SphagnumUpper, duff::SphagnumUpper);
  cmp_duff("FeatherMoss", duff::FeatherMoss, duff::FeatherMoss);
  cmp_duff("Reindeer", duff::Reindeer, duff::Reindeer);
  cmp_duff("WhiteSpruce", duff::WhiteSpruce, duff::WhiteSpruce);
  cmp_duff("Peat", duff::Peat, duff::Peat);
  cmp_duff("PeatMuck", duff::PeatMuck, duff::PeatMuck);
  cmp_duff("PineSeney", duff::PineSeney, duff::PineSeney);
  cmp_duff("SprucePine", duff::SprucePine, duff::SprucePine);
  return 0;
}
}
[[nodiscard]] fs::ThresholdSize fs::duff::Duff::probabilityOfSurvival(const MathSize mc_pct
) const noexcept
{
  /**
   * \brief Constant part of ignition probability equation [eq Ig-1]
   */
  const auto ConstantPart = b0 + b2 * ash + b3 * rho;
  const auto d = 1 + exp(-(b1 * mc_pct + ConstantPart));
  if (0 == d)
  {
    return 1.0;
  }
  return 1.0 / d;
}
[[nodiscard]] bool fs::duff::Duff::operator==(const Duff& rhs) const
{
  // HACK: only equivalent if identical
  return this == &rhs;
}
[[nodiscard]] bool fs::duff::Duff::operator!=(const Duff& rhs) const { return !operator==(rhs); }

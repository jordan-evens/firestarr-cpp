/* SPDX-License-Identifier: AGPL-3.0-or-later */
#include "StandardFuel.h"
#include "../util/LookupTable.h"
namespace fs::fuel
{
/**
 * \brief Limit to slope when calculating ISI
 */
static constexpr MathSize SLOPE_LIMIT_ISI = 0.01;
/**
 * \brief Calculate standard foliar moisture effect (FME) based on FMC [ST-X-3 eq 61]
 * \param fmc Foliar Moisture Content (FMC)
 * \return Standard foliar moisture effect (FME) based on FMC [ST-X-3 eq 61]
 */
[[nodiscard]] static MathSize calculate_standard_foliar_moisture_fmc(const MathSize fmc) noexcept
{
  return pow_int<4>(1.5 - 0.00275 * fmc) / (460.0 + 25.9 * fmc) / 0.778 * 1000.0;
}
/**
 * \brief Standard foliar moisture effect (FME) based on FMC [ST-X-3 eq 61]
 */
static const LookupTable<&calculate_standard_foliar_moisture_fmc> STANDARD_FOLIAR_MOISTURE_FMC{};
/**
 * \brief Crown fire spread rate (m/min) / Foliar Moisture Effect (RSC / (FME / FME_avg)) [ST-X-3 eq
 * 64]
 * \param isi Initial Spread Index
 * \return RSC / (FME / FME_avg) [ST-X-3 eq 64]
 */
[[nodiscard]] static MathSize calculate_standard_foliar_moisture_isi(const MathSize isi) noexcept
{
  return 60.0 * (1.0 - exp(-0.0497 * isi));
}
/**
 * \brief Crown fire spread rate (m/min) / Foliar Moisture Effect (RSC / (FME / FME_avg)) [ST-X-3 eq
 * 64]
 * \return RSC / (FME / FME_avg) [ST-X-3 eq 64]
 */
static const LookupTable<&calculate_standard_foliar_moisture_isi> STANDARD_FOLIAR_MOISTURE_ISI{};
/**
 * \brief Length to Breadth ratio [ST-X-3 eq 79]
 * \param ws Wind Speed (km/h)
 * \return Length to Breadth ratio [ST-X-3 eq 79]
 */
[[nodiscard]] static MathSize calculate_standard_length_to_breadth(const MathSize ws) noexcept
{
  return 1.0 + 8.729 * pow(1.0 - exp(-0.030 * ws), 2.155);
}
/**
 * \brief Length to Breadth ratio [ST-X-3 eq 79]
 * \return Length to Breadth ratio [ST-X-3 eq 79]
 */
static const LookupTable<&calculate_standard_length_to_breadth> STANDARD_LENGTH_TO_BREADTH{};
StandardFuel::StandardFuel(
  const FuelCodeSize& code,
  const char* name,
  const bool can_crown,
  const LogValue log_q,
  const MathSize a,
  const MathSize b,
  const MathSize c,
  const MathSize bui0,
  const MathSize cbh,
  const MathSize cfl,
  const MathSize bulk_density,
  const MathSize inorganic_percent,
  const MathSize duff_depth,
  const Duff* duff_ffmc,
  const Duff* duff_dmc
) noexcept
  : FuelBase(
      code,
      name,
      can_crown,
      // FIX: change these to actual numbers when we get to derived class
      bulk_density / 1000.0,
      inorganic_percent / 100.0,
      duff_depth / 10.0,
      duff_ffmc,
      duff_dmc
    ),
    log_q_(log_q), a_(a), b_(b), c_(c), bui0_(bui0), cbh_(cbh), cfl_(cfl)
{
  assert(-negB() < 1);
  assert(StandardFuel::c() < 10 && StandardFuel::c() > 1);
}
StandardFuel::StandardFuel(
  const FuelCodeSize& code,
  const char* name,
  const bool can_crown,
  const LogValue log_q,
  const MathSize a,
  const MathSize b,
  const MathSize c,
  const MathSize bui0,
  const MathSize cbh,
  const MathSize cfl,
  const MathSize bulk_density,
  const MathSize inorganic_percent,
  const MathSize duff_depth,
  const Duff* duff
) noexcept
  : StandardFuel(
      code,
      name,
      can_crown,
      log_q,
      a,
      b,
      c,
      bui0,
      cbh,
      cfl,
      bulk_density,
      inorganic_percent,
      duff_depth,
      duff,
      duff
    )
{ }
[[nodiscard]] MathSize StandardFuel::rosBasic(const MathSize isi) const noexcept
{
  return a() * pow(1.0 - exp(negB() * isi), c());
}
MathSize StandardFuel::crownConsumption(const MathSize cfb) const noexcept { return cfl() * cfb; }
[[nodiscard]] MathSize StandardFuel::limitIsf(const MathSize mu, const MathSize rsf) const noexcept
{
  return (1.0 / negB())
       * log(max(SLOPE_LIMIT_ISI, (rsf > 0.0) ? (1.0 - pow((rsf / (mu * a())), (1.0 / c()))) : 1.0)
       );
}
[[nodiscard]] MathSize StandardFuel::criticalSurfaceIntensity(const SpreadInfo& spread
) const noexcept
{
  return 0.001 * pow(cbh(), 1.5) * pow(460.0 + 25.9 * spread.foliarMoisture(), 1.5);
}
[[nodiscard]] MathSize StandardFuel::lengthToBreadth(const MathSize ws) const noexcept
{
  return STANDARD_LENGTH_TO_BREADTH(ws);
}
MathSize StandardFuel::finalRos(const SpreadInfo&, MathSize, MathSize, const MathSize rss)
  const noexcept
{
  return rss;
}
[[nodiscard]] MathSize StandardFuel::buiEffect(const MathSize bui) const noexcept
{
  return (0 < bui) ? exp(50.0 * log_q_.value * ((1.0 / bui) - (1.0 / bui0()))) : 1.0;
}
[[nodiscard]] MathSize StandardFuel::bui0() const noexcept { return bui0_; }
[[nodiscard]] MathSize StandardFuel::cbh() const { return cbh_; }
[[nodiscard]] MathSize StandardFuel::cfl() const { return cfl_ / 100.0; }
[[nodiscard]] MathSize StandardFuel::a() const noexcept { return a_; }
[[nodiscard]] MathSize StandardFuel::negB() const noexcept
{
  // the only places this gets used it gets negated so just store it that way
  return -b_ / 10000.0;
}
[[nodiscard]] MathSize StandardFuel::c() const noexcept { return c_ / 100.0; }
[[nodiscard]] MathSize StandardFuel::crownRateOfSpread(
  const MathSize isi,
  const MathSize fmc
) noexcept
{
  return STANDARD_FOLIAR_MOISTURE_ISI(isi) * STANDARD_FOLIAR_MOISTURE_FMC(fmc);
}
[[nodiscard]] MathSize StandardFuel::logQ() const noexcept { return log_q_.value; }
}

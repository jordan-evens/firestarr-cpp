/* SPDX-License-Identifier: AGPL-3.0-or-later */
#include "unstable.h"
#include <cfenv>
namespace fs
{
MathSize cos(const MathSize angle) noexcept { return static_cast<MathSize>(std::cos(angle)); }
MathSize sin(const MathSize angle) noexcept { return static_cast<MathSize>(std::sin(angle)); }
MathSize acos(const MathSize angle) noexcept
{
  // FIX: change to use RAII
  std::fenv_t env{};
  std::feholdexcept(&env);
  const auto v = std::acos(angle);
  // clear FE_INVALID but not other exceptions
  std::feclearexcept(FE_INVALID);
  std::feupdateenv(&env);
  return v;
}
}
void setfperrors()
{
  // FE_INEXACT happens in stoa()
  fexcept_t excepts{};
  fesetexceptflag(&excepts, FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID);
}

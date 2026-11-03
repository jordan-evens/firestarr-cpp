/* SPDX-License-Identifier: AGPL-3.0-or-later */
#ifndef FS_TEST_H
#define FS_TEST_H
#include "../fs/log/Log.h"
#include "../fs/unstable.h"
namespace fs::testing
{
template <class T>
auto check_range(
  const char* name_fct,
  const char* name_param,
  const auto fct_a,
  const auto fct_b,
  const MathSize epsilon,
  const T& it
)
{
  logging::debug("Checking {:s}", name_fct);
  std::for_each(
#if !defined(__APPLE__) || !defined(__clang__)
    // apple clang doesn't support this?
    std::execution::par_unseq,
#endif
    it.begin(),
    it.end(),
    [&](const auto& v) {
      const auto msg = std::format("{} ({} = {})", name_fct, name_param, v);
      logging::check_tolerance(epsilon, fct_a(v), fct_b(v), msg.c_str());
      logging::verbose("{:s}", msg);
    }
  );
}
}
#endif

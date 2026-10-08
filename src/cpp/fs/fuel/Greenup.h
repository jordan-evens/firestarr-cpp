/* SPDX-License-Identifier: AGPL-3.0-or-later */
#ifndef FS_GREENUP_H
#define FS_GREENUP_H
namespace fs::fuel
{
/**
 * \brief Calculate if green-up has occurred
 * \param nd Difference between date and the date of minimum foliar moisture content
 * \return Whether or no green-up has occurred
 */
[[nodiscard]] bool calculate_is_green(const int nd);
[[nodiscard]] int calculate_grass_curing(const int nd);
}
#endif

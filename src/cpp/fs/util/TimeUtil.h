/* SPDX-License-Identifier: AGPL-3.0-or-later */
#ifndef FS_TIME_UTIL_H
#define FS_TIME_UTIL_H
#include <ctime>
namespace fs
{
// HACK: define in std since not on windows
#ifdef _WIN32
struct tm* localtime_r(const time_t* timer, struct tm* result);
#else
using ::localtime_r;
#endif
/**
 * \brief Calculate tm fields from values already there
 * @param t tm object to update
 */
void fix_tm(tm* t);
}
#endif

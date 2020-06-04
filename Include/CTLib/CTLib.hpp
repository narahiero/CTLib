//////////////////////////////////////////////////
//  Copyright (c) 2020 Nara Hiero
//
// This file is licensed under GPLv3+
// Refer to the `License.txt` file included.
//////////////////////////////////////////////////

#pragma once

/*! @dir CTLib
 * 
 *  @brief The directory containing all CT Lib headers.
 */


/*! @file CTLib.hpp
 * 
 *  @brief The main header of CT Lib.
 */


#include <string>


/*! @name Version Macros
 *  @{
 */

/*! @brief CT Lib headers' version major.
 * 
 *  The CT Lib version major increments only when huge backward-incompatible
 *  API changes are made.
 * 
 *  @note The version macros represents the version of the headers and **NOT**
 *  the binaries.
 */
#define CT_LIB_VERSION_MAJOR 0

/*! @brief CT Lib headers' version minor.
 * 
 *  The CT Lib version minor increments when API changes are made in a
 *  backward-compatible way.
 * 
 *  @note The version macros represents the version of the headers and **NOT**
 *  the binaries.
 */
#define CT_LIB_VERSION_MINOR 3

/*! @brief CT Lib headers' version patch.
 * 
 *  The CT Lib version patch increments whenever small changes to the source
 *  code are made, such as bug fixes and security enhancements.
 * 
 *  @note The version macros represents the version of the headers and **NOT**
 *  the binaries.
 */
#define CT_LIB_VERSION_PATCH 0

/*! @} name Version Macros */


namespace CTLib
{

/*! @brief Queries the CT Lib version of the binaries.
 * 
 *  @param[out] major Will be set to the version major.
 *  @param[out] minor Will be set to the version minor.
 *  @param[out] patch Will be set to the version patch.
 */
void getVersion(int* major, int* minor, int* patch);

/*! @brief Returns the version in an std::string.
 * 
 *  A new std::string with the contents of ctlib::getVersionCString() is
 *  created and returned every time this function is called.
 * 
 *  @return The version string.
 */
std::string getVersionString();

/*! @brief Returns the version in a C-style string.
 * 
 *  The returned string is a C-style string formatted as `"MAJ.MIN.PAT"`.
 *  Note that the returned value is always the same pointer.
 * 
 *  @return The version string.
 */
const char* getVersionCString();

/*! @brief Ensures the CT Lib version of the binaries matches the arguments.
 * 
 *  This function can be used to ensure the headers' version matches the
 *  binaries' by passing the version macros such as:
 * 
 *  ~~~{.cpp}
 *  ctlib::binariesVersionMatches(
 *      CT_LIB_VERSION_MAJOR, CT_LIB_VERSION_MINOR, CT_LIB_VERSION_PATCH
 *  )
 *  ~~~
 * 
 *  Doing this will prevent developers from using outdated or more recent
 *  headers than the binaries.
 * 
 *  @param[in] major The version major to compare.
 *  @param[in] minor The version minor to compare.
 *  @param[in] patch The version patch to compare.
 * 
 *  @return Whether the binaries version matches the arguments.
 */
bool binariesVersionMatches(int major, int minor, int patch);
}

########################################
# General CMake setup
########################################

cmake_minimum_required(VERSION 3.12)

project(CTLib VERSION 0.0.0)


########################################
# Constants
########################################

set(CT_LIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")
set(CT_LIB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")


########################################
# Options
########################################

# General
option(CT_LIB_BUILD_DOCS "Build HTML docs" ON)

# Modules
option(CT_LIB_MODULE_BRRES "Include BRRES module" ON)


########################################
# Find packages
########################################

# Find Doxygen
if (CT_LIB_BUILD_DOCS)
    find_package(Doxygen)
endif()


########################################
# Add subdirectories
########################################

add_subdirectory(Source)

if (CT_LIB_BUILD_DOCS AND DOXYGEN_FOUND)
    add_subdirectory(Docs)
endif()

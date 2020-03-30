########################################
# General CMake setup
########################################

cmake_minimum_required(VERSION 3.11)

project(CTLib VERSION 0.0.0)


########################################
# Constants
########################################

# Directories
set(CT_LIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/Include")
set(CT_LIB_SOURCE_DIR "${PROJECT_SOURCE_DIR}/Source")

# Whether CTLib is current project
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CT_LIB_IS_CURRENT TRUE)
else()
    set(CT_LIB_IS_CURRENT FALSE)
endif()


########################################
# Options
########################################

# General
option(CT_LIB_BUILD_DOCS "Build HTML docs" ON)
option(CT_LIB_BUILD_TESTS "Build tests" ON)

# Modules
option(CT_LIB_MODULE_YAZ "Include Yaz module" ON)
option(CT_LIB_MODULE_U8 "Include U8 module" ON)
option(CT_LIB_MODULE_BRRES "Include BRRES module" ON)

# Miscellaneous
option(CT_LIB_FORCE_BUILD_TESTS "Build tests even if CTLib is not current project" OFF)


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

if ((CT_LIB_IS_CURRENT AND CT_LIB_BUILD_TESTS) OR CT_LIB_FORCE_BUILD_TESTS)
    enable_testing()
    include(GoogleTest)
    add_subdirectory(Tests)
endif()

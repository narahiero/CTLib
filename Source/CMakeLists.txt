########################################
# Setup target
########################################

add_library(CTLib STATIC
    "${CT_LIB_INCLUDE_DIR}/CTLib/CTLib.hpp" CTLib.cpp
    "${CT_LIB_INCLUDE_DIR}/CTLib/Utilities.hpp" Utilities.cpp
    "${CT_LIB_INCLUDE_DIR}/CTLib/Memory.hpp" Memory.cpp
)

# Set C++ settings
target_compile_features(CTLib PUBLIC cxx_std_17)
set_target_properties(CTLib PROPERTIES CXX_EXTENSIONS OFF)

# Add include directory
target_include_directories(CTLib PUBLIC "${CT_LIB_INCLUDE_DIR}")


########################################
# Add modules
########################################

# Yaz module
if(CT_LIB_MODULE_YAZ)
    target_sources(CTLib PRIVATE
        "${CT_LIB_INCLUDE_DIR}/CTLib/Yaz.hpp"
        Yaz/Yaz.cpp
        Yaz/Decompress.cpp
    )
endif()

# BRRES module
if(CT_LIB_MODULE_BRRES)
    target_sources(CTLib PRIVATE
        "${CT_LIB_INCLUDE_DIR}/CTLib/BRRES.hpp"
        BRRES/BRRES.cpp
    )
endif()

########################################
# Setup target
########################################

add_library(CTLib STATIC
    "${CT_LIB_INCLUDE_DIR}/CTLib/CTLib.hpp" CTLib.cpp
    "${CT_LIB_INCLUDE_DIR}/CTLib/Math.hpp" Math.cpp
    "${CT_LIB_INCLUDE_DIR}/CTLib/Utilities.hpp" Utilities.cpp
    "${CT_LIB_INCLUDE_DIR}/CTLib/Memory.hpp" Memory.cpp
)

# Set C++ settings
target_compile_features(CTLib PUBLIC cxx_std_17)
set_target_properties(CTLib PROPERTIES CXX_EXTENSIONS OFF)

# Add include directory
target_include_directories(CTLib PUBLIC "${CT_LIB_INCLUDE_DIR}" "${CT_LIB_SOURCE_DIR}")

# Add dependencies
target_link_libraries(CTLib ${CT_LIB_DEPS})


########################################
# Add modules
########################################

# Image sources
target_sources(CTLib PRIVATE
    "${CT_LIB_INCLUDE_DIR}/CTLib/Image.hpp"
    Image/Image.cpp
    Image/Encode.cpp
    Image/Decode.cpp
)

# Yaz module
if(CT_LIB_MODULE_YAZ)
    target_sources(CTLib PRIVATE
        "${CT_LIB_INCLUDE_DIR}/CTLib/Yaz.hpp"
        Yaz/Yaz.cpp
        Yaz/Compress.cpp
        Yaz/Decompress.cpp
    )
endif()

# U8 module
if(CT_LIB_MODULE_U8)
    target_sources(CTLib PRIVATE
        "${CT_LIB_INCLUDE_DIR}/CTLib/U8.hpp"
        U8/U8.cpp
        U8/U8Arc.cpp
        U8/Read.cpp
        U8/Write.cpp
    )
endif()

# BRRES module
if(CT_LIB_MODULE_BRRES)
    target_sources(CTLib PRIVATE
        "${CT_LIB_INCLUDE_DIR}/CTLib/BRRES.hpp"
        BRRES/BRRES.cpp
        BRRES/MDL0.cpp
        BRRES/RW/BRRESRWCommon.cpp
        BRRES/RW/BRRESRWCommon.hpp
        BRRES/RW/Read.cpp
        BRRES/RW/Write.cpp
        BRRES/RW/WriteMDL0.cpp
        BRRES/RW/WriteTEX0.cpp
        BRRES/TEX0.cpp
    )
endif()

# KMP module
if(CT_LIB_MODULE_KMP)
    target_sources(CTLib PRIVATE
        "${CT_LIB_INCLUDE_DIR}/CTLib/KMP.hpp"
        KMP/KMP.cpp
    )
endif()

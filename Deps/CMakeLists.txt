########################################
# STB libraries
########################################

add_library(Deps_STB STATIC
    STB/Include/stb_dxt.h
    STB/Include/stb_image.h
    STB/Include/stb_image_resize.h
    STB/Include/stb_image_write.h
    STB/STBImpl.c
)

target_include_directories(Deps_STB PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/STB/Include")


########################################
# Constants
########################################

# Libraries
set(CT_LIB_DEPS Deps_STB CACHE INTERNAL "")

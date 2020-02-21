########################################
# Fetch GoogleTest
########################################

include(FetchContent)
FetchContent_Declare(gtest_fetch
        GIT_REPOSITORY  "https://github.com/google/googletest.git"
        GIT_TAG         "release-1.10.0")

FetchContent_GetProperties(gtest_fetch)

if (NOT gtest_fetch_POPULATED)
    FetchContent_Populate(gtest_fetch)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory("${gtest_fetch_SOURCE_DIR}" "${gtest_fetch_BINARY_DIR}")
endif()


########################################
# Function to add tests
########################################

#function(ct_lib_add_test EXECNAME)
#    cmake_parse_arguments(CT_LIB_TEST
#            ""
#            ""
#            "SOURCES"
#            ${ARGN})
#    
#    add_executable(${EXECNAME} "${CT_LIB_TEST_SOURCES}")
#    target_include_directories(${EXECNAME} PRIVATE "${CT_LIB_INCLUDE_DIR}")
#    target_link_libraries(${EXECNAME} gtest_main CTLib)
#    add_test(NAME ${EXECNAME} COMMAND ${EXECNAME})
#endfunction()

function(ct_lib_add_test EXECNAME)
    cmake_parse_arguments(CT_LIB_TEST
            ""
            ""
            "SOURCES"
            ${ARGN})
    
    add_executable(${EXECNAME} "${CT_LIB_TEST_SOURCES}")
    target_include_directories(${EXECNAME} PRIVATE "${CT_LIB_INCLUDE_DIR}")
    target_link_libraries(${EXECNAME} gtest_main CTLib)
    gtest_discover_tests(${EXECNAME}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endfunction()


########################################
# Add tests
########################################

ct_lib_add_test(CTLibTest SOURCES ctlib.cpp)
ct_lib_add_test(BRRESTest SOURCES brres.cpp)

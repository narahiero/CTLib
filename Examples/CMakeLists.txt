########################################
# Function to add examples
########################################

function(ct_lib_add_example EXECNAME SOURCE)
    add_executable(${EXECNAME} ${SOURCE})
    target_include_directories(${EXECNAME} PRIVATE "${CT_LIB_INCLUDE_DIR}")
    target_link_libraries(${EXECNAME} CTLib)
endfunction()


########################################
# Add examples
########################################

ct_lib_add_example(SZSTool SZSTool.cpp)

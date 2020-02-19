########################################
# Set Doxygen settings
########################################

# Set project info
set(DOXYGEN_PROJECT_NAME "CT Lib")
set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")


########################################
# Add Doxygen docs target
########################################

doxygen_add_docs(Doxy "${CT_LIB_INCLUDE_DIR}")

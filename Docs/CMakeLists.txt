########################################
# Set Doxygen settings
########################################

# Set project info
set(DOXYGEN_PROJECT_NAME "CT Lib")
set(DOXYGEN_PROJECT_NUMBER "${PROJECT_VERSION}")

# Set customization files
set(DOXYGEN_LAYOUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/DoxygenLayout.xml")


########################################
# Add Doxygen docs target
########################################

doxygen_add_docs(Doxy "${CT_LIB_INCLUDE_DIR}" ALL)

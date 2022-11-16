cmake_minimum_required (VERSION 3.12)

project (hello)

# Initialize CMake library for KasperskyOS SDK.
include (platform)
initialize_platform (FORCE_STATIC)

# Add Doxygen documentation
include (platform/doxygen)
add_project_documentation_main_target ()

add_subdirectory (hello)
add_subdirectory (einit)

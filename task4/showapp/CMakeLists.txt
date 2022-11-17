project (showapp)

# Tools for using NK parser.
include (platform/nk)

# Set compile flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

add_executable (Showapp "src/showapp.c")
add_dependencies (Showapp mosquitto_showapp_edl_files)

#set_target_properties (Server PROPERTIES LINK_FLAGS "-no-pie -Ttext 0x00c00000")

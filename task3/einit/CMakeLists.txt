project (einit)

# Tools for working with KasperskyOS images.
include (platform/image)

# Set compiler flags.
project_header_default ("STANDARD_GNU_11:YES" "STRICT_WARNINGS:NO")

find_program (NTPD_ENTITY
              NAMES "ntpd" REQUIRED
              NO_CMAKE_PATH
              NO_CMAKE_ENVIRONMENT_PATH
              NO_SYSTEM_ENVIRONMENT_PATH)

# Define an ENTITIES variable with a list of programs.
set (ENTITIES
    Subscriber
    ${precompiled_vfsVfsNet}
    ${NTPD_ENTITY}
    ${rump_DHCPCD_ENTITY})

if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "aarch64")
     set (SECURITY_PSL_FILE "src/security.psl")
else ()
     message (FATAL_ERROR "${CMAKE_SYSTEM_PROCESSOR} architecture is not supported." )
endif ()

if (ROOTFS_SDCARD)
    set (ROOTFS_ENTITY "kl.VfsSdCardFs")
    set (FS_BACKEND ${precompiled_vfsVfsSdCardFs})
    list (APPEND ENTITIES ${precompiled_vfsVfsSdCardFs})
else()
    set (ROOTFS_ENTITY "kl.VfsRamFs")
    set (FS_BACKEND ${precompiled_vfsVfsRamFs})
    list (APPEND ENTITIES ${precompiled_vfsVfsRamFs})
endif()

set_target_properties(${FS_BACKEND} PROPERTIES
    EXTRA_ARGS "\
    - -l
    - devfs /dev devfs 0")

# Redefine the DHCPCD program connections.
set_target_properties (${rump_DHCPCD_ENTITY} PROPERTIES
    DEPENDS_ON_ENTITY ""
    EXTRA_CONNECTIONS "\
  - target: kl.VfsNet
    id: kl.VfsNet
  - target: ${ROOTFS_ENTITY}
    id: ${ROOTFS_ENTITY}"
    EXTRA_ENV "\
    VFS_FILESYSTEM_BACKEND: client{fs->net}:${ROOTFS_ENTITY}
    VFS_NETWORK_BACKEND: client:kl.VfsNet"
    EXTRA_ARGS "\
    - '-4'
    - -f
    - /etc/dhcpcd.conf")

# Add a target for making and copying the disk image to the example build
# directory.
add_custom_command (OUTPUT ramdisk0.img
                    COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${CMAKE_BINARY_DIR}/hdd
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_SOURCE_DIR}/resources/ramfs
                        ${CMAKE_BINARY_DIR}/hdd
                    COMMAND $ENV{SDK_PREFIX}/common/prepare_hdd_img.sh -d
                        ${CMAKE_BINARY_DIR}/hdd -img ramdisk0.img -f ext4 -s 128)
add_custom_target (ramdisk0.img DEPENDS ramdisk0.img)

# Add a target for making and copying the disk image to the example build
# directory.
add_custom_command (OUTPUT sdcard0.img
                    COMMAND ${CMAKE_COMMAND} -E make_directory
                        ${CMAKE_BINARY_DIR}/hdd
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${CMAKE_SOURCE_DIR}/resources/sdcard
                        ${CMAKE_BINARY_DIR}/hdd
                    COMMAND $ENV{SDK_PREFIX}/common/prepare_hdd_img.sh -d
                        ${CMAKE_BINARY_DIR}/hdd -img sdcard0.img -f fat32)
add_custom_target (sdcard0.img DEPENDS sdcard0.img)

set (QEMU_FLAGS "-sd sdcard0.img")
set (SECURITY_PSL_FILE "src/security.psl")
set (FILES ${CMAKE_BINARY_DIR}/einit/ramdisk0.img)

# KasperskyOS image for the target hardware platform.
build_kos_hw_image (kos-image
                    EINIT_ENTITY EinitHw
                    CONNECTIONS_CFG "src/init.yaml.in"
                    SECURITY_PSL ${SECURITY_PSL_FILE}
                    IMAGE_FILES  ${ENTITIES}
                                 ${FILES})
add_dependencies (kos-image ramdisk0.img)

# KasperskyOS image for QEMU with simulation targets (sim, gdbsim, gdb).
build_kos_qemu_image (kos-qemu-image
                      EINIT_ENTITY EinitQemu
                      CONNECTIONS_CFG "src/init.yaml.in"
                      SECURITY_PSL ${SECURITY_PSL_FILE}
                      QEMU_FLAGS   ${QEMU_FLAGS}
                      IMAGE_FILES  ${ENTITIES}
                                   ${CMAKE_BINARY_DIR}/einit/sdcard0.img
                                   ${FILES})
add_dependencies (kos-qemu-image ramdisk0.img sdcard0.img)

set_target_properties (EinitHw PROPERTIES LINK_FLAGS ${VULN_LDFLAGS})
set_target_properties (EinitQemu PROPERTIES LINK_FLAGS ${VULN_LDFLAGS})

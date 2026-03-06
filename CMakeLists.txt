cmake_minimum_required(VERSION 3.22)
project(video C)

option(BUILD_TEST "build test" ON)
option(BUILD_MAIN "build main" ON)

if(BUILD_TEST OR BUILD_MAIN)
    # set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    # find lib
    find_library(ALSA_LIB
        NAMES libasound.so.2
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )
    find_library(XCB_LIB
        NAMES libxcb.so
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )
    find_library(XCB_DRI2_LIB
        NAMES libxcb-dri2.so.0
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )
    find_library(WAYLAND_CLIENT_LIB
        NAMES libwayland-client.so
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )
    find_library(X11_LIB
        NAMES libX11.so
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )
    find_library(DRM_LIB
        NAMES libdrm.so
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )

    find_library(MALI_LIB
        NAMES libmali.so.1
        PATHS ${SYSROOT_PATH}/usr/lib/aarch64-linux-gnu
        REQUIRED
    )

    find_library(ROCKIT_LIB
        NAMES librockit.so
        REQUIRED
    )
    find_library(ROCKCHIP_MPP_LIB
        NAMES librockchip_mpp.so
        REQUIRED
    )
    find_library(RGA_LIB
        NAMES librga.so
        PATHS ${SYSROOT_PATH}/usr/local/lib/aarch64-linux-gnu
        REQUIRED
    )
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/include
)

if(BUILD_TEST)
    # build hardware venc test
    add_executable(
        hw_venc_test
        "tests/hw_venc_test.c"
    )
    # link
    target_link_libraries(hw_venc_test PRIVATE
        ${ROCKIT_LIB}
        ${ROCKCHIP_MPP_LIB}
        ${RGA_LIB}
        ${MALI_LIB}
        ${ALSA_LIB}
        ${XCB_LIB}
        ${XCB_DRI2_LIB}
        ${WAYLAND_CLIENT_LIB}
        ${X11_LIB}
        ${DRM_LIB}
    )

    # build hardware video test
    add_executable(
        hw_video_test
        "tests/hw_video_test.c"
    )

    # build unit args test
    add_executable(
        unit_args_test
        "tests/unit_args_test.c"
        "src/args.c"
    )
    target_include_directories(unit_args_test PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )

    # build unit utils test
    add_executable(
        unit_utils_test
        "tests/unit_utils_test.c"
        "src/utils.c"
    )
    target_include_directories(unit_utils_test PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )

    # build unit socket test
    add_executable(
        unit_socket_test
        "tests/unit_socket_test.c"
        "src/socket.c"
        "src/utils.c"
    )
    target_include_directories(unit_socket_test PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )

    # build unit video test
    add_executable(
        unit_video_test
        "tests/unit_video_test.c"
        "src/utils.c"
        "src/video.c"
    )
    target_include_directories(unit_video_test PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )
    # link
    target_link_libraries(unit_video_test PRIVATE
        ${ROCKIT_LIB}
        ${ROCKCHIP_MPP_LIB}
        ${RGA_LIB}
        ${MALI_LIB}
        ${ALSA_LIB}
        ${XCB_LIB}
        ${XCB_DRI2_LIB}
        ${WAYLAND_CLIENT_LIB}
        ${X11_LIB}
        ${DRM_LIB}
        Threads::Threads
    )
endif()

if(BUILD_MAIN)
    # read version
    file(READ ${CMAKE_SOURCE_DIR}/version VERSION_STRING)
    string(STRIP ${VERSION_STRING} VERSION_STRING)

    # read git hash
    execute_process(
        COMMAND git config --global --add safe.directory ${CMAKE_SOURCE_DIR}
    )
    execute_process(
        COMMAND git rev-parse --short HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT COMMIT_HASH)
        set(COMMIT_HASH "unknown")
    endif()

    # get time
    string(TIMESTAMP BUILD_TIMESTAMP "%Y-%m-%dT%H:%M:%SZ" UTC)

    # create exec
    add_executable(
        ${PROJECT_NAME}
        "src/args.c"
        "src/main.c"
        "src/socket.c"
        "src/utils.c"
        "src/video.c"
    )
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${PROJECT_SOURCE_DIR}/src
    )
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        VERSION="${VERSION_STRING}"
        COMMIT="${COMMIT_HASH}"
        BUILD_TIME="${BUILD_TIMESTAMP}"
    )
    # link
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${ROCKIT_LIB}
        ${ROCKCHIP_MPP_LIB}
        ${RGA_LIB}
        ${MALI_LIB}
        ${ALSA_LIB}
        ${XCB_LIB}
        ${XCB_DRI2_LIB}
        ${WAYLAND_CLIENT_LIB}
        ${X11_LIB}
        ${DRM_LIB}
        Threads::Threads
    )
endif()

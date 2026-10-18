cmake_minimum_required(VERSION 3.10)

# Borealis lives in extern/ as a git submodule (xfangfang/borealis, wiliwili branch).
set(BOREALIS_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/extern/borealis/library)

# On the Switch, draw with deko3d (devkitPro's own GPU API) rather than mesa's
# OpenGL: mesa (libEGL + libglapi) was ~5 MB of the 9.4 MB .nro and most of
# the start-up time. -DUSE_DEKO3D=OFF (make GL=1) builds the OpenGL variant.
# Declared before borealis' options so this default wins; ignored on desktop.
option(USE_DEKO3D "Switch: draw with deko3d instead of OpenGL (mesa)" ON)

# The web simulator (web/README.md): the desktop build, compiled by
# Emscripten for a browser, with SDL2 and OpenGL ES 3 (WebGL 2) from
# Emscripten's own ports instead of GLFW and the system's OpenGL. Its
# toolchain file (emcmake) is only read by project(), below.
set(PLAYGUARD_WEB OFF)
if (CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten\\.cmake$")
    set(PLAYGUARD_WEB ON)
endif ()
if (PLAYGUARD_WEB)
    set(PLATFORM_DESKTOP ON CACHE BOOL "" FORCE)
    set(USE_SDL2 ON CACHE BOOL "" FORCE)
    set(USE_SYSTEM_SDL2 ON CACHE BOOL "" FORCE)
    set(USE_GLES3 ON CACHE BOOL "" FORCE)
    set(SDL2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/web/cmake)
    add_compile_options(-pthread -sUSE_SDL=2 -sUSE_ZLIB=1 -fexceptions)
endif ()

# PLATFORM_SWITCH / PLATFORM_DESKTOP / USE_DEKO3D … options.
include(${BOREALIS_LIBRARY}/cmake/commonOption.cmake)

# Sets up the devkitPro cross-toolchain when PLATFORM_SWITCH=ON.
include(${BOREALIS_LIBRARY}/cmake/toolchain.cmake)

project(playguard LANGUAGES CXX C)

# borealis' toolchain.cmake sets CMAKE_C_FLAGS / CMAKE_CXX_FLAGS as normal
# variables (include paths), which hides the cache values devkitPro's
# NintendoSwitch platform file initialises: the Cortex-A57 tuning, the crc and
# crypto extensions and the thread-local model libnx is built with. Put them
# back for every target (borealis, yoga, fmt, tinyxml2 and the app).
if (PLATFORM_SWITCH AND NX_ARCH_SETTINGS)
    string(PREPEND CMAKE_C_FLAGS "${NX_ARCH_SETTINGS} ")
    string(PREPEND CMAKE_CXX_FLAGS "${NX_ARCH_SETTINGS} ")
endif ()
# devkitPro's platform file defaults to Release (-O2 -g; elf2nro keeps no
# debug data). borealis prints its "Build Type:" line before project() picks
# that default, so say what is actually used.
# deko3d draws with precompiled shaders: borealis' nanovg loads them from
# romfs:/shaders/. uam (part of switch-dev) does not always give the same
# bytes for the same GLSL (one scheduling word of fill_vsh differs from run to
# run), which broke the reproducible zips, so the compiled shaders are kept in
# shaders/ and copied as they are. shaders/glsl.sha256 is the GLSL they were
# compiled from: after a borealis bump changes it, configure with
# -DPLAYGUARD_UAM=ON, copy build/dksh/*.dksh to shaders/ and update the checksums.
option(PLAYGUARD_UAM "Switch: compile the deko3d shaders with uam instead of using shaders/" OFF)
set(PLAYGUARD_DKSH_DIR ${CMAKE_BINARY_DIR}/dksh)
set(PLAYGUARD_SHADERS fill_vsh fill_fsh fill_aa_fsh)
if (PLATFORM_SWITCH AND USE_DEKO3D)
    if (PLAYGUARD_UAM)
        gen_dksh(${PLAYGUARD_DKSH_DIR})
        foreach (shader ${PLAYGUARD_SHADERS})
            if (NOT EXISTS ${PLAYGUARD_DKSH_DIR}/${shader}.dksh)
                message(FATAL_ERROR "uam did not produce ${shader}.dksh")
            endif ()
        endforeach ()
    else ()
        file(STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/glsl.sha256 PLAYGUARD_GLSL_SUMS)
        foreach (line ${PLAYGUARD_GLSL_SUMS})
            string(REGEX MATCH "^([0-9a-f]+)  (.+)$" _ "${line}")
            file(SHA256 ${BOREALIS_LIBRARY}/lib/extern/nanovg/deko3d/shaders/${CMAKE_MATCH_2} PLAYGUARD_GLSL_SUM)
            if (NOT PLAYGUARD_GLSL_SUM STREQUAL CMAKE_MATCH_1)
                message(FATAL_ERROR "borealis' ${CMAKE_MATCH_2} changed since shaders/ was compiled: "
                        "configure with -DPLAYGUARD_UAM=ON, copy build/dksh/*.dksh to shaders/ "
                        "and update shaders/glsl.sha256")
            endif ()
        endforeach ()
        file(REMOVE_RECURSE ${PLAYGUARD_DKSH_DIR})
        foreach (shader ${PLAYGUARD_SHADERS})
            file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${shader}.dksh DESTINATION ${PLAYGUARD_DKSH_DIR})
        endforeach ()
    endif ()
endif ()
if (PLATFORM_SWITCH)
    if (USE_DEKO3D)
        message(STATUS "PlayGuard renderer: deko3d")
    else ()
        message(STATUS "PlayGuard renderer: OpenGL (mesa)")
    endif ()
endif ()
string(TOUPPER "${CMAKE_BUILD_TYPE}" PLAYGUARD_BUILD_TYPE_UC)
message(STATUS "PlayGuard build type: ${CMAKE_BUILD_TYPE}; C++ flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${PLAYGUARD_BUILD_TYPE_UC}}")

# App metadata. The NACP version must match the release tag (CI checks it).
# release-please bumps these three lines (see release-please-config.json).
set(VERSION_MAJOR "1") # x-release-please-major
set(VERSION_MINOR "2") # x-release-please-minor
set(VERSION_ALTER "0") # x-release-please-patch
set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}")
set(PROJECT_TITLEID "")
set(PROJECT_AUTHOR "JigSawFr")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

# On desktop borealis looks for "<BRLS_RESOURCES_DIR>/resources/" at runtime:
# point it at the merged copy assembled below.
if (PLAYGUARD_WEB)
    # Packaged into the page's file system at /app/resources (web/CMakeLists).
    set(BRLS_RESOURCES_DIR /app)
elseif (PLATFORM_DESKTOP)
    set(BRLS_RESOURCES_DIR ${CMAKE_BINARY_DIR})
endif ()

# PlayGuard's fixes to borealis, until upstream has them (patches/borealis/,
# its README says why each one): applied to the submodule's files at
# configure time, in order. One already in place is left as it is, so
# configuring again (or a second build, as CI's reproducibility check does)
# changes nothing; one that no longer applies (after a borealis bump) stops
# the configure. safe.directory: CI's container checks out as another user.
file(GLOB PLAYGUARD_BOREALIS_PATCHES ${CMAKE_CURRENT_SOURCE_DIR}/patches/borealis/*.patch)
list(SORT PLAYGUARD_BOREALIS_PATCHES)
find_package(Git REQUIRED)
foreach (patch ${PLAYGUARD_BOREALIS_PATCHES})
    get_filename_component(patch_name ${patch} NAME)
    execute_process(COMMAND ${GIT_EXECUTABLE} -c safe.directory=* apply --reverse --check ${patch}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/borealis
                    RESULT_VARIABLE patch_missing OUTPUT_QUIET ERROR_QUIET)
    if (patch_missing)
        execute_process(COMMAND ${GIT_EXECUTABLE} -c safe.directory=* apply ${patch}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern/borealis
                        RESULT_VARIABLE patch_failed)
        if (patch_failed)
            message(FATAL_ERROR "patches/borealis/${patch_name} does not apply to extern/borealis: "
                                "update it for the new borealis, or drop it if upstream has the fix")
        endif ()
        message(STATUS "borealis: ${patch_name} applied")
    endif ()
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${patch})
endforeach ()

# borealis links stdc++fs under clang, which Emscripten's libc++ has no
# need of: an empty stand-in.
if (PLAYGUARD_WEB)
    add_library(stdc++fs INTERFACE)
endif ()

add_subdirectory(${BOREALIS_LIBRARY} ${CMAKE_BINARY_DIR}/borealis_library)

find_package(Threads REQUIRED)
list(APPEND APP_PLATFORM_LIB ${CMAKE_THREAD_LIBS_INIT})

# libcurl for the update check (util/http.cpp). On the Switch: devkitPro's
# switch-curl portlib (TLS through the console's ssl service), static, so its
# own dependencies are listed after it.
if (PLATFORM_SWITCH)
    list(APPEND APP_PLATFORM_LIB curl z nx)
elseif (PLAYGUARD_WEB)
    # No libcurl in a browser: source/sim/web_http.cpp answers offline.
else ()
    find_package(CURL REQUIRED)
    find_package(ZLIB REQUIRED)   # util/zip_read.cpp (the Switch links z above)
    list(APPEND APP_PLATFORM_LIB CURL::libcurl ZLIB::ZLIB)
endif ()

# Sources: UI (C++) + service layer. The real service layer (source/core/*.c)
# talks to libnx and is Switch-only; the desktop build swaps in the simulated
# backend from source/sim/ so the UI can be run and screenshotted on a PC.
file(GLOB_RECURSE PLAYGUARD_SRC CONFIGURE_DEPENDS source/*.cpp source/*.c)
if (PLATFORM_SWITCH)
    list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/sim/.*")
    list(APPEND PLAYGUARD_SRC ${BOREALIS_LIBRARY}/lib/platforms/switch/switch_wrapper.c)
else ()
    list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/core/.*\\.c$")
    if (PLAYGUARD_WEB)
        list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/util/http\\.cpp$")
    else ()
        list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/sim/web_.*")
    endif ()
    # The console-free parts are shared with the simulator: one implementation.
    list(APPEND PLAYGUARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/core/pure.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/calendar.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/gamecheck_rules.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/rescue.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/write_guard.c)
endif ()

# QR codes of the funding links in About (MIT, vendored: extern/qrcodegen).
list(APPEND PLAYGUARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/extern/qrcodegen/qrcodegen.cpp)

program_target(${PROJECT_NAME} "${PLAYGUARD_SRC}")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

target_include_directories(${PROJECT_NAME} PRIVATE source extern/qrcodegen ${APP_PLATFORM_INCLUDE})
# -ffunction-sections / -fdata-sections come from borealis (PUBLIC) and
# --gc-sections from libnx's switch.specs.
# Warnings on PlayGuard's own sources (not borealis, not the vendored
# qrcodegen); CI turns them into errors with -DPLAYGUARD_WERROR=ON.
option(PLAYGUARD_WERROR "Treat PlayGuard's compiler warnings as errors" OFF)
set(PLAYGUARD_WARNINGS -Wall -Wextra)
if (PLAYGUARD_WERROR)
    list(APPEND PLAYGUARD_WARNINGS -Werror)
endif ()
target_compile_options(${PROJECT_NAME} PRIVATE ${APP_PLATFORM_OPTION} ${PLAYGUARD_WARNINGS})
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/extern/qrcodegen/qrcodegen.cpp PROPERTIES COMPILE_OPTIONS "-w")
# The commit built (at configure time): development builds are told apart by
# it (util/dev_builds.hpp). PLAYGUARD_COMMIT (CI: a pull request's head)
# wins over the checkout's. Empty outside a git checkout.
find_package(Git QUIET)
set(APP_COMMIT "")
if (DEFINED ENV{PLAYGUARD_COMMIT} AND NOT "$ENV{PLAYGUARD_COMMIT}" STREQUAL "")
    string(SUBSTRING "$ENV{PLAYGUARD_COMMIT}" 0 7 APP_COMMIT)
elseif (GIT_FOUND)
    execute_process(COMMAND ${GIT_EXECUTABLE} rev-parse --short=7 HEAD
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    OUTPUT_VARIABLE APP_COMMIT OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)
endif()
message(STATUS "PlayGuard commit: ${APP_COMMIT}")
# A published release (CI: the build release-please.yml calls for a vX.Y.Z tag,
# PLAYGUARD_RELEASE=1). Anything else is a development build: the version
# badge in the header (view/version_badge.hpp) says so, with the commit.
set(APP_RELEASE 0)
if ("$ENV{PLAYGUARD_RELEASE}" STREQUAL "1")
    set(APP_RELEASE 1)
endif ()
message(STATUS "PlayGuard release build: ${APP_RELEASE}")
# The recovery sysmodule this version ships with (tools/rescue_rev.cmake).
include(${CMAKE_CURRENT_SOURCE_DIR}/tools/rescue_rev.cmake)
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${RESCUE_REV_SOURCES})
message(STATUS "PlayGuard recovery sysmodule revision: ${RESCUE_REV}")
target_compile_definitions(${PROJECT_NAME} PRIVATE APP_VERSION="${APP_VERSION}" APP_COMMIT="${APP_COMMIT}"
                           APP_RELEASE=${APP_RELEASE} APP_RESCUE_REV="${RESCUE_REV}")
target_link_libraries(${PROJECT_NAME} PRIVATE borealis ${APP_PLATFORM_LIB})
# borealis' headers as system headers: their warnings are not PlayGuard's.
get_target_property(BOREALIS_INCLUDES borealis INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${BOREALIS_INCLUDES})
# On the Switch: a linker map (build/borealis.map) to see what makes the .nro big.
target_link_options(${PROJECT_NAME} PRIVATE ${APP_PLATFORM_LINK_OPTION})
# The web simulator: WebGL 2, threads from a pool started with the page (a
# thread asked for later waits for the browser, which a busy main thread never
# lets it have), the resources packaged at /app/resources, and ENV and FS for
# the page to set the PLAYGUARD_SIM_* variables and write config.json before
# main() (web/simulator.js).
if (PLAYGUARD_WEB)
    target_link_options(${PROJECT_NAME} PRIVATE
        -pthread -fexceptions -sUSE_SDL=2 -sUSE_ZLIB=1
        -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3
        -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=128MB -sSTACK_SIZE=1MB -sDEFAULT_PTHREAD_STACK_SIZE=1MB
        -sPTHREAD_POOL_SIZE=16
        -sEXIT_RUNTIME=0
        -sEXPORTED_RUNTIME_METHODS=ENV,FS
        --preload-file ${CMAKE_BINARY_DIR}/resources@/app/resources)
endif ()
# On the Switch: borealis' userAppInit, which libnx calls before main, timed
# for the diagnostic report's start-up section (source/core/startup_hook.c).
if (PLATFORM_SWITCH)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--wrap=userAppInit)
endif ()

# Resources: borealis built-ins first, then ours on top. On the Switch the
# bundled fonts and demo assets are dropped (the system shared font is used).
# en-GB reuses the English strings, already in British spelling (borealis looks
# the locale up verbatim), except the quoted system menu labels, spelt as on a
# US console ("Synchronize Clock via Internet").
set(BOREALIS_RESOURCES ${BOREALIS_LIBRARY}/../resources)
set(MERGED_RESOURCES ${CMAKE_BINARY_DIR}/resources)
set(ASSEMBLE_RESOURCES
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${BOREALIS_RESOURCES} ${MERGED_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}/xml
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}/img/pokemon
    COMMAND ${CMAKE_COMMAND} -E remove -f ${MERGED_RESOURCES}/img/tiles.png ${MERGED_RESOURCES}/img/demo_icon.jpg
            ${MERGED_RESOURCES}/img/borealis_96.png ${MERGED_RESOURCES}/img/borealis_256.png
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${MERGED_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES}/i18n/en-US ${MERGED_RESOURCES}/i18n/en-GB
    # About shows the latest releases from it (util/changelog.cpp).
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md ${MERGED_RESOURCES}/CHANGELOG.md
    # ... and the highlights of the running version from its summary
    # (view/release_notes.cpp), the full list a press away.
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/.github/release-notes ${MERGED_RESOURCES}/release-notes
    COMMAND ${CMAKE_COMMAND} -E remove -f ${MERGED_RESOURCES}/release-notes/README.md ${MERGED_RESOURCES}/img/emoji/README.md
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_ICON} ${MERGED_RESOURCES}/img/app_icon.jpg)

set(PLAYGUARD_COPY_SHADERS)
if (PLATFORM_SWITCH AND USE_DEKO3D)
    set(PLAYGUARD_COPY_SHADERS COMMAND ${CMAKE_COMMAND} -E copy_directory ${PLAYGUARD_DKSH_DIR} ${MERGED_RESOURCES}/shaders)
endif ()

if (PLATFORM_SWITCH)
    # The recovery sysmodule rides in the .nro (romfs:/rescue/exefs.nsp), so
    # PlayGuard can update an installed one that is out of date
    # (action/rescue.hpp). Its own Makefile (sysmodule/) builds it; it is
    # only copied to the SD card when a parent asks.
    find_program(PLAYGUARD_MAKE NAMES make gmake)
    set(RESCUE_NSP ${CMAKE_CURRENT_SOURCE_DIR}/sysmodule/out/playguard-rescue.nsp)
    add_custom_target(playguard_rescue
        COMMAND ${PLAYGUARD_MAKE} --no-print-directory -C ${CMAKE_CURRENT_SOURCE_DIR}/sysmodule
        BYPRODUCTS ${RESCUE_NSP}
        COMMENT "Building the recovery sysmodule")
    add_custom_target(${PROJECT_NAME}.nro ALL DEPENDS ${PROJECT_NAME} playguard_rescue
        COMMAND ${NX_NACPTOOL_EXE} --create "PlayGuard" "${PROJECT_AUTHOR}"
                "${APP_VERSION}" ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
        ${ASSEMBLE_RESOURCES}
        ${PLAYGUARD_COPY_SHADERS}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MERGED_RESOURCES}/rescue
        COMMAND ${CMAKE_COMMAND} -E copy ${RESCUE_NSP} ${MERGED_RESOURCES}/rescue/exefs.nsp
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}/font
        COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro
                --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp
                --romfsdir=${MERGED_RESOURCES}
    )
else ()
    add_custom_target(${PROJECT_NAME}_resources ALL ${ASSEMBLE_RESOURCES})
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_resources)
endif ()

# The web simulator's site, ready to serve (web/README.md): the page from
# web/, PlayGuard and its packaged resources, in <build>/site/.
if (PLAYGUARD_WEB)
    set(PLAYGUARD_SITE ${CMAKE_BINARY_DIR}/site)
    add_custom_target(${PROJECT_NAME}_site ALL DEPENDS ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/web/index.html ${CMAKE_CURRENT_SOURCE_DIR}/web/simulator.js
                ${CMAKE_CURRENT_SOURCE_DIR}/web/simulator.css ${CMAKE_CURRENT_SOURCE_DIR}/web/params.json
                ${CMAKE_CURRENT_SOURCE_DIR}/web/coi-serviceworker.js ${PROJECT_ICON}
                ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm
                ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.data
                ${PLAYGUARD_SITE}
        COMMENT "Assembling the web simulator's site in ${PLAYGUARD_SITE}")
endif ()

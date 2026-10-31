cmake_minimum_required(VERSION 3.10)

# Aurore (JigSawFr/Aurore, main) lives in library/aurore as a git submodule:
# xfangfang/borealis renamed, with our fixes and some from its other forks.
set(AURORE_LIBRARY ${CMAKE_CURRENT_SOURCE_DIR}/library/aurore/library)

# On the Switch, draw with deko3d (devkitPro's own GPU API) rather than mesa's
# OpenGL: mesa (libEGL + libglapi) was ~5 MB of the 9.4 MB .nro and most of
# the start-up time. -DAURORE_GRAPHICS=GL3 (make GL=1) builds the OpenGL
# variant. Set before Aurore's options so this default wins.
if (AURORE_PLATFORM STREQUAL "SWITCH" AND NOT AURORE_GRAPHICS)
    set(AURORE_GRAPHICS DEKO3D CACHE STRING "" FORCE)
endif ()

# The web simulator (web/README.md): the desktop build, compiled by
# Emscripten for a browser, with SDL3 and OpenGL ES 3 (WebGL 2) from
# Emscripten's own ports instead of the system's. Its toolchain file
# (emcmake) is only read by project(), below.
set(PLAYGUARD_WEB OFF)
if (CMAKE_TOOLCHAIN_FILE MATCHES "Emscripten\\.cmake$")
    set(PLAYGUARD_WEB ON)
endif ()
if (PLAYGUARD_WEB)
    set(AURORE_PLATFORM DESKTOP CACHE STRING "" FORCE)
    set(AURORE_WINDOW SDL3 CACHE STRING "" FORCE)
    set(USE_SYSTEM_SDL3 ON CACHE BOOL "" FORCE)
    set(AURORE_GRAPHICS GLES3 CACHE STRING "" FORCE)
    set(SDL3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/web/cmake)
    add_compile_options(-pthread -sUSE_SDL=3 -sUSE_ZLIB=1 -fexceptions)
endif ()

# The desktop build is the simulator, built in Release: Aurore's debugging
# aids still read from the environment there (AURORE_INSPECTOR,
# AURORE_HOT_RELOAD…; docs/environment.md in Aurore). -DAURORE_DEV_TOOLS=AUTO
# leaves them out of a release build again.
# Its window is SDL3's, Aurore's default on the desktop
# (JigSawFr/Aurore#92); -DAURORE_WINDOW=GLFW builds the GLFW one.
if (AURORE_PLATFORM STREQUAL "DESKTOP" AND NOT PLAYGUARD_WEB)
    set(AURORE_DEV_TOOLS ON CACHE STRING "")
endif ()

# AURORE_PLATFORM / AURORE_GRAPHICS … options, and the PLATFORM_* and USE_*
# variables they set.
include(${AURORE_LIBRARY}/cmake/commonOption.cmake)

# Sets up the devkitPro cross-toolchain when AURORE_PLATFORM=SWITCH.
include(${AURORE_LIBRARY}/cmake/toolchain.cmake)

project(playguard LANGUAGES CXX C)

if (PLATFORM_SWITCH)
    if (USE_DEKO3D)
        message(STATUS "PlayGuard renderer: deko3d")
    else ()
        message(STATUS "PlayGuard renderer: OpenGL (mesa)")
    endif ()
endif ()
# devkitPro's platform file defaults to Release (-O2 -g; elf2nro keeps no
# debug data). Aurore prints its "Build Type:" line before project() picks
# that default, so say what is actually used.
string(TOUPPER "${CMAKE_BUILD_TYPE}" PLAYGUARD_BUILD_TYPE_UC)
message(STATUS "PlayGuard build type: ${CMAKE_BUILD_TYPE}; C++ flags: ${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_${PLAYGUARD_BUILD_TYPE_UC}}")

# App metadata. The NACP version must match the release tag (CI checks it).
# release-please bumps these three lines (see release-please-config.json).
set(VERSION_MAJOR "1") # x-release-please-major
set(VERSION_MINOR "4") # x-release-please-minor
set(VERSION_ALTER "0") # x-release-please-patch
set(APP_VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_ALTER}")
set(PROJECT_TITLEID "")
set(PROJECT_AUTHOR "JigSawFr")
set(PROJECT_ICON ${CMAKE_CURRENT_SOURCE_DIR}/icon.jpg)
set(PROJECT_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/resources)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_EXTENSIONS ON)

# On desktop Aurore looks for "<AURORE_RESOURCES_ROOT>/resources/" at runtime:
# point it at the merged copy assembled below.
if (PLAYGUARD_WEB)
    # Packaged into the page's file system at /app/resources (the web target's
    # --preload-file, below).
    set(AURORE_RESOURCES_ROOT /app)
elseif (PLATFORM_DESKTOP)
    set(AURORE_RESOURCES_ROOT ${CMAKE_BINARY_DIR})
endif ()

# Aurore links stdc++fs under clang, which Emscripten's libc++ has no
# need of: an empty stand-in.
if (PLAYGUARD_WEB)
    add_library(stdc++fs INTERFACE)
endif ()

add_subdirectory(${AURORE_LIBRARY} ${CMAKE_BINARY_DIR}/aurore)

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
    list(APPEND PLAYGUARD_SRC ${AURORE_LIBRARY}/lib/platforms/switch/switch_wrapper.c)
else ()
    list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/core/.*\\.c$")
    if (PLAYGUARD_WEB)
        list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/util/http\\.cpp$")
    else ()
        list(FILTER PLAYGUARD_SRC EXCLUDE REGEX ".*/source/sim/web_.*")
    endif ()
    # The console-free parts are shared with the simulator: one implementation
    # (tools/check_sources.py fails on one left out).
    list(APPEND PLAYGUARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/source/core/pure.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/calendar.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/console_info_pure.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/gamecheck_rules.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/forwarder_pack.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/gate.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/hbl_lock.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/rescue.c
                              ${CMAKE_CURRENT_SOURCE_DIR}/source/core/write_guard.c
                              # The simulated agent runs the agent's own command dispatch.
                              ${CMAKE_CURRENT_SOURCE_DIR}/sysmodule/agent/source/agent_core.c)
endif ()

# The remote link's transport, shared by the Boréale homebrews
# (JigSawFr/boreale-link, a submodule in library/boreale-link).
file(GLOB BOREALE_LINK_SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/library/boreale-link/source/*.c)
list(APPEND PLAYGUARD_SRC ${BOREALE_LINK_SRC})

# QR codes of the funding links in About (MIT, vendored: extern/qrcodegen).
list(APPEND PLAYGUARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/extern/qrcodegen/qrcodegen.cpp)
# JPEG writing for the NRO forwarders' icons (public domain, vendored: extern/stb).
list(APPEND PLAYGUARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/extern/stb/stb_image_write.c)
# Fonts and JPEGs for the PIN gate's screen (core/gate_screen.c), on the console.
if (PLATFORM_SWITCH)
    list(APPEND PLAYGUARD_SRC ${CMAKE_CURRENT_SOURCE_DIR}/extern/stb/pg_stb.c)
endif ()

program_target(${PROJECT_NAME} "${PLAYGUARD_SRC}")
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 23)

target_include_directories(${PROJECT_NAME} PRIVATE source library/boreale-link/source extern/qrcodegen extern/stb)
if (NOT PLATFORM_SWITCH)
    target_include_directories(${PROJECT_NAME} PRIVATE source/sync sysmodule/agent/source)
endif ()
# -ffunction-sections / -fdata-sections come from Aurore (PUBLIC) and
# --gc-sections from libnx's switch.specs.
# Warnings on PlayGuard's own sources (not Aurore, not the vendored
# qrcodegen and stb); CI turns them into errors with -DPLAYGUARD_WERROR=ON.
option(PLAYGUARD_WERROR "Treat PlayGuard's compiler warnings as errors" OFF)
# Also a switch case falling into the next unmarked, a class with virtual
# functions whose destructor is not virtual (C++ only), and a name that hides
# another one (a local, a parameter, a lambda's or a member).
set(PLAYGUARD_WARNINGS -Wall -Wextra -Wimplicit-fallthrough -Wshadow $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>)
if (PLAYGUARD_WERROR)
    list(APPEND PLAYGUARD_WARNINGS -Werror)
endif ()
target_compile_options(${PROJECT_NAME} PRIVATE ${PLAYGUARD_WARNINGS})
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/extern/qrcodegen/qrcodegen.cpp PROPERTIES COMPILE_OPTIONS "-w")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/extern/stb/stb_image_write.c PROPERTIES COMPILE_OPTIONS "-w")
set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/extern/stb/pg_stb.c PROPERTIES COMPILE_OPTIONS "-w")
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
# The pull request built (CI: PLAYGUARD_PR, its number), 0 for any other build:
# the change history says which pull request's build made a change.
set(APP_PR 0)
if ("$ENV{PLAYGUARD_PR}" MATCHES "^[1-9][0-9]*$")
    set(APP_PR "$ENV{PLAYGUARD_PR}")
endif ()
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
# On the two files that read them only: on every other file's command line
# the commit (and the pull request) would change it at each build, and ccache
# would have to preprocess each file again before finding it already compiled
# (the desktop's precompiled header, below, would be built anew every time).
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/app.cpp APPEND PROPERTY COMPILE_DEFINITIONS
             APP_VERSION="${APP_VERSION}" APP_COMMIT="${APP_COMMIT}" APP_PR=${APP_PR} APP_RELEASE=${APP_RELEASE})
set_property(SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/source/action/rescue.cpp APPEND PROPERTY COMPILE_DEFINITIONS
             APP_RESCUE_REV="${RESCUE_REV}")
target_link_libraries(${PROJECT_NAME} PRIVATE Aurore::aurore ${APP_PLATFORM_LIB})
# On the desktop: Aurore and the standard headers most files include,
# parsed once (a precompiled header) instead of in each of the 200-odd C++
# files: a change to a header many include (ui/ui.hpp, app.hpp) rebuilds in
# about half the time. The Switch and the web keep compiling as before (their
# toolchains, and the release's reproducibility check, are left alone); a
# file that forgets an #include the header gives the desktop fails there.
# qrcodegen is not ours (-w).
if (PLATFORM_DESKTOP AND NOT PLAYGUARD_WEB AND NOT CMAKE_VERSION VERSION_LESS 3.16)
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        "$<$<COMPILE_LANGUAGE:CXX>:<aurore.hpp$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<format$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<algorithm$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<chrono$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<functional$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<memory$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<string$<ANGLE-R>>"
        "$<$<COMPILE_LANGUAGE:CXX>:<vector$<ANGLE-R>>")
    set_source_files_properties(${CMAKE_CURRENT_SOURCE_DIR}/extern/qrcodegen/qrcodegen.cpp
                                PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
endif ()
# Aurore's headers as system headers: their warnings are not PlayGuard's.
get_target_property(AURORE_INCLUDES Aurore::aurore INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${AURORE_INCLUDES})
# On the Switch: a linker map (build/playguard.map) to see what makes the .nro big.
if (PLATFORM_SWITCH)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,-Map,playguard.map)
endif ()
# The web simulator: WebGL 2, threads from a pool started with the page (a
# thread asked for later waits for the browser, which a busy main thread never
# lets it have), the resources packaged at /app/resources and the answers of a
# simulated GitHub and switchbrew at /app/samples (web/samples/), ENV and FS
# for the page to set the PLAYGUARD_SIM_* variables and write config.json
# before main() (and run dependencies to make main() wait for the data folder
# or the CJK font), IDBFS to keep the data folder in the browser (exported:
# Emscripten 6.0.13 dropped FS.filesystems), and ccall
# for what the page calls while PlayGuard runs (source/sim/web_page.cpp,
# web/simulator.js).
if (PLAYGUARD_WEB)
    target_link_options(${PROJECT_NAME} PRIVATE
        -pthread -fexceptions -sUSE_SDL=3 -sUSE_ZLIB=1
        -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sFULL_ES3
        # Not -sGROWABLE_ARRAYBUFFERS, which Emscripten suggests for threads
        # with memory growth: WebGL refuses views of a resizable buffer
        # ("The provided ArrayBuffer value must not be resizable", Chrome).
        -sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=128MB -sSTACK_SIZE=1MB -sDEFAULT_PTHREAD_STACK_SIZE=1MB
        -sPTHREAD_POOL_SIZE=16
        -sEXIT_RUNTIME=0
        -sEXPORTED_RUNTIME_METHODS=ENV,FS,IDBFS,ccall,addRunDependency,removeRunDependency
        -lidbfs.js
        # SHELL: keeps each pair together (CMake drops a repeated option).
        "SHELL:--preload-file ${CMAKE_BINARY_DIR}/resources@/app/resources"
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/web/samples@/app/samples")
endif ()
# On the Switch: Aurore's userAppInit, which libnx calls before main, timed
# for the diagnostic report's start-up section (source/core/startup_hook.c).
if (PLATFORM_SWITCH)
    target_link_options(${PROJECT_NAME} PRIVATE -Wl,--wrap=userAppInit)
endif ()

# Resources: Aurore's built-ins first, then ours on top, without Aurore's demo
# assets. On the Switch Aurore's desktop fonts (Inter, PromptFont's buttons)
# are dropped too (the system shared font is used); ours stay (resources/font:
# Cadran's figures).
# en-GB reuses the English strings, already in British spelling (Aurore looks
# the locale up verbatim), except the quoted system menu labels, spelt as on a
# US console ("Synchronize Clock via Internet").
set(AURORE_RESOURCES ${AURORE_LIBRARY}/../resources)
set(MERGED_RESOURCES ${CMAKE_BINARY_DIR}/resources)
set(ASSEMBLE_RESOURCES
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${AURORE_RESOURCES} ${MERGED_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}/xml
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}/themes
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${MERGED_RESOURCES}/img/gallery
    COMMAND ${CMAKE_COMMAND} -E remove -f ${MERGED_RESOURCES}/img/tiles.png ${MERGED_RESOURCES}/img/demo_icon.jpg
            ${MERGED_RESOURCES}/img/aurore_96.png ${MERGED_RESOURCES}/img/aurore_256.png
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES} ${MERGED_RESOURCES}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_RESOURCES}/i18n/en-US ${MERGED_RESOURCES}/i18n/en-GB
    # About shows the latest releases from it (util/changelog.cpp).
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/CHANGELOG.md ${MERGED_RESOURCES}/CHANGELOG.md
    # ... and the highlights of the running version from its summary
    # (view/release_notes.cpp), the full list a press away.
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/.github/release-notes ${MERGED_RESOURCES}/release-notes
    COMMAND ${CMAKE_COMMAND} -E remove -f ${MERGED_RESOURCES}/release-notes/README.md ${MERGED_RESOURCES}/img/emoji/README.md
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_ICON} ${MERGED_RESOURCES}/img/app_icon.jpg
    # The optional sysmodules, when built (make rescue / make agent).
    COMMAND ${CMAKE_COMMAND} -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR} -DDEST=${MERGED_RESOURCES}/sysmodules
            -DAPP_VERSION=${APP_VERSION} -DAPP_COMMIT=${APP_COMMIT}
            -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/bundle_sysmodules.cmake)

# The web simulator: Japanese, Korean and Chinese drawn with the characters
# the catalogs use, cut out of Noto Sans CJK (tools/web_cjk_font.py), as the
# console draws them with its system fonts. It is served next to the page, not
# packaged: the page waits for it only in those languages, and fetches it
# after PlayGuard started in the others (web/README.md). Without the font or
# fontTools, the build goes on and those languages show boxes.
if (PLAYGUARD_WEB)
    set(PLAYGUARD_WEB_CJK_FONT "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc" CACHE FILEPATH
        "Web simulator: Noto Sans CJK (a .ttc) to take the CJK font from")
    find_package(Python3 COMPONENTS Interpreter)
    if (Python3_FOUND)
        execute_process(COMMAND ${Python3_EXECUTABLE} -c "import fontTools" RESULT_VARIABLE PLAYGUARD_NO_FONTTOOLS
                        OUTPUT_QUIET ERROR_QUIET)
    endif ()
    if (Python3_FOUND AND NOT PLAYGUARD_NO_FONTTOOLS AND EXISTS "${PLAYGUARD_WEB_CJK_FONT}")
        set(PLAYGUARD_CJK_CATALOGS)
        foreach (resources ${PROJECT_RESOURCES} ${AURORE_RESOURCES})
            file(GLOB catalogs ${resources}/i18n/ja/*.json ${resources}/i18n/ko/*.json
                 ${resources}/i18n/zh-Hans/*.json ${resources}/i18n/zh-Hant/*.json)
            list(APPEND PLAYGUARD_CJK_CATALOGS ${catalogs})
        endforeach ()
        add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/cjk.otf
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/tools/web_cjk_font.py
                    ${PLAYGUARD_WEB_CJK_FONT} ${CMAKE_BINARY_DIR}/cjk.otf ${CMAKE_CURRENT_SOURCE_DIR}
            DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/tools/web_cjk_font.py ${PLAYGUARD_CJK_CATALOGS}
            COMMENT "Cutting the web simulator's CJK font out of ${PLAYGUARD_WEB_CJK_FONT}")
        add_custom_target(${PROJECT_NAME}_cjk DEPENDS ${CMAKE_BINARY_DIR}/cjk.otf)
        set(PLAYGUARD_SITE_CJK COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/cjk.otf ${CMAKE_BINARY_DIR}/simulator/cjk.otf)
        message(STATUS "Web simulator CJK font: from ${PLAYGUARD_WEB_CJK_FONT}")
    else ()
        message(WARNING "Web simulator: no CJK font (needs Python 3 with fontTools and ${PLAYGUARD_WEB_CJK_FONT}): "
                        "Japanese, Korean and Chinese will show boxes")
    endif ()
endif ()

if (PLATFORM_SWITCH)
    # The program of the NRO forwarders PlayGuard installs (core/forwarder.h)
    # rides in the .nro (romfs:/forwarder/main and main.npdm), built by
    # forwarder/'s Makefile.
    find_program(PLAYGUARD_MAKE NAMES make gmake)
    set(FORWARDER_OUT ${CMAKE_CURRENT_SOURCE_DIR}/forwarder/out)
    add_custom_target(playguard_forwarder
        COMMAND ${PLAYGUARD_MAKE} --no-print-directory -C ${CMAKE_CURRENT_SOURCE_DIR}/forwarder
        BYPRODUCTS ${FORWARDER_OUT}/forwarder.nso ${FORWARDER_OUT}/forwarder.npdm
        COMMENT "Building the NRO forwarders' loader")
    # The gate a forwarder protected with the PIN starts first (core/gate.h):
    # romfs:/gate/gate.nro, written to config/playguard/ when one is made.
    set(GATE_NRO ${CMAKE_CURRENT_SOURCE_DIR}/gate/out/gate.nro)
    add_custom_target(playguard_gate
        COMMAND ${PLAYGUARD_MAKE} --no-print-directory -C ${CMAKE_CURRENT_SOURCE_DIR}/gate
        BYPRODUCTS ${GATE_NRO}
        COMMENT "Building the forwarders' PIN gate")
    add_custom_target(${PROJECT_NAME}.nro ALL DEPENDS ${PROJECT_NAME} playguard_forwarder playguard_gate
        COMMAND ${NX_NACPTOOL_EXE} --create "PlayGuard" "${PROJECT_AUTHOR}"
                "${APP_VERSION}" ${PROJECT_NAME}.nacp --titleid=${PROJECT_TITLEID}
        ${ASSEMBLE_RESOURCES}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MERGED_RESOURCES}/forwarder
        COMMAND ${CMAKE_COMMAND} -E copy ${FORWARDER_OUT}/forwarder.nso ${MERGED_RESOURCES}/forwarder/main
        COMMAND ${CMAKE_COMMAND} -E copy ${FORWARDER_OUT}/forwarder.npdm ${MERGED_RESOURCES}/forwarder/main.npdm
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MERGED_RESOURCES}/gate
        COMMAND ${CMAKE_COMMAND} -E copy ${GATE_NRO} ${MERGED_RESOURCES}/gate/gate.nro
        COMMAND ${CMAKE_COMMAND} -E remove -f ${MERGED_RESOURCES}/font/aurore_text.ttf ${MERGED_RESOURCES}/font/aurore_buttons.ttf
                ${MERGED_RESOURCES}/font/OFL-Inter.txt ${MERGED_RESOURCES}/font/OFL-PromptFont.txt
        COMMAND ${NX_ELF2NRO_EXE} ${PROJECT_NAME}.elf ${PROJECT_NAME}.nro
                --icon=${PROJECT_ICON} --nacp=${PROJECT_NAME}.nacp
                --romfsdir=${MERGED_RESOURCES}
    )
else ()
    add_custom_target(${PROJECT_NAME}_resources ALL ${ASSEMBLE_RESOURCES})
    add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_resources)
endif ()

# The web simulator's site, ready to serve (web/README.md): the page from
# web/, PlayGuard and its packaged resources, in <build>/simulator/; from
# boreale-site, the header's look and menus (boreale.css, boreale.js), their
# fonts (fonts/) and the sites' marks (marks/).
if (PLAYGUARD_WEB)
    set(PLAYGUARD_SITE ${CMAKE_BINARY_DIR}/simulator)
    add_custom_target(${PROJECT_NAME}_simulator ALL DEPENDS ${PROJECT_NAME}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/web/index.html ${CMAKE_CURRENT_SOURCE_DIR}/web/theme.js ${CMAKE_CURRENT_SOURCE_DIR}/web/sim_logic.js
                ${CMAKE_CURRENT_SOURCE_DIR}/web/simulator.js
                ${CMAKE_CURRENT_SOURCE_DIR}/web/simulator.css ${CMAKE_CURRENT_SOURCE_DIR}/web/params.json
                ${CMAKE_CURRENT_SOURCE_DIR}/web/sw.js ${CMAKE_CURRENT_SOURCE_DIR}/web/manifest.webmanifest ${PROJECT_ICON}
                ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.js ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.wasm
                ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.data
                ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/branding/icon.svg ${PLAYGUARD_SITE}/icon.svg
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/branding/mark.svg ${PLAYGUARD_SITE}/mark.svg
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/library/boreale-site/assets/marks ${PLAYGUARD_SITE}/marks
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/library/boreale-site/assets/fonts ${PLAYGUARD_SITE}/fonts
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/library/boreale-site/assets/boreale.css
                ${CMAKE_CURRENT_SOURCE_DIR}/library/boreale-site/assets/boreale.js ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/images/apple-touch-icon.png ${PLAYGUARD_SITE}/apple-touch-icon.png
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/images/icon-192.png ${CMAKE_CURRENT_SOURCE_DIR}/images/icon-512.png ${PLAYGUARD_SITE}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/images/splash ${PLAYGUARD_SITE}/splash
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/i18n ${PLAYGUARD_SITE}/i18n
        ${PLAYGUARD_SITE_CJK}
        COMMENT "Assembling the web simulator's site in ${PLAYGUARD_SITE}")
    if (TARGET ${PROJECT_NAME}_cjk)
        add_dependencies(${PROJECT_NAME}_simulator ${PROJECT_NAME}_cjk)
    endif ()
endif ()

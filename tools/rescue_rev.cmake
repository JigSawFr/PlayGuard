# The recovery sysmodule's revision: the first 12 hex digits of the SHA-256
# of what it is built from (its source, the code it shares with the app, its
# NPDM settings). The same sources give the same revision, whatever the
# PlayGuard version: the app compares it with the one written next to the
# installed module (playguard-rescue.txt, Makefile's dist-rescue) to tell a
# module out of date.
#
# Included by CMakeLists.txt (sets RESCUE_REV), or run as a script, which
# prints it:  cmake -DROOT=<repository> -P tools/rescue_rev.cmake
if (NOT DEFINED ROOT)
    get_filename_component(ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
endif ()
set(RESCUE_REV_SOURCES
    sysmodule/rescue/source/main.c
    sysmodule/rescue/playguard-rescue.json
    source/core/rescue.c
    source/core/rescue.h)
set(_rescue_all "")
foreach (_f ${RESCUE_REV_SOURCES})
    file(READ "${ROOT}/${_f}" _content HEX)
    string(APPEND _rescue_all "${_content}")
endforeach ()
string(SHA256 _rescue_hash "${_rescue_all}")
string(SUBSTRING "${_rescue_hash}" 0 12 RESCUE_REV)
if (CMAKE_SCRIPT_MODE_FILE)
    execute_process(COMMAND ${CMAKE_COMMAND} -E echo "${RESCUE_REV}")
endif ()

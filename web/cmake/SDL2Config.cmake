# For the web simulator: SDL2 is Emscripten's port (-sUSE_SDL=2, set by
# PlayGuard's CMakeLists.txt), so there is nothing to find or link here.
set(SDL2_FOUND TRUE)
set(SDL2_INCLUDE_DIRS "")
set(SDL2_LIBRARIES "")

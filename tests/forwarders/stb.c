// stb_image (borealis' copy, as nanovg builds it in the app), for
// util/forwarder_icon.cpp; stb_image_write's implementation is
// extern/stb/stb_image_write.c's, linked as the app links it. Neither is
// PlayGuard's code: built without warnings (forwarders_CFLAGS).
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

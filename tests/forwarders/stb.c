// stb_image (borealis' copy, as nanovg builds it in the app) and
// stb_image_write (extern/stb), for util/forwarder_icon.cpp. System
// headers here (-isystem): their warnings are not PlayGuard's.
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO
#include <stb_image_write.h>

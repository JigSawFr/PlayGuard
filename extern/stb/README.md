# stb_image_write

`stb_image_write.h` from [nothings/stb](https://github.com/nothings/stb)
v1.16 (as GLFW ships it in `deps/`), one line changed: the JPEG writer's bit
buffer shifted without overflowing an `int` (undefined behaviour the host
tests' UBSan stops on; the bits it writes are the same). Public domain (or MIT, its author's
choice; in the file's end). `stb_image_write.c` builds its implementation,
without stdio.

PlayGuard writes an NRO forwarder's icon with it, when the `.nro`'s own is
not the 256 × 256 JPEG the HOME menu wants (`source/util/forwarder_icon.cpp`);
reading images is borealis' stb_image (nanovg).

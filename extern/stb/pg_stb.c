// stb_truetype and stb_image (JPEG only), static, behind pg_stb.h (public
// domain: their authors' terms at the end of each header, in borealis'
// nanovg folder, which this includes).
#include "pg_stb.h"

#include <stdlib.h>
#include <string.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wimplicit-fallthrough"
#pragma GCC diagnostic ignored "-Wtype-limits"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <borealis/extern/nanovg/stb_truetype.h>
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_NO_STDIO
#define STBI_NO_LINEAR
#define STBI_NO_HDR
#include <borealis/extern/nanovg/stb_image.h>
// (Not popped: the declarations it leaves unused are warned of at the end
// of this file.)

_Static_assert(sizeof(stbtt_fontinfo) <= sizeof(((PgFont *)0)->opaque), "PgFont too small");

int pg_font_init(PgFont *f, const unsigned char *data, float px)
{
    memset(f, 0, sizeof(*f));
    stbtt_fontinfo *info = (stbtt_fontinfo *)f->opaque;
    if (!data || !stbtt_InitFont(info, data, stbtt_GetFontOffsetForIndex(data, 0))) return 0;
    f->scale = stbtt_ScaleForPixelHeight(info, px);
    stbtt_GetFontVMetrics(info, &f->ascent, &f->descent, &f->line_gap);
    return 1;
}

float pg_font_advance(const PgFont *f, int cp)
{
    int adv = 0, lsb = 0;
    stbtt_GetCodepointHMetrics((const stbtt_fontinfo *)f->opaque, cp, &adv, &lsb);
    return adv * f->scale;
}

float pg_font_kern(const PgFont *f, int cp, int next)
{
    return stbtt_GetCodepointKernAdvance((const stbtt_fontinfo *)f->opaque, cp, next) * f->scale;
}

unsigned char *pg_font_glyph(const PgFont *f, int cp, float shift_x, int *w, int *h, int *x0, int *y0)
{
    return stbtt_GetCodepointBitmapSubpixel((const stbtt_fontinfo *)f->opaque, f->scale, f->scale, shift_x, 0, cp, w,
                                            h, x0, y0);
}

unsigned char *pg_jpeg_rgba(const unsigned char *data, size_t size, int *w, int *h)
{
    int n = 0;
    if (!data || size == 0 || size > 0x7FFFFFFF) return NULL;
    return stbi_load_from_memory(data, (int)size, w, h, &n, 4);
}

void pg_free(void *p)
{
    free(p);
}

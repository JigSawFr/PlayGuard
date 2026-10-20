// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarder_icon.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <cmath>

#include "util/nro.hpp"

namespace forwarder_icon
{

namespace
{

constexpr int PX      = nro::ICON_PX;
constexpr int QUALITY = 90;
// What stb_image is let decode: an icon far larger than this is no icon.
constexpr int MAX_SIDE = 4096;

void append(void* ctx, void* data, int size)
{
    auto* out = static_cast<std::vector<uint8_t>*>(ctx);
    const auto* p = static_cast<const uint8_t*>(data);
    out->insert(out->end(), p, p + size);
}

std::vector<uint8_t> jpeg_of(const std::vector<uint8_t>& rgb)
{
    std::vector<uint8_t> out;
    if (!stbi_write_jpg_to_func(append, &out, PX, PX, 3, rgb.data(), QUALITY)) out.clear();
    return out;
}

// One channel of the source at (x, y), clamped to its edges.
float at(const uint8_t* rgba, int w, int h, int x, int y, int c)
{
    x = std::clamp(x, 0, w - 1);
    y = std::clamp(y, 0, h - 1);
    return rgba[((size_t)y * w + x) * 4 + c];
}

// The source's colour over the target pixel (tx, ty) of a `dst` x `dst`
// square: the mean of the source pixels it covers when shrinking, the
// interpolation of the nearest four when growing; RGBA, 0..255.
void sample(const uint8_t* rgba, int w, int h, int dst, int tx, int ty, float out[4])
{
    const float sx = (float)w / dst, sy = (float)h / dst;
    for (int c = 0; c < 4; c++) out[c] = 0;
    if (sx > 1 || sy > 1) {
        const int x0 = (int)(tx * sx), x1 = std::max(x0 + 1, (int)std::ceil((tx + 1) * sx));
        const int y0 = (int)(ty * sy), y1 = std::max(y0 + 1, (int)std::ceil((ty + 1) * sy));
        int n = 0;
        for (int y = y0; y < y1 && y < h; y++)
            for (int x = x0; x < x1 && x < w; x++, n++)
                for (int c = 0; c < 4; c++) out[c] += at(rgba, w, h, x, y, c);
        for (int c = 0; c < 4; c++) out[c] /= (float)std::max(n, 1);
        return;
    }
    const float fx = (tx + 0.5f) * sx - 0.5f, fy = (ty + 0.5f) * sy - 0.5f;
    const int x = (int)std::floor(fx), y = (int)std::floor(fy);
    const float ax = fx - x, ay = fy - y;
    for (int c = 0; c < 4; c++) {
        const float top = at(rgba, w, h, x, y, c) * (1 - ax) + at(rgba, w, h, x + 1, y, c) * ax;
        const float bottom = at(rgba, w, h, x, y + 1, c) * (1 - ax) + at(rgba, w, h, x + 1, y + 1, c) * ax;
        out[c] = top * (1 - ay) + bottom * ay;
    }
}

uint8_t byte(float v)
{
    return (uint8_t)std::clamp((int)std::lround(v), 0, 255);
}

}   // namespace

std::vector<uint8_t> jpeg_from_rgba(const uint8_t* rgba, int w, int h)
{
    if (!rgba || w <= 0 || h <= 0) return {};
    std::vector<uint8_t> rgb((size_t)PX * PX * 3);
    for (int y = 0; y < PX; y++) {
        for (int x = 0; x < PX; x++) {
            float px[4];
            sample(rgba, w, h, PX, x, y, px);
            const float a = px[3] / 255.0f;
            uint8_t* o = &rgb[((size_t)y * PX + x) * 3];
            for (int c = 0; c < 3; c++) o[c] = byte(px[c] * a + 255.0f * (1 - a));
        }
    }
    return jpeg_of(rgb);
}

std::vector<uint8_t> normalize(const std::vector<uint8_t>& image)
{
    if (nro::icon_usable(image)) return image;
    if (image.empty() || image.size() > 16u * 1024 * 1024) return {};
    int w = 0, h = 0, comp = 0;
    if (!stbi_info_from_memory(image.data(), (int)image.size(), &w, &h, &comp) || w > MAX_SIDE || h > MAX_SIDE)
        return {};
    uint8_t* rgba = stbi_load_from_memory(image.data(), (int)image.size(), &w, &h, &comp, 4);
    if (!rgba) return {};
    std::vector<uint8_t> out = jpeg_from_rgba(rgba, w, h);
    stbi_image_free(rgba);
    return out;
}

std::vector<uint8_t> placeholder(const std::vector<uint8_t>& png)
{
    int w = 0, h = 0, comp = 0;
    uint8_t* rgba = png.empty() ? nullptr : stbi_load_from_memory(png.data(), (int)png.size(), &w, &h, &comp, 4);
    if (!rgba) return {};
    // The emoji 160 px wide in the middle, over the app's pale grey.
    constexpr int SIDE = 160, FROM = (PX - SIDE) / 2;
    const uint8_t bg[3] = { 0xE8, 0xEC, 0xF1 };
    std::vector<uint8_t> rgb((size_t)PX * PX * 3);
    for (int y = 0; y < PX; y++) {
        for (int x = 0; x < PX; x++) {
            uint8_t* o = &rgb[((size_t)y * PX + x) * 3];
            float px[4] = { 0, 0, 0, 0 };
            if (x >= FROM && x < FROM + SIDE && y >= FROM && y < FROM + SIDE) sample(rgba, w, h, SIDE, x - FROM, y - FROM, px);
            const float a = px[3] / 255.0f;
            for (int c = 0; c < 3; c++) o[c] = byte(px[c] * a + bg[c] * (1 - a));
        }
    }
    stbi_image_free(rgba);
    return jpeg_of(rgb);
}

}   // namespace forwarder_icon

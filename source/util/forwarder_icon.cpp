// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "util/forwarder_icon.hpp"

#include <stb_image.h>
#include <stb_image_write.h>

#include <algorithm>
#include <cstring>
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

std::vector<uint8_t> square_rgba(const uint8_t* rgba, int w, int h, int x, int px)
{
    if (!rgba || w <= 0 || h <= 0 || px <= 0) return {};
    // The square, the shorter side's, `x` along the longer one.
    x = std::clamp(x, 0, 100);
    const int side = std::min(w, h);
    const int x0 = w > side ? (w - side) * x / 100 : 0, y0 = h > side ? (h - side) * x / 100 : 0;
    std::vector<uint8_t> square((size_t)side * side * 4);
    for (int y = 0; y < side; y++)
        std::memcpy(&square[(size_t)y * side * 4], rgba + ((size_t)(y0 + y) * w + x0) * 4, (size_t)side * 4);
    if (side == px) return square;
    std::vector<uint8_t> out((size_t)px * px * 4);
    for (int ty = 0; ty < px; ty++) {
        for (int tx = 0; tx < px; tx++) {
            float p[4];
            sample(square.data(), side, side, px, tx, ty, p);
            for (int c = 0; c < 4; c++) out[((size_t)ty * px + tx) * 4 + c] = byte(p[c]);
        }
    }
    return out;
}

std::vector<uint8_t> square_of(const std::vector<uint8_t>& image, int x, int px)
{
    if (image.empty() || image.size() > 16u * 1024 * 1024 || px <= 0) return {};
    int w = 0, h = 0, comp = 0;
    if (!stbi_info_from_memory(image.data(), (int)image.size(), &w, &h, &comp) || w > MAX_SIDE || h > MAX_SIDE)
        return {};
    uint8_t* rgba = stbi_load_from_memory(image.data(), (int)image.size(), &w, &h, &comp, 4);
    if (!rgba) return {};
    std::vector<uint8_t> out = square_rgba(rgba, w, h, x, px);
    stbi_image_free(rgba);
    return out;
}

std::vector<uint8_t> rgba_of(const std::vector<uint8_t>& image, int& w, int& h)
{
    w = h = 0;
    if (image.empty() || image.size() > 16u * 1024 * 1024) return {};
    int comp = 0;
    if (!stbi_info_from_memory(image.data(), (int)image.size(), &w, &h, &comp) || w > MAX_SIDE || h > MAX_SIDE) {
        w = h = 0;
        return {};
    }
    uint8_t* rgba = stbi_load_from_memory(image.data(), (int)image.size(), &w, &h, &comp, 4);
    if (!rgba) {
        w = h = 0;
        return {};
    }
    std::vector<uint8_t> out(rgba, rgba + (size_t)w * h * 4);
    stbi_image_free(rgba);
    return out;
}

std::vector<uint8_t> from_screenshot(const std::vector<uint8_t>& image, int x)
{
    const std::vector<uint8_t> square = square_of(image, x, PX);
    if (square.empty()) return {};
    return jpeg_from_rgba(square.data(), PX, PX);
}

std::vector<uint8_t> exif_thumbnail(const std::vector<uint8_t>& jpeg)
{
    const size_t n = jpeg.size();
    if (n < 4 || jpeg[0] != 0xFF || jpeg[1] != 0xD8) return {};
    // The segments before the image data, an APP1 "Exif" among them.
    for (size_t i = 2; i + 4 <= n;) {
        if (jpeg[i] != 0xFF) return {};
        const uint8_t marker = jpeg[i + 1];
        if (marker == 0xDA || marker == 0xD9) return {};   // the image data: no Exif before it
        const size_t len = (size_t)jpeg[i + 2] << 8 | jpeg[i + 3];
        if (len < 2 || i + 2 + len > n) return {};
        const uint8_t* seg = jpeg.data() + i + 4;
        const size_t size = len - 2;
        if (marker == 0xE1 && size >= 14 && std::memcmp(seg, "Exif\0\0", 6) == 0) {
            // TIFF: its byte order, IFD0, then IFD1 (the thumbnail's).
            const uint8_t* t = seg + 6;
            const size_t tn = size - 6;
            const bool le = t[0] == 'I' && t[1] == 'I';
            if (!le && !(t[0] == 'M' && t[1] == 'M')) return {};
            auto u16 = [&](size_t o) -> uint32_t {
                return o + 2 > tn ? 0 : le ? (uint32_t)(t[o] | t[o + 1] << 8) : (uint32_t)(t[o] << 8 | t[o + 1]);
            };
            auto u32 = [&](size_t o) -> uint32_t {
                if (o + 4 > tn) return 0;
                return le ? (uint32_t)t[o] | (uint32_t)t[o + 1] << 8 | (uint32_t)t[o + 2] << 16 | (uint32_t)t[o + 3] << 24
                          : (uint32_t)t[o] << 24 | (uint32_t)t[o + 1] << 16 | (uint32_t)t[o + 2] << 8 | (uint32_t)t[o + 3];
            };
            const size_t ifd0 = u32(4);
            if (ifd0 < 8 || ifd0 + 2 > tn) return {};
            const size_t ifd1 = u32(ifd0 + 2 + (size_t)u16(ifd0) * 12);
            if (ifd1 < 8 || ifd1 + 2 > tn) return {};
            size_t at = 0, length = 0;
            const uint32_t count = u16(ifd1);
            for (uint32_t k = 0; k < count; k++) {
                const size_t e = ifd1 + 2 + (size_t)k * 12;
                if (e + 12 > tn) return {};
                const uint32_t tag = u16(e);
                // JPEGInterchangeFormat and its length: a LONG each, or a SHORT.
                const uint32_t v = u16(e + 2) == 3 ? u16(e + 8) : u32(e + 8);
                if (tag == 0x0201) at = v;
                else if (tag == 0x0202) length = v;
            }
            if (!at || length < 4 || at > tn || length > tn - at || t[at] != 0xFF || t[at + 1] != 0xD8) return {};
            return std::vector<uint8_t>(t + at, t + at + length);
        }
        i += 2 + len;
    }
    return {};
}

std::vector<uint8_t> with_thumbnail(const std::vector<uint8_t>& jpeg, const std::vector<uint8_t>& thumbnail)
{
    if (jpeg.size() < 2 || thumbnail.empty()) return jpeg;
    // "Exif\0\0", a little-endian TIFF header, an empty IFD0, then IFD1
    // with the thumbnail's place and length, then the thumbnail.
    std::vector<uint8_t> t = { 'I', 'I', 42, 0, 8, 0, 0, 0, 0, 0, 14, 0, 0, 0, 2, 0 };
    auto put16 = [&t](uint32_t v) {
        t.push_back((uint8_t)v);
        t.push_back((uint8_t)(v >> 8));
    };
    auto put32 = [&put16](uint32_t v) {
        put16(v & 0xFFFF);
        put16(v >> 16);
    };
    const uint32_t at = 14 + 2 + 2 * 12 + 4;
    put16(0x0201), put16(4), put32(1), put32(at);
    put16(0x0202), put16(4), put32(1), put32((uint32_t)thumbnail.size());
    put32(0);
    t.insert(t.end(), thumbnail.begin(), thumbnail.end());
    const size_t len = 2 + 6 + t.size();
    if (len > 0xFFFF) return jpeg;
    std::vector<uint8_t> out(jpeg.begin(), jpeg.begin() + 2);
    out.push_back(0xFF);
    out.push_back(0xE1);
    out.push_back((uint8_t)(len >> 8));
    out.push_back((uint8_t)len);
    const char exif[6] = { 'E', 'x', 'i', 'f', 0, 0 };
    out.insert(out.end(), exif, exif + 6);
    out.insert(out.end(), t.begin(), t.end());
    out.insert(out.end(), jpeg.begin() + 2, jpeg.end());
    return out;
}

void draw_over(std::vector<uint8_t>& rgba, int px, const uint8_t* over, int w, int h)
{
    if (!over || w <= 0 || h <= 0 || px <= 0 || rgba.size() != (size_t)px * px * 4) return;
    for (int y = 0; y < px; y++) {
        for (int x = 0; x < px; x++) {
            float o[4];
            sample(over, w, h, px, x, y, o);
            const float a = o[3] / 255.0f;
            uint8_t* d = &rgba[((size_t)y * px + x) * 4];
            for (int c = 0; c < 3; c++) d[c] = byte(o[c] * a + d[c] * (1 - a));
            d[3] = byte(o[3] + d[3] * (1 - a));
        }
    }
}

std::vector<uint8_t> with_badge(const std::vector<uint8_t>& icon, const std::vector<uint8_t>& badge)
{
    std::vector<uint8_t> base = square_of(icon, 50, PX);
    int w = 0, h = 0;
    const std::vector<uint8_t> over = rgba_of(badge, w, h);
    if (base.empty() || over.empty()) return icon;
    draw_over(base, PX, over.data(), w, h);
    std::vector<uint8_t> out = jpeg_from_rgba(base.data(), PX, PX);
    return out.empty() ? icon : out;
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

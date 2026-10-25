// Copyright (C) 2026 JigSawFr.  GPLv3-or-later (see LICENSE).
#include "action/icon_logic.hpp"

namespace icon_logic
{

const std::vector<unsigned char>* Lru::get(uint64_t id)
{
    const auto it = this->index.find(id);
    if (it == this->index.end()) return nullptr;
    this->order.splice(this->order.begin(), this->order, it->second);
    return &it->second->second;
}

void Lru::put(uint64_t id, std::vector<unsigned char> bytes)
{
    const auto it = this->index.find(id);
    if (it != this->index.end()) {
        it->second->second = std::move(bytes);
        this->order.splice(this->order.begin(), this->order, it->second);
        return;
    }
    this->order.emplace_front(id, std::move(bytes));
    this->index[id] = this->order.begin();
    while (this->index.size() > this->capacity && !this->order.empty()) {
        this->index.erase(this->order.back().first);
        this->order.pop_back();
    }
}

void Lru::erase(uint64_t id)
{
    const auto it = this->index.find(id);
    if (it == this->index.end()) return;
    this->order.erase(it->second);
    this->index.erase(it);
}

std::pair<int, int> fit(int w, int h, int max_side)
{
    if (w <= 0 || h <= 0 || max_side <= 0 || (w <= max_side && h <= max_side)) return { w, h };
    // In 64 bits: w * max_side cannot overflow.
    int dw, dh;
    if (w >= h) {
        dw = max_side;
        dh = (int)((int64_t)h * max_side / w);
    } else {
        dh = max_side;
        dw = (int)((int64_t)w * max_side / h);
    }
    return { dw < 1 ? 1 : dw, dh < 1 ? 1 : dh };
}

std::vector<unsigned char> shrink(const unsigned char* src, int w, int h, int dw, int dh)
{
    if (!src || w <= 0 || h <= 0 || dw <= 0 || dh <= 0 || dw > w || dh > h) return {};
    std::vector<unsigned char> out((size_t)dw * dh * 4);
    for (int y = 0; y < dh; y++) {
        // The source rows (then columns) this pixel covers: they tile the
        // source, each used once.
        const int y0 = (int)((int64_t)y * h / dh), y1 = (int)((int64_t)(y + 1) * h / dh);
        for (int x = 0; x < dw; x++) {
            const int x0 = (int)((int64_t)x * w / dw), x1 = (int)((int64_t)(x + 1) * w / dw);
            uint32_t sum[4] = { 0, 0, 0, 0 };
            for (int sy = y0; sy < y1; sy++) {
                const unsigned char* p = src + ((size_t)sy * w + x0) * 4;
                for (int sx = x0; sx < x1; sx++, p += 4)
                    for (int c = 0; c < 4; c++) sum[c] += p[c];
            }
            const uint32_t n = (uint32_t)(x1 - x0) * (uint32_t)(y1 - y0);
            unsigned char* d = &out[((size_t)y * dw + x) * 4];
            for (int c = 0; c < 4; c++) d[c] = (unsigned char)((sum[c] + n / 2) / n);
        }
    }
    return out;
}

}   // namespace icon_logic

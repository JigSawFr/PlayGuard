// sim_hmac — HMAC-SHA256 for the simulated console's PIN rescue proof
// (sim_pctl.c's pctl_pin_hmac).
// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include "util/sha256.hpp"

// `dst` gets 32 bytes.
extern "C" void sim_hmac_sha256(void* dst, const void* key, size_t key_size, const void* msg, size_t size)
{
    const std::array<uint8_t, 32> mac = hmac_sha256({static_cast<const uint8_t*>(key), key_size}, {static_cast<const uint8_t*>(msg), size});
    std::memcpy(dst, mac.data(), mac.size());
}

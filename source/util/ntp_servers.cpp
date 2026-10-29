// Copyright (C) 2026 JigSawFr, (C) 2026 Taylor.  GPLv3-or-later (see LICENSE).
#include "util/ntp_servers.hpp"

#include <cstdio>

#include "core/platform.h"

namespace ntp
{

const std::vector<Region>& regions()
{
    static const std::vector<Region> list = {
        { "global",   { "pool.ntp.org", "0.pool.ntp.org", "1.pool.ntp.org", "2.pool.ntp.org", "3.pool.ntp.org" } },
        { "public",   { "time.cloudflare.com", "time.google.com", "time.apple.com", "time.windows.com",
                        "time.nist.gov", "time.facebook.com", "time.aws.com", "ntp.ubuntu.com" } },
        { "europe",   { "europe.pool.ntp.org", "fr.pool.ntp.org", "de.pool.ntp.org", "uk.pool.ntp.org",
                        "es.pool.ntp.org", "it.pool.ntp.org", "nl.pool.ntp.org", "be.pool.ntp.org",
                        "ch.pool.ntp.org", "pt.pool.ntp.org", "pl.pool.ntp.org", "se.pool.ntp.org" } },
        { "americas", { "north-america.pool.ntp.org", "us.pool.ntp.org", "ca.pool.ntp.org", "mx.pool.ntp.org",
                        "south-america.pool.ntp.org", "br.pool.ntp.org", "ar.pool.ntp.org" } },
        { "asia",     { "asia.pool.ntp.org", "jp.pool.ntp.org", "ntp.nict.jp", "kr.pool.ntp.org",
                        "tw.pool.ntp.org", "hk.pool.ntp.org", "sg.pool.ntp.org", "in.pool.ntp.org",
                        "oceania.pool.ntp.org", "au.pool.ntp.org", "nz.pool.ntp.org" } },
        { "china",    { "cn.pool.ntp.org", "ntp.aliyun.com", "ntp1.tencent.com", "ntp.ntsc.ac.cn" } },
        { "africa",   { "africa.pool.ntp.org", "za.pool.ntp.org", "ae.pool.ntp.org", "il.pool.ntp.org" } },
    };
    return list;
}

std::string default_server_for_console()
{
    int region = -1;
    if (platform_region(&region)) {
        switch (region) {
            case 0: return "jp.pool.ntp.org";             // Japan
            case 1: return "north-america.pool.ntp.org";  // Americas
            case 2: return "europe.pool.ntp.org";         // Europe
            case 3: return "oceania.pool.ntp.org";        // Australia / New Zealand
            case 4: return "asia.pool.ntp.org";           // Hong Kong / Taiwan / Korea
            case 5: return "cn.pool.ntp.org";             // China
            default: break;
        }
    }
    return "pool.ntp.org";
}

namespace
{
// A country's flag: its code's two regional indicator letters ("fr").
std::string flag(std::string code)
{
    if (code == "uk") code = "gb";   // the pool's name for Great Britain
    if (code.size() != 2) return "";
    std::string out;
    for (char c : code) {
        if (c < 'a' || c > 'z') return "";
        char hex[8];
        std::snprintf(hex, sizeof hex, "%x", 0x1F1E6 + (c - 'a'));
        if (!out.empty()) out += '-';
        out += hex;
    }
    return out;
}

}   // namespace

std::string host_emoji(const std::string& host)
{
    static const std::string POOL = "pool.ntp.org";
    if (host == POOL || host.ends_with("." + POOL)) {
        const std::string zone = host.size() > POOL.size() ? host.substr(0, host.size() - POOL.size() - 1) : "";
        if (zone.empty() || (zone.size() == 1 && zone[0] >= '0' && zone[0] <= '9')) return "1f310";   // 🌐
        if (zone == "europe") return "1f1ea-1f1fa";                                                   // 🇪🇺
        if (zone == "north-america" || zone == "south-america") return "1f30e";                       // 🌎
        if (zone == "asia" || zone == "oceania") return "1f30f";                                      // 🌏
        if (zone == "africa") return "1f30d";                                                         // 🌍
        return flag(zone);
    }
    // China's own servers, under .com.
    for (const char* cn : { "aliyun.com", "tencent.com" })
        if (host.ends_with(std::string(".") + cn)) return flag("cn");
    // A country's domain: ntp.nict.jp, ntp.ntsc.ac.cn.
    const size_t dot = host.rfind('.');
    return dot == std::string::npos ? "" : flag(host.substr(dot + 1));
}

std::vector<std::string> cross_check_servers(const std::string& primary)
{
    std::vector<std::string> out;
    for (const char* h : { "time.cloudflare.com", "pool.ntp.org", "time.google.com" }) {
        if (primary != h) out.push_back(h);
        if (out.size() == 2) break;
    }
    return out;
}

}   // namespace ntp

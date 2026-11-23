#pragma once
#include "fx_common.hpp"
#include <cstdio>

namespace wauvio {
    namespace fx {
        struct AudioData {
            int sample_rate = 44100;
            std::vector<std::vector<float>> channels;
            size_t frames() const { return channels.empty() ? 0 : channels[0].size(); }
        };

        inline AudioData read_wav(const std::string& path) {
            FILE* f = std::fopen(path.c_str(), "rb");
            if (!f) throw std::runtime_error("read_wav: cannot open " + path);
            std::vector<unsigned char> d;
            unsigned char tmp[65536]; size_t got;
            while ((got = std::fread(tmp, 1, sizeof tmp, f)) > 0) d.insert(d.end(), tmp, tmp + got);
            std::fclose(f);
            auto u16 = [&](size_t o) { return static_cast<uint32_t>(d[o]) | (static_cast<uint32_t>(d[o + 1]) << 8); };
            auto u32 = [&](size_t o) { return u16(o) | (u16(o + 2) << 16); };
            if (d.size() < 12 || std::memcmp(d.data(), "RIFF", 4) || std::memcmp(d.data() + 8, "WAVE", 4))
                throw std::runtime_error("read_wav: not a RIFF/WAVE file: " + path);
            int fmt_tag = 0, ch = 0, bits = 0, sr = 0; size_t data_off = 0, data_len = 0;
            for (size_t o = 12; o + 8 <= d.size();) {
                const size_t len = u32(o + 4);
                if (!std::memcmp(d.data() + o, "fmt ", 4) && o + 8 + 16 <= d.size()) {
                    fmt_tag = static_cast<int>(u16(o + 8)); ch = static_cast<int>(u16(o + 10));
                    sr = static_cast<int>(u32(o + 12)); bits = static_cast<int>(u16(o + 22));
                    if (fmt_tag == 0xFFFE && len >= 26 && o + 8 + 26 <= d.size()) fmt_tag = static_cast<int>(u16(o + 8 + 24));
                } else if (!std::memcmp(d.data() + o, "data", 4)) {
                    data_off = o + 8; data_len = std::min(len, d.size() - data_off); break;
                }
                o += 8 + len + (len & 1);
            }
            if (!ch || !data_off) throw std::runtime_error("read_wav: missing fmt/data chunk: " + path);
            const bool is_float = (fmt_tag == 3);
            if (!(fmt_tag == 1 || fmt_tag == 3) || !(bits == 8 || bits == 16 || bits == 24 || bits == 32) || (is_float && bits != 32))
                throw std::runtime_error("read_wav: unsupported sample format in " + path);
            const size_t bps = static_cast<size_t>(bits / 8), frames = data_len / (bps * static_cast<size_t>(ch));
            AudioData out; out.sample_rate = sr; out.channels.assign(static_cast<size_t>(ch), std::vector<float>(frames));
            for (size_t i = 0; i < frames; ++i)
                for (int c = 0; c < ch; ++c) {
                    const unsigned char* p = d.data() + data_off + (i * static_cast<size_t>(ch) + static_cast<size_t>(c)) * bps;
                    float v;
                    if (is_float) { float x; std::memcpy(&x, p, 4); v = x; }
                    else if (bits == 8)  v = (static_cast<int>(p[0]) - 128) / 128.0f;
                    else if (bits == 16) v = static_cast<int16_t>(p[0] | (p[1] << 8)) / 32768.0f;
                    else if (bits == 24) { int32_t x = (p[0] << 8) | (p[1] << 16) | (static_cast<int32_t>(p[2]) << 24); v = static_cast<float>(x / 2147483648.0); }
                    else { int32_t x = static_cast<int32_t>(static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
                                                            (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24));
                        v = static_cast<float>(x / 2147483648.0); }
                    out.channels[static_cast<size_t>(c)][i] = v;
                }
            return out;
        }

        inline std::vector<float> resample_linear(const std::vector<float>& in, int from_sr, int to_sr) {
            if (from_sr == to_sr || in.empty()) return in;
            const double ratio = static_cast<double>(from_sr) / to_sr;
            const size_t n = static_cast<size_t>(std::floor(static_cast<double>(in.size()) / ratio));
            std::vector<float> out(n);
            for (size_t i = 0; i < n; ++i) {
                const double p = i * ratio; const size_t k = static_cast<size_t>(p); const float fr = static_cast<float>(p - k);
                const float a = in[k], b = (k + 1 < in.size()) ? in[k + 1] : in[k];
                out[i] = a + (b - a) * fr;
            }
            return out;
        }
    }
}

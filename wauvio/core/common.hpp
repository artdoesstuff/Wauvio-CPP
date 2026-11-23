#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <list>
#include <memory>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#  define WAUVIO_WINDOWS
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <mmsystem.h>
#else
#  define WAUVIO_LINUX
#  include <cstdio>
#endif

namespace wauvio {
    // =============================================================================
    //  §1  CONSTANTS
    // =============================================================================

    constexpr double TWO_PI = 6.283185307179586476925286766559;
    constexpr double PI     = 3.141592653589793238462643383279;

    // =============================================================================
    //  §2  GLOBAL CONFIGURATION
    // =============================================================================

    struct Config {
        int sample_rate = 44100;
        int channels    = 1;
        int bits        = 16;
    };

    inline Config& global_config() {
        static Config cfg;
        return cfg;
    }

    // =============================================================================
    //  §3  BUFFER SYSTEM
    //  Mono: std::vector<float>
    //  Stereo: StereoBuffer - pair of equal-length mono buffers (L, R)
    // =============================================================================

    using Buffer = std::vector<float>;

    struct StereoBuffer {
        Buffer L, R;

        StereoBuffer() = default;
        StereoBuffer(size_t n, float val = 0.0f) : L(n, val), R(n, val) {}

        size_t size() const noexcept { return L.size(); }
        bool   empty() const noexcept { return L.empty(); }

        void resize(size_t n, float val = 0.0f) { L.assign(n, val); R.assign(n, val); }

        /// Collapse to mono by averaging L+R.
        Buffer to_mono() const {
            Buffer out(L.size());
            for (size_t i = 0; i < out.size(); ++i)
                out[i] = (L[i] + R[i]) * 0.5f;
            return out;
        }

        /// Interleave L/R into LRLRLR… PCM for stereo WAV output.
        Buffer interleave() const {
            Buffer out;
            out.reserve(L.size() * 2);
            for (size_t i = 0; i < L.size(); ++i) {
                out.push_back(L[i]);
                out.push_back(R[i]);
            }
            return out;
        }
    };

    // ---------------------------------------------------------------------------
    //  Mono buffer helpers
    // ---------------------------------------------------------------------------

    inline Buffer make_buffer(double seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        return Buffer(static_cast<size_t>(seconds * sample_rate), 0.0f);
    }

    inline StereoBuffer make_stereo(double seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        return StereoBuffer(static_cast<size_t>(seconds * sample_rate), 0.0f);
    }

    inline void mix_into(Buffer& dst, const Buffer& src, float gain = 1.0f) {
        size_t n = std::min(dst.size(), src.size());
        for (size_t i = 0; i < n; ++i) dst[i] += src[i] * gain;
    }

    inline void mix_into(StereoBuffer& dst, const StereoBuffer& src, float gain = 1.0f) {
        size_t n = std::min(dst.size(), src.size());
        for (size_t i = 0; i < n; ++i) {
            dst.L[i] += src.L[i] * gain;
            dst.R[i] += src.R[i] * gain;
        }
    }

    inline void apply_gain(Buffer& buf, float gain) {
        for (auto& s : buf) s *= gain;
    }

    inline void apply_gain(StereoBuffer& buf, float gain) {
        for (auto& s : buf.L) s *= gain;
        for (auto& s : buf.R) s *= gain;
    }

    inline void clamp_buffer(Buffer& buf) {
        for (auto& s : buf) s = std::max(-1.0f, std::min(1.0f, s));
    }

    inline void clamp_buffer(StereoBuffer& buf) {
        for (auto& s : buf.L) s = std::max(-1.0f, std::min(1.0f, s));
        for (auto& s : buf.R) s = std::max(-1.0f, std::min(1.0f, s));
    }

    inline void normalize(Buffer& buf, float target = 0.9f) {
        float peak = 0.0f;
        for (const auto& s : buf) peak = std::max(peak, std::abs(s));
        if (peak > 1e-7f) { const float inv = target / peak; for (auto& s : buf) s *= inv; }
    }

    inline void normalize(StereoBuffer& buf, float target = 0.9f) {
        float peak = 0.0f;
        for (const auto& s : buf.L) peak = std::max(peak, std::abs(s));
        for (const auto& s : buf.R) peak = std::max(peak, std::abs(s));
        if (peak > 1e-7f) {
            const float inv = target / peak;
            for (auto& s : buf.L) s *= inv;
            for (auto& s : buf.R) s *= inv;
        }
    }

    inline void fade_in(Buffer& buf, double fade_seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
        for (size_t i = 0; i < n; ++i)
            buf[i] *= static_cast<float>(i) / static_cast<float>(n);
    }

    inline void fade_out(Buffer& buf, double fade_seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
        for (size_t i = 0; i < n; ++i)
            buf[buf.size() - 1 - i] *= static_cast<float>(i) / static_cast<float>(n);
    }

    inline void fade_in(StereoBuffer& buf, double fade_seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
        for (size_t i = 0; i < n; ++i) {
            float f = static_cast<float>(i) / static_cast<float>(n);
            buf.L[i] *= f;  buf.R[i] *= f;
        }
    }

    inline void fade_out(StereoBuffer& buf, double fade_seconds, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        size_t n = std::min(buf.size(), static_cast<size_t>(fade_seconds * sample_rate));
        for (size_t i = 0; i < n; ++i) {
            float f = static_cast<float>(i) / static_cast<float>(n);
            buf.L[buf.size()-1-i] *= f;
            buf.R[buf.size()-1-i] *= f;
        }
    }

    inline std::vector<int16_t> to_pcm16(const Buffer& buf) {
        std::vector<int16_t> out;
        out.reserve(buf.size());
        for (float s : buf)
            out.push_back(static_cast<int16_t>(std::max(-1.0f,std::min(1.0f,s)) * 32767.0f));
        return out;
    }

    // =============================================================================
    //  §20  STEREO BUFFER UTILITIES
    // =============================================================================

    /// Pan a mono buffer into a stereo buffer using constant-power panning.
    /// pan: -1.0 = full left, 0.0 = centre, 1.0 = full right
    inline StereoBuffer pan_mono(const Buffer& mono, float pan, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const float angle = (pan + 1.0f) * static_cast<float>(PI) * 0.25f; // [0, π/2]
        const float gainL = std::cos(angle);
        const float gainR = std::sin(angle);
        StereoBuffer out(mono.size());
        for (size_t i = 0; i < mono.size(); ++i) {
            out.L[i] = mono[i] * gainL;
            out.R[i] = mono[i] * gainR;
        }
        return out;
    }

    /// Haas stereo widening: L = original, R = original + tiny delay (1–30 ms).
    inline StereoBuffer haas_widen(const Buffer& mono, double delay_ms = 12.0,
                                    int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const size_t delay_samples = static_cast<size_t>(delay_ms * 0.001 * sample_rate);
        StereoBuffer out(mono.size());
        for (size_t i = 0; i < mono.size(); ++i) {
            out.L[i] = mono[i];
            out.R[i] = (i >= delay_samples) ? mono[i - delay_samples] : 0.0f;
        }
        return out;
    }

    /// Mix a mono buffer into a stereo bus at given pan position.
    inline void mix_panned(StereoBuffer& dst, const Buffer& src,
                            float pan = 0.0f, float gain = 1.0f) {
        const float angle = (pan + 1.0f) * static_cast<float>(PI) * 0.25f;
        const float gL = std::cos(angle) * gain;
        const float gR = std::sin(angle) * gain;
        size_t n = std::min(dst.size(), src.size());
        for (size_t i = 0; i < n; ++i) {
            dst.L[i] += src[i] * gL;
            dst.R[i] += src[i] * gR;
        }
    }

    /// Mix a stereo buffer into a stereo bus.
    inline void mix_stereo(StereoBuffer& dst, const StereoBuffer& src, float gain = 1.0f) {
        size_t n = std::min(dst.size(), src.size());
        for (size_t i = 0; i < n; ++i) {
            dst.L[i] += src.L[i] * gain;
            dst.R[i] += src.R[i] * gain;
        }
    }
}

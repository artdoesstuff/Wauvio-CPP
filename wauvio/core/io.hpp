#pragma once

#include "common.hpp"

namespace wauvio {
    // =============================================================================
    //  §28  WAV FILE EXPORT  (mono and stereo)
    // =============================================================================

    namespace detail {
        /// Write a standard 16-bit PCM WAV header and samples to an open file.
        /// `channels` must be 1 (mono) or 2 (stereo).
        inline void write_wav_impl(FILE* f, const std::vector<int16_t>& pcm,
                                    int sample_rate, int channels)
        {
            const uint32_t dsz   = static_cast<uint32_t>(pcm.size() * 2u);
            const uint32_t chksz = 36u + dsz;
            const uint16_t ch    = static_cast<uint16_t>(channels);
            const uint16_t ba    = static_cast<uint16_t>(ch * 2u);   // block align
            const uint32_t abps  = static_cast<uint32_t>(sample_rate) * ba;

            auto w4 = [&](uint32_t v){ std::fwrite(&v, 4, 1, f); };
            auto w2 = [&](uint16_t v){ std::fwrite(&v, 2, 1, f); };
            auto ws = [&](const char* s, size_t n){ std::fwrite(s, 1, n, f); };

            ws("RIFF", 4);  w4(chksz);  ws("WAVE", 4);
            ws("fmt ",  4);  w4(16u);   w2(1u);  w2(ch);
            w4(static_cast<uint32_t>(sample_rate));  w4(abps);  w2(ba);  w2(16u);
            ws("data",  4);  w4(dsz);
            std::fwrite(pcm.data(), 2, pcm.size(), f);
        }
    }

    /// Save a mono buffer as a 16-bit PCM WAV file.
    inline void save_wav(const Buffer& buf, const std::string& path, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const auto pcm = to_pcm16(buf);
        FILE* f = nullptr;
#ifdef WAUVIO_WINDOWS
        fopen_s(&f, path.c_str(), "wb");
#else
        f = std::fopen(path.c_str(), "wb");
#endif
        if (!f) throw std::runtime_error("save_wav: cannot open " + path);
        detail::write_wav_impl(f, pcm, sample_rate, 1);
        std::fclose(f);
    }

    /// Save a stereo buffer as an interleaved L/R 16-bit PCM WAV file.
    inline void save_wav_stereo(const StereoBuffer& buf, const std::string& path,
                                 int sample_rate = 0)
    {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        const auto pcm = to_pcm16(buf.interleave());
        FILE* f = nullptr;
#ifdef WAUVIO_WINDOWS
        fopen_s(&f, path.c_str(), "wb");
#else
        f = std::fopen(path.c_str(), "wb");
#endif
        if (!f) throw std::runtime_error("save_wav_stereo: cannot open " + path);
        detail::write_wav_impl(f, pcm, sample_rate, 2);
        std::fclose(f);
    }

    // =============================================================================
    //  §29  PLAYBACK  v1.3.3
    // =============================================================================

    namespace detail {
        // ---------------------------------------------------------------------------
        //  Internal state of one playback stream
        // ---------------------------------------------------------------------------
        class PlaybackInstance {
        public:
            // Construct with a pre-converted PCM buffer + metadata.
            // The PCM data is moved in - this instance owns it exclusively.
            PlaybackInstance(std::vector<int16_t> pcm, int sample_rate, int channels)
                : pcm_(std::move(pcm))
                , sample_rate_(sample_rate)
                , channels_(channels)
                , stopped_(false)
                , paused_(false)
                , playing_(true)
                , volume_(1.0f)
            {}

            // Non-copyable, non-movable (shared_ptr is the indirection layer).
            PlaybackInstance(const PlaybackInstance&)            = delete;
            PlaybackInstance& operator=(const PlaybackInstance&) = delete;

            // ---- Public control API ----

            void stop() {
                stopped_.store(true, std::memory_order_release);
                // Wake the worker if it is paused so it can exit cleanly.
                {
                    std::lock_guard<std::mutex> lk(pause_mtx_);
                    paused_.store(false, std::memory_order_release);
                }
                pause_cv_.notify_all();
            }

            void pause() {
                paused_.store(true, std::memory_order_release);
            }

            void resume() {
                {
                    std::lock_guard<std::mutex> lk(pause_mtx_);
                    paused_.store(false, std::memory_order_release);
                }
                pause_cv_.notify_all();
            }

            bool is_playing() const noexcept {
                return playing_.load(std::memory_order_acquire);
            }

            void set_volume(float v) noexcept {
                volume_.store(v < 0.0f ? 0.0f : (v > 2.0f ? 2.0f : v),
                              std::memory_order_relaxed);
            }

            float volume() const noexcept {
                return volume_.load(std::memory_order_relaxed);
            }

            /// Returns the current playback position in seconds.
            double position_seconds() const noexcept {
                const size_t pos = position_.load(std::memory_order_relaxed);
                return static_cast<double>(pos) / (sample_rate_ * channels_);
            }

            /// Total duration of the buffer in seconds.
            double duration_seconds() const noexcept {
                return static_cast<double>(pcm_.size()) / (sample_rate_ * channels_);
            }

            /// Seek to `seconds` into the buffer.  Clamped to [0, duration].
            void seek(double seconds) noexcept {
                const size_t target = static_cast<size_t>(
                    std::max(0.0, seconds) * sample_rate_ * channels_);
                position_.store(std::min(target, pcm_.size()), std::memory_order_relaxed);
                seek_pending_.store(true, std::memory_order_release);
            }

            // ---- Worker entry point ----
            // Called once from the worker thread spawned by PlaybackRuntime.

            void run() {
#ifdef WAUVIO_LINUX
                run_linux();
#elif defined(WAUVIO_WINDOWS)
                run_windows();
#endif
                playing_.store(false, std::memory_order_release);
            }

        private:
            // PCM data (owned exclusively by this instance)
            std::vector<int16_t> pcm_;
            int                  sample_rate_;
            int                  channels_;

            // Atomic flags - readable from any thread without locking
            std::atomic<bool>   stopped_;
            std::atomic<bool>   paused_;
            std::atomic<bool>   playing_;
            std::atomic<float>  volume_;
            std::atomic<size_t> position_{0};       ///< Current read position (in PCM samples)
            std::atomic<bool>   seek_pending_{false}; ///< Worker re-reads position_ when true

            // Pause mechanism
            std::mutex              pause_mtx_;
            std::condition_variable pause_cv_;

            // Block the worker while paused (or exit immediately if stopped).
            void wait_while_paused() {
                std::unique_lock<std::mutex> lk(pause_mtx_);
                pause_cv_.wait(lk, [this]{
                    return !paused_.load(std::memory_order_acquire)
                        || stopped_.load(std::memory_order_acquire);
                });
            }

            // Apply current volume to a chunk of samples (in-place, temporary copy).
            // We write from a local scaled buffer so the master pcm_ stays intact
            // (allows replay in future iterations if needed).
            static void scale_chunk(const int16_t* src, int16_t* dst,
                                     size_t count, float vol) noexcept
            {
                for (size_t i = 0; i < count; ++i) {
                    float s = static_cast<float>(src[i]) * vol;
                    s = s < -32768.0f ? -32768.0f : (s > 32767.0f ? 32767.0f : s);
                    dst[i] = static_cast<int16_t>(s);
                }
            }

#ifdef WAUVIO_LINUX
            // ---- Linux: stream PCM to ffmpeg/ffplay via popen ----
            // We write in small chunks so we can honour stop/pause between writes.

            static constexpr size_t CHUNK_FRAMES = 4096; // frames per write

            void run_linux() {
                const std::string sr  = std::to_string(sample_rate_);
                const std::string ach = std::to_string(channels_);
                const std::string cmd =
                    "ffmpeg -hide_banner -loglevel error "
                    "-f s16le -ar " + sr + " -ac " + ach + " -i pipe:0 "
                    "-f alsa default 2>/dev/null || "
                    "ffmpeg -hide_banner -loglevel error "
                    "-f s16le -ar " + sr + " -ac " + ach + " -i pipe:0 "
                    "-f pulse default 2>/dev/null || "
                    "ffplay -hide_banner -loglevel error "
                    "-f s16le -ar " + sr + " -ac " + ach + " -";

                FILE* pipe = popen(cmd.c_str(), "w");
                if (!pipe) { playing_.store(false, std::memory_order_release); return; }

                const size_t chunk_samples = CHUNK_FRAMES * static_cast<size_t>(channels_);
                std::vector<int16_t> chunk(chunk_samples);
                size_t pos = position_.load(std::memory_order_relaxed);

                while (pos < pcm_.size()) {
                    // Honour stop
                    if (stopped_.load(std::memory_order_acquire)) break;

                    // Honour pause: block until resumed or stopped
                    if (paused_.load(std::memory_order_acquire)) {
                        wait_while_paused();
                        if (stopped_.load(std::memory_order_acquire)) break;
                    }

                    // Honour seek
                    if (seek_pending_.load(std::memory_order_acquire)) {
                        pos = position_.load(std::memory_order_relaxed);
                        seek_pending_.store(false, std::memory_order_release);
                    }

                    const size_t remaining = pcm_.size() - pos;
                    const size_t n         = remaining < chunk_samples ? remaining : chunk_samples;

                    scale_chunk(pcm_.data() + pos, chunk.data(), n,
                                volume_.load(std::memory_order_relaxed));

                    if (std::fwrite(chunk.data(), sizeof(int16_t), n, pipe) != n) break;
                    pos += n;
                    position_.store(pos, std::memory_order_relaxed);
                }

                pclose(pipe);
            }
#endif // WAUVIO_LINUX

#ifdef WAUVIO_WINDOWS
            // ---- Windows: stream PCM via WaveOut double-buffering ----
            // We use two alternating WAVEHDR buffers so we can check
            // stop/pause between buffer completions without a Sleep() spin.

            static constexpr DWORD CHUNK_FRAMES_WIN = 4096;

            void run_windows() {
                const DWORD  sr   = static_cast<DWORD>(sample_rate_);
                const WORD   ch   = static_cast<WORD>(channels_);
                const WORD   ba   = static_cast<WORD>(ch * 2);          // block align
                const DWORD  abps = static_cast<DWORD>(sr) * ba;        // avg bytes/sec

                WAVEFORMATEX wfx{};
                wfx.wFormatTag      = WAVE_FORMAT_PCM;
                wfx.nChannels       = ch;
                wfx.nSamplesPerSec  = sr;
                wfx.wBitsPerSample  = 16;
                wfx.nBlockAlign     = ba;
                wfx.nAvgBytesPerSec = abps;

                HWAVEOUT hwo = nullptr;
                if (waveOutOpen(&hwo, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL)
                        != MMSYSERR_NOERROR)
                    return;

                const size_t chunk_samples =
                    static_cast<size_t>(CHUNK_FRAMES_WIN) * static_cast<size_t>(channels_);
                const DWORD  chunk_bytes   = static_cast<DWORD>(chunk_samples * 2u);

                // Two alternating PCM buffers (double-buffering)
                std::vector<int16_t> bufs[2];
                bufs[0].resize(chunk_samples);
                bufs[1].resize(chunk_samples);
                WAVEHDR hdrs[2]{};
                for (int b = 0; b < 2; ++b) {
                    hdrs[b].lpData         = reinterpret_cast<LPSTR>(bufs[b].data());
                    hdrs[b].dwBufferLength = chunk_bytes;
                    waveOutPrepareHeader(hwo, &hdrs[b], sizeof(WAVEHDR));
                }

                size_t pos  = position_.load(std::memory_order_relaxed);
                int    slot = 0;

                while (pos < pcm_.size()) {
                    if (stopped_.load(std::memory_order_acquire)) break;
                    if (paused_.load(std::memory_order_acquire)) {
                        // Wait while paused, but don't block WaveOut - let it drain
                        wait_while_paused();
                        if (stopped_.load(std::memory_order_acquire)) break;
                    }

                    // Honour seek
                    if (seek_pending_.load(std::memory_order_acquire)) {
                        pos = position_.load(std::memory_order_relaxed);
                        seek_pending_.store(false, std::memory_order_release);
                    }

                    WAVEHDR& hdr = hdrs[slot];

                    // Wait for this slot to finish if it's still playing
                    while ((hdr.dwFlags & WHDR_INQUEUE) && !stopped_.load(std::memory_order_acquire)) {
                        Sleep(1);
                    }
                    if (stopped_.load(std::memory_order_acquire)) break;

                    const size_t remaining = pcm_.size() - pos;
                    const size_t n = remaining < chunk_samples ? remaining : chunk_samples;

                    scale_chunk(pcm_.data() + pos, bufs[slot].data(), n,
                                volume_.load(std::memory_order_relaxed));

                    // Zero-pad the last partial chunk
                    if (n < chunk_samples)
                        std::fill(bufs[slot].begin() + static_cast<ptrdiff_t>(n),
                                  bufs[slot].end(), int16_t{0});

                    hdr.dwBufferLength = static_cast<DWORD>(n * 2u);
                    waveOutWrite(hwo, &hdr, sizeof(WAVEHDR));

                    pos  += n;
                    position_.store(pos, std::memory_order_relaxed);
                    slot  = 1 - slot;
                }

                // Drain: wait for both buffers to complete
                for (int b = 0; b < 2; ++b) {
                    while ((hdrs[b].dwFlags & WHDR_INQUEUE)) Sleep(1);
                }

                for (int b = 0; b < 2; ++b)
                    waveOutUnprepareHeader(hwo, &hdrs[b], sizeof(WAVEHDR));

                waveOutReset(hwo);
                waveOutClose(hwo);
            }
#endif // WAUVIO_WINDOWS
        };

        // ---------------------------------------------------------------------------
        //  PlaybackRuntime - singleton registry of active instances
        // ---------------------------------------------------------------------------
        class PlaybackRuntime {
        public:
            static PlaybackRuntime& instance() {
                static PlaybackRuntime rt;
                return rt;
            }

            // Launch a new PlaybackInstance on its own thread.
            // Returns a shared_ptr so the caller can build a PlaybackHandle.
            std::shared_ptr<PlaybackInstance> launch(
                    std::vector<int16_t> pcm, int sample_rate, int channels)
            {
                auto inst = std::make_shared<PlaybackInstance>(
                        std::move(pcm), sample_rate, channels);

                // Register a weak_ptr in the registry before spawning the thread.
                {
                    std::lock_guard<std::mutex> lk(registry_mtx_);
                    // Opportunistic cleanup of dead entries
                    registry_.remove_if(
                        [](const std::weak_ptr<PlaybackInstance>& w){ return w.expired(); });
                    registry_.emplace_back(inst);
                }

                // Spawn worker thread.  The thread holds a strong shared_ptr so the
                // instance lives until playback finishes even if the caller discards
                // their PlaybackHandle.
                std::thread([inst]() mutable {
                    inst->run();
                    // inst falls out of scope here - ref-count drops, memory freed
                }).detach();

                return inst;
            }

            // Request all active streams to stop (useful for clean shutdown).
            void stop_all() {
                std::lock_guard<std::mutex> lk(registry_mtx_);
                for (auto& w : registry_) {
                    if (auto s = w.lock()) s->stop();
                }
                registry_.clear();
            }

            ~PlaybackRuntime() { stop_all(); }

        private:
            PlaybackRuntime() = default;

            std::mutex                                    registry_mtx_;
            std::list<std::weak_ptr<PlaybackInstance>>    registry_;
        };
    }

    // =============================================================================
    //  PUBLIC API - PlaybackHandle
    // =============================================================================

    /// Lightweight, copyable handle to an in-flight playback stream.
    /// All methods are thread-safe.
    /// Discarding the handle does NOT stop playback (fire-and-forget).
    class PlaybackHandle {
    public:
        PlaybackHandle() = default;
        explicit PlaybackHandle(std::shared_ptr<detail::PlaybackInstance> inst)
            : inst_(std::move(inst)) {}

        /// Stop playback immediately and release resources.
        void stop()   { if (inst_) inst_->stop();   }

        /// Pause playback (resume() to continue).
        void pause()  { if (inst_) inst_->pause();  }

        /// Resume after pause().
        void resume() { if (inst_) inst_->resume(); }

        /// Returns true while the worker thread is still active.
        bool is_playing() const noexcept {
            return inst_ && inst_->is_playing();
        }

        /// Set output volume multiplier [0.0, 2.0].  Thread-safe.
        void set_volume(float v) { if (inst_) inst_->set_volume(v); }

        /// Current playback position in seconds.
        double position_seconds() const noexcept {
            return inst_ ? inst_->position_seconds() : 0.0;
        }

        /// Total buffer duration in seconds.
        double duration_seconds() const noexcept {
            return inst_ ? inst_->duration_seconds() : 0.0;
        }

        /// Seek to `seconds`.  Best-effort: the worker honours it between chunks.
        void seek(double seconds) { if (inst_) inst_->seek(seconds); }

        /// Block the calling thread until playback finishes (or is stopped).
        void wait() const {
            if (!inst_) return;
            while (inst_->is_playing()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        }

        /// True if this handle refers to a real stream.
        explicit operator bool() const noexcept { return inst_ != nullptr; }

    private:
        std::shared_ptr<detail::PlaybackInstance> inst_;
    };

    // =============================================================================
    //  play_async  -  non-blocking launch
    // =============================================================================

    /// Launch mono buffer playback asynchronously.
    /// Returns immediately; audio runs on a background thread.
    inline PlaybackHandle play_async(const Buffer& buf, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        auto pcm = to_pcm16(buf);
        auto inst = detail::PlaybackRuntime::instance().launch(
                std::move(pcm), sample_rate, 1);
        return PlaybackHandle(std::move(inst));
    }

    /// Launch stereo buffer playback asynchronously.
    /// Returns immediately; audio runs on a background thread.
    inline PlaybackHandle play_async(const StereoBuffer& buf, int sample_rate = 0) {
        if (sample_rate <= 0) sample_rate = global_config().sample_rate;
        Buffer il  = buf.interleave();
        auto pcm   = to_pcm16(il);
        auto inst  = detail::PlaybackRuntime::instance().launch(
                std::move(pcm), sample_rate, 2);
        return PlaybackHandle(std::move(inst));
    }

    // =============================================================================
    //  play  -  blocking (backward-compatible)
    // =============================================================================

    /// Blocking playback of a mono buffer.
    /// Internally delegates to play_async() and waits for completion.
    inline void play(const Buffer& buf, int sample_rate = 0) {
        play_async(buf, sample_rate).wait();
    }

    /// Blocking playback of a stereo buffer.
    /// Internally delegates to play_async() and waits for completion.
    inline void play(const StereoBuffer& buf, int sample_rate = 0) {
        play_async(buf, sample_rate).wait();
    }

    /// Stop every active playback stream immediately.
    inline void stop_all_playback() {
        detail::PlaybackRuntime::instance().stop_all();
    }
}

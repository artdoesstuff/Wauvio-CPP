#pragma once
#include "../mastering/master.hpp"

namespace wauvio {
    namespace fx {
        struct Source {
            std::string  name;
            StereoBuffer audio;
            double       start_sec = 0.0;   ///< where this source begins on the timeline
            float        gain      = 1.0f;  ///< fader (linear)
        };

        struct SidechainSettings {
            double threshold_db = -30.0;
            double ratio        = 6.0;
            double attack_ms    = 5.0;
            double release_ms   = 150.0;
            double knee_db      = 4.0;
            double makeup_db    = 0.0;
        };

        class Mixer {
        public:
            explicit Mixer(int sample_rate = 0) : sr_(sample_rate > 0 ? sample_rate : global_config().sample_rate) {}
            int sample_rate() const noexcept { return sr_; }

            size_t add_source(Source s) { srcs_.push_back(Entry{std::move(s), Chain()}); return srcs_.size() - 1; }
            size_t source_count() const noexcept { return srcs_.size(); }
            Source& source(size_t i) { return srcs_.at(i).src; }
            /// Insert chain for a source (runs on the source before sidechain, fader and sends).
            Chain& insert(size_t i) { return srcs_.at(i).chain; }

            size_t add_bus(const std::string& name) { buses_.push_back(Bus{name, Chain()}); return buses_.size() - 1; }
            size_t bus_count() const noexcept { return buses_.size(); }
            Chain& bus(size_t i) { return buses_.at(i).chain; }
            /// Look a bus up by name; throws std::out_of_range if missing.
            size_t bus_index(const std::string& name) const {
                for (size_t i = 0; i < buses_.size(); ++i) if (buses_[i].name == name) return i;
                throw std::out_of_range("Mixer: no bus named '" + name + "'");
            }
            /// Send `amount` (linear) of a source to a bus. post_fader = scaled by the source fader.
            void send(size_t src, size_t bus_idx, double amount, bool post_fader = true) {
                if (src >= srcs_.size() || bus_idx >= buses_.size()) throw std::out_of_range("Mixer::send: bad index");
                sends_.push_back(Send{src, bus_idx, static_cast<float>(amount), post_fader});
            }
            void send(size_t src, const std::string& bus_name, double amount, bool post_fader = true) { send(src, bus_index(bus_name), amount, post_fader); }

            /// Duck `target` by the level of `trigger` (post-insert, pre-fader signal of the trigger).
            void sidechain(size_t target, size_t trigger, const SidechainSettings& s = SidechainSettings()) {
                if (target >= srcs_.size() || trigger >= srcs_.size() || target == trigger) throw std::out_of_range("Mixer::sidechain: bad index");
                sc_.push_back(Sc{target, trigger, s});
            }

            Chain& master() { return master_; }
            /// Optional final stage: master the summed mix to a loudness target (after the master chain).
            void set_mastering(const MasterSettings& s) { mastering_ = s; do_mastering_ = true; }
            void clear_mastering() { do_mastering_ = false; }
            bool has_mastering() const noexcept { return do_mastering_; }
            const MasterReport& last_master_report() const noexcept { return report_; }

            /// Renders the mix (inserts -> sidechain -> sends/buses -> sum -> master chain -> mastering).
            /// The result is NOT normalised or clamped; write it with a level-preserving writer
            /// (see fx::write_wav in wauvio_fx_ext.hpp) when mastering to a loudness target.
            StereoBuffer render() {
                const size_t N = srcs_.size();
                // 1) insert chains (extend each source by its tail)
                std::vector<StereoBuffer> buf(N);
                for (size_t i = 0; i < N; ++i) {
                    buf[i] = srcs_[i].src.audio;
                    srcs_[i].chain.process(buf[i], sr_, true);
                }
                // 2) sidechain ducking, all keys taken from the stage-1 signals
                if (!sc_.empty()) {
                    std::vector<StereoBuffer> ducked = buf;
                    for (const Sc& sc : sc_) {
                        StereoBuffer& tgt = ducked[sc.target];
                        const StereoBuffer& key = buf[sc.trigger];
                        const long long shift = std::llround((srcs_[sc.target].src.start_sec - srcs_[sc.trigger].src.start_sec) * sr_);
                        Buffer kl(tgt.size(), 0.0f), kr(tgt.size(), 0.0f);
                        for (size_t j = 0; j < tgt.size(); ++j) {
                            const long long k = static_cast<long long>(j) + shift;
                            if (k >= 0 && static_cast<size_t>(k) < key.size()) { kl[j] = key.L[static_cast<size_t>(k)]; kr[j] = key.R[static_cast<size_t>(k)]; }
                        }
                        Compressor c;
                        c.threshold_db = sc.s.threshold_db; c.ratio = sc.s.ratio; c.attack_ms = sc.s.attack_ms;
                        c.release_ms = sc.s.release_ms; c.knee_db = sc.s.knee_db; c.makeup_db = sc.s.makeup_db;
                        c.detector = Compressor::Detect::Peak; c.stereo_link = 1.0;
                        c.prepare(sr_);
                        c.process_sidechain(tgt.L.data(), tgt.R.data(), tgt.size(), kl.data(), kr.data());
                    }
                    buf = std::move(ducked);
                }
                // 3) buses
                std::vector<StereoBuffer> busbuf(buses_.size());
                for (const Send& sd : sends_) {
                    const StereoBuffer& s = buf[sd.src];
                    const double start = srcs_[sd.src].src.start_sec;
                    const size_t off = static_cast<size_t>(std::llround(std::max(0.0, start) * sr_));
                    StereoBuffer& b = busbuf[sd.bus];
                    if (b.size() < off + s.size()) { b.L.resize(off + s.size(), 0.0f); b.R.resize(off + s.size(), 0.0f); }   // (StereoBuffer::resize would wipe the content)
                    const float g = sd.amount * (sd.post ? srcs_[sd.src].src.gain : 1.0f);
                    for (size_t j = 0; j < s.size(); ++j) { b.L[off + j] += s.L[j] * g; b.R[off + j] += s.R[j] * g; }
                }
                for (size_t i = 0; i < buses_.size(); ++i) if (!busbuf[i].empty()) buses_[i].chain.process(busbuf[i], sr_, true);

                // 4) sum
                size_t total = 0;
                for (size_t i = 0; i < N; ++i) total = std::max(total, off_of(i) + buf[i].size());
                for (auto& b : busbuf) total = std::max(total, b.size());
                StereoBuffer out(total);
                for (size_t i = 0; i < N; ++i) {
                    const size_t off = off_of(i); const float g = srcs_[i].src.gain;
                    for (size_t j = 0; j < buf[i].size(); ++j) { out.L[off + j] += buf[i].L[j] * g; out.R[off + j] += buf[i].R[j] * g; }
                }
                for (auto& b : busbuf) for (size_t j = 0; j < b.size(); ++j) { out.L[j] += b.L[j]; out.R[j] += b.R[j]; }

                // 5) master chain, then optional loudness mastering
                if (!master_.empty()) master_.process(out, sr_, true);
                if (do_mastering_) report_ = fx::master(out, sr_, mastering_);
                return out;
            }

        private:
            struct Entry { Source src; Chain chain; };
            struct Bus   { std::string name; Chain chain; };
            struct Send  { size_t src, bus; float amount; bool post; };
            struct Sc    { size_t target, trigger; SidechainSettings s; };
            size_t off_of(size_t i) const { return static_cast<size_t>(std::llround(std::max(0.0, srcs_[i].src.start_sec) * sr_)); }

            int sr_;
            std::vector<Entry> srcs_;
            std::vector<Bus> buses_;
            std::vector<Send> sends_;
            std::vector<Sc> sc_;
            Chain master_;
            MasterSettings mastering_; bool do_mastering_ = false; MasterReport report_;
        };
    }
}

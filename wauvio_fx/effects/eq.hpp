#pragma once
#include "../core/fx_effect.hpp"

namespace wauvio {
    namespace fx {
        class EQ : public Effect {
        public:
            struct Band {
                FilterType type = FilterType::Peak;
                double freq = 1000.0;
                double gain_db = 0.0;     ///< shelves / peak
                double q = 0.7071;
                int    slope_db_oct = 12; ///< HighPass / LowPass only: 12, 24, 36 or 48
                bool   enabled = true;
            };

            std::string name() const override { return "EQ"; }

            EQ& highpass(double f, int slope_db_oct = 12)  { return add({FilterType::HighPass, f, 0, 0.7071, slope_db_oct, true}); }
            EQ& lowpass(double f, int slope_db_oct = 12)   { return add({FilterType::LowPass,  f, 0, 0.7071, slope_db_oct, true}); }
            EQ& low_shelf(double f, double gain_db, double q = 0.7071)  { return add({FilterType::LowShelf,  f, gain_db, q, 12, true}); }
            EQ& high_shelf(double f, double gain_db, double q = 0.7071) { return add({FilterType::HighShelf, f, gain_db, q, 12, true}); }
            EQ& peak(double f, double gain_db, double q = 1.0)          { return add({FilterType::Peak, f, gain_db, q, 12, true}); }
            EQ& notch(double f, double q = 8.0)                         { return add({FilterType::Notch, f, 0, q, 12, true}); }
            EQ& add(const Band& b) { bands_.push_back(b); return *this; }
            EQ& clear_bands() { bands_.clear(); return *this; }

            std::vector<Band>& bands() { return bands_; }
            const std::vector<Band>& bands() const { return bands_; }

            /// Combined magnitude response in dB at `freq` (uses the current sample rate).
            double response_db(double freq) const {
                const double fs = sr_;
                double mag = 1.0;
                for (const auto& b : bands_) {
                    if (!b.enabled) continue;
                    for (const auto& c : sections_for(b, fs)) mag *= biquad_magnitude(c, fs, freq);
                }
                return lin_to_db(mag);
            }

            void reset() override { for (auto& ch : st_) for (auto& b : ch) b.reset(); }

            using Effect::process;

            void process(float* L, float* R, size_t n) override {
                rebuild();
                for (size_t s = 0; s < coefs_.size(); ++s) {
                    Biquad& bl = st_[0][s]; Biquad& br = st_[1][s];
                    for (size_t i = 0; i < n; ++i) { L[i] = bl.tick(L[i]); R[i] = br.tick(R[i]); }
                }
            }

        private:
            static std::vector<BiquadCoef> sections_for(const Band& b, double fs) {
                std::vector<BiquadCoef> out;
                if (b.type == FilterType::HighPass || b.type == FilterType::LowPass) {
                    static const std::vector<std::vector<double>> qs = {
                        {0.70710678}, {0.54119610, 1.30656296},
                        {0.51763809, 0.70710678, 1.93185165}, {0.50979558, 0.60134489, 0.89997622, 2.56291545} };
                    const int order_idx = std::min(3, std::max(0, b.slope_db_oct / 12 - 1));
                    for (double q : qs[static_cast<size_t>(order_idx)]) out.push_back(design_biquad(b.type, fs, b.freq, q));
                } else {
                    out.push_back(design_biquad(b.type, fs, b.freq, b.q, b.gain_db));
                }
                return out;
            }
            void rebuild() {
                coefs_.clear();
                for (const auto& b : bands_) {
                    if (!b.enabled) continue;
                    for (const auto& c : sections_for(b, sr_)) coefs_.push_back(c);
                }
                for (auto& ch : st_) {
                    if (ch.size() != coefs_.size()) ch.assign(coefs_.size(), Biquad());
                    for (size_t i = 0; i < coefs_.size(); ++i) ch[i].set(coefs_[i]);
                }
            }
            std::vector<Band> bands_;
            std::vector<BiquadCoef> coefs_;
            std::vector<Biquad> st_[2];
        };
    }
}

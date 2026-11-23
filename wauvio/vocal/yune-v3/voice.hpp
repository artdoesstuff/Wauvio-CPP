#pragma once

#include "japanese.hpp"
#include "../../../wauvio.hpp"

#include <algorithm>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace wauvio {
    namespace vocal {
        namespace v3 {
            struct VowelTarget {
                float f1, f2, f3;       // Hz
                float b1, b2, b3;       // bandwidths, Hz
                float gain;             // per-vowel loudness trim
                float f1_track;         // 0..1: how strongly F1 follows F0 when F0 approaches F1
                float f2_track;         // 0..1: same for F2 (only matters for very high notes)
            };

            // Engine gains. Lives inside VoiceModel (V2) so two singers can be tuned
            // independently and rendering is thread-safe.
            struct Tune {
                float out_scale     = 0.65f;   // overall level of voiced signal
                float asp_gain      = 3.00f;   // aspiration noise level into the tract
                float fric_gain     = 0.30f;   // frication level scale
                float upper_gain    = 4.40f;   // parallel upper-formant bank (V3: the former hidden x2.2 is folded in)
                float sing_gain     = 2.40f;   // singer's formant           (V3: the former hidden x1.2 is folded in)
                float air_gain      = 0.10f;   // un-filtered breath "air"
                float pre_gain      = 1.0f;    // source pre-emphasis amount (x brightness)
                float f23_gain      = 30.0f;   // parallel boost of the moving F2/F3 (x brightness)
            };

            // Consonant loci (V3). The F2/F3 a consonant's burst/closure points toward are written as locus
            // equations, F_locus = slope * F_vowel + intercept (Sussman-style), one set per place of articulation.
            // Intercepts are for the reference (Yune-sized) tract and are scaled to the voice by VoiceModel::tract_scale().
            struct LocusEq { float s2, c2, s3, c3; };
            enum Place { LABIAL = 0, ALVEOLAR = 1, PALATAL = 2, VELAR = 3 };

            // Identity of the singer. The "Yune" character lives here; personas (see persona.hpp)
            // are bounded variations expressed through Style, not edits of this struct.
            struct VoiceModel {
                std::string name = "Wauvio Japanese Female (default)";
                Tune tune;

                // Range (MIDI notes)
                int range_low = 52, range_high = 88;       // E3 .. E6 comfortable
                int hard_low = 40,  hard_high = 96;

                // Japanese vowels, female, deliberately peripheral for clarity.
                VowelTarget vowel[5] = {
                    /* a */ { 880, 1480, 3000, 135, 130, 190, 1.00f, 0.85f, 0.0f },
                    /* i */ { 330, 2850, 3500,  95, 150, 200, 1.00f, 0.45f, 0.0f },
                    /* u */ { 370, 1450, 2750, 100, 125, 180, 1.00f, 0.45f, 0.0f },
                    /* e */ { 560, 2450, 3200, 110, 135, 190, 1.00f, 0.75f, 0.0f },
                    /* o */ { 570,  980, 2800, 110, 120, 180, 1.00f, 0.75f, 0.0f },
                };
                // Fixed upper formants / brightness structure
                float f4 = 4300.f, f5 = 5600.f, f6 = 7200.f;
                float b4 = 260.f,  b5 = 320.f,  b6 = 500.f;
                float singer_formant_hz = 3100.f, singer_formant_bw = 600.f;

                // Glottal source defaults (Rd parameter of the LF model; lower = tenser/brighter)
                float rd_base      = 0.95f;
                float aspiration   = 0.020f;    // steady aspiration level at breathiness 0
                float tilt_hz_min  = 2600.f;    // source low-pass at brightness 0
                float tilt_hz_max  = 11000.f;   // ... at brightness 1

                // Vibrato defaults
                float vib_depth_st = 0.32f;     // semitones, peak
                float vib_rate_hz  = 5.7f;
                float vib_delay    = 0.30f;     // s before vibrato begins
                float vib_fade     = 0.35f;     // s to reach full depth

                // Timbre of the synthetic character
                float synthetic    = 0.65f;     // 0..1: tighter bandwidths, steadier source, brighter uppers

                // Consonant loci per place (labial, alveolar, palatal, velar): slope2, intercept2, slope3, intercept3
                // Fitted to real speech (V3): the engine's control track, read 6 ms after release the way F2/F3 onsets are
                // read from recordings, was matched to the per-vowel onset/vowel-formant ratios of one female Japanese
                // speaker (JSUT: loanword128 + onomatopee300 for the fit, voiceactress100 held out; about 4000 stop
                // tokens; see yune-v3/RESEARCH_NOTES.md). These are closure TARGETS, which is why they differ from the
                // measured onset equations. The palatal row was not measured and is hand-set.
                LocusEq loci[4] = { { 0.496f,  594.f, 0.054f, 2522.f },  // labial   (b p m f w)
                                    { 0.302f, 1385.f, 0.713f, 1174.f },  // alveolar (t d n s z r)
                                    { 0.25f,  2000.f, 0.40f,  2000.f },  // palatal  (ch j sh ky ...), not fitted
                                    { 0.928f,  129.f, 1.096f,  -37.f } };// velar    (k g)

                // Nasal consonants / ん: the zero (anti-formant) is a real notch; poles broaden and a low nasal pole appears.
                float nasal_zero_q     = 2.5f;     // notch Q (lower = wider)
                float nasal_zero_depth = 0.92f;    // 1 = full-depth notch at nasal = 1
                float nasal_pole_hz    = 270.f;    // extra low-frequency nasal resonance
                float nasal_pole_gain  = 1.6f;
                float nasal_bw_widen   = 0.9f;     // F1 bandwidth growth at nasal = 1 (F2/F3 get a fraction)

                // How much bigger/smaller the tract is than the reference (Yune). Used to scale locus intercepts.
                float tract_scale() const { return 0.5f * (vowel[0].f2 / 1480.f + vowel[4].f2 / 980.f); }
            };

            inline const VoiceModel& default_voice() { static const VoiceModel v; return v; }

            // A control curve (time, value). Time units depend on use (see Singer docs).
            using Curve = std::vector<std::pair<double, double>>;

            inline double curve_eval(const Curve& c, double t, double outside = 0.0) {
                if (c.empty()) return outside;
                if (t <= c.front().first) return c.front().second;
                if (t >= c.back().first)  return c.back().second;
                size_t lo = 0, hi = c.size() - 1;
                while (hi - lo > 1) { size_t mid = (lo + hi) / 2; (c[mid].first <= t ? lo : hi) = mid; }
                const double t0 = c[lo].first, t1 = c[hi].first;
                const double f = t1 > t0 ? (t - t0) / (t1 - t0) : 0.0;
                return c[lo].second + (c[hi].second - c[lo].second) * f;
            }

            // Sum of two curves (knots merged), used when several sources bend the same note.
            inline Curve curve_add(const Curve& a, const Curve& b) {
                if (a.empty()) return b;
                if (b.empty()) return a;
                std::vector<double> ts;
                for (auto& k : a) ts.push_back(k.first);
                for (auto& k : b) ts.push_back(k.first);
                std::sort(ts.begin(), ts.end());
                ts.erase(std::unique(ts.begin(), ts.end()), ts.end());
                Curve o;
                for (double t : ts) o.push_back({ t, curve_eval(a, t) + curve_eval(b, t) });
                return o;
            }

            // Musical controls. All 0..1 unless a unit is given. Every field can be set as a
            // singer-wide default and overridden per note.
            struct Style {
                // voice
                float brightness  = 0.70f;   // high-frequency presence (source tilt + upper formants)
                float breathiness = 0.12f;   // aspiration + more open glottis
                float tension     = 0.50f;   // glottal tenseness (low Rd -> sharper, brighter)
                float softness    = 0.00f;   // gentle, rounded onset and breathier tone
                float power       = 0.50f;   // vocal effort: singer's formant + tenser source
                float resonance   = 0.60f;   // formant sharpness (narrower bandwidths)
                float femininity  = 0.50f;   // 0.5 = model default; shifts formants by about +-6%

                // pitch
                float vibrato_depth = -1.f;  // semitones peak; <0 = voice default
                float vibrato_rate  = -1.f;  // Hz; <0 = voice default
                float vibrato_delay = -1.f;  // s before onset; <0 = voice default
                float vibrato_fade  = -1.f;  // s fade-in;     <0 = voice default
                float portamento    = 0.060f;   // s glide between connected notes
                float pitch_response= 0.55f;   // 0..1 how fast pitch settles on target
                float overshoot     = 0.20f;   // 0..1 pitch overshoot (0 = none)
                float scoop         = 0.30f;   // semitones: note-entry slide from below after a rest
                float exit_fall     = 0.20f;   // semitones: pitch sag at the end of a phrase-final note
                float drift         = 0.00f;   // semitones: slow deterministic drift
                float micro         = 0.00f;   // 0..1 tiny deterministic microvariation of vibrato

                // articulation
                float consonant_strength = 0.55f;
                float consonant_duration = 1.00f;   // scale on consonant length
                float vowel_clarity      = 0.60f;
                float legato             = 0.50f;   // 1 = notes run together
                float attack_sharpness   = 0.50f;
                float release            = 0.50f;   // 0 = abrupt, 1 = long fade
                bool  devoicing          = true;    // devoice short i/u between voiceless consonants

                // dynamics
                float volume            = 1.00f;
                float velocity_response = 0.65f;
                float compression       = 0.30f;
                float dynamic_timbre    = 0.50f;    // louder -> brighter/tenser, softer -> breathier

                // ---- V2 expression controls. Every default is "off": the default Style renders exactly as V1.
                float formant_shift = 0.f;   // semitones: moves F1-F3 without changing pitch (gender/size axis)
                float mouth_open    = 0.f;   // -1..1: jaw/mouth opening (raises/lowers F1)
                float growl         = 0.f;   // 0..1: sub-harmonic roughness
                float tremor        = 0.f;   // 0..1: shaky voice (pitch + amplitude)
                float tremor_rate   = 9.f;   // Hz
                float fry           = 0.f;   // 0..1: vocal fry / creak (period doubling)
                float air           = 0.f;   // 0..1: extra unfiltered breath "air"
                float reg           = 0.f;   // register: 0 chest/modal .. 1 falsetto/head
                float reg_auto      = 0.f;   // 0..1: let pitch move the register (passaggio B4..G5)
                float glottal_onset = 0.f;   // 0..1: hard glottal onset on each note
                float pitch_loud    = 0.f;   // semitones per unit of (velocity-0.5): louder sings sharper
                float pitch_snap    = 0.f;   // 0..1: robotic stepping of glides toward semitones
                float undershoot    = 0.f;   // 0..1: fast notes fall short of their target pitch
                float porta_interval= 0.f;   // 0..1: wider intervals glide longer
                float exit_fall_min = 0.20f; // s: shortest note that still gets the phrase-end sag
                float bidakuon      = 0.f;   // 0..1: word-medial ガ行 as nasal [ŋ] (standard singing/speech)
                float n_nasalize    = 1.f;   // 0..1: ん before a vowel/glide becomes a nasalised vowel instead of a closure (V3: on by default)
                float mumble        = 0.f;   // -1 crisp .. +1 mumble: vowel centralisation + weaker consonants
                float transition_speed = 1.f;// scale on vowel/consonant transition times (>1 snappier, <1 smoother)
                float vv_glide      = 0.f;   // 0..1: diphthong-aware vowel->vowel glides (longer, F2 leads F1)
                float devoice_max   = 0.11f; // s: longest i/u that may devoice between voiceless consonants
                float mora_weighting= 1.f;   // 0..1: weight mora time by type (ん/っ shorter, ー longer); 0 = even split (V1). V3: on by default
                float glottal_final = 0.f;   // 0..1: creak + abrupt stop before a final っ (あっ!)
                float grit          = 0.f;   // 0..1: roughness that appears only when sung hard (velocity above ~0.65)
                float natural       = 0.f;   // 0..1: living-voice imperfection: cycle jitter/shimmer, slow pitch/level/breath/formant wander, irregular vibrato

                // ---- V3 ----
                float vib_am         = 0.35f; // 0..1: amplitude vibrato coupled to the pitch vibrato (about 1.2 dB peak-to-peak at 0.25 st depth; a real singer measured 1.26 dB)
                float vib_shape      = 0.f;   // 0..1: 0 = sine, 1 = clearly skewed (slow up / fast down) vibrato wave. A real singer measured symmetric (0.500), so 0
                float vib_accel      = 0.05f; // 0..1: vibrato rate speeds up by this fraction over the first ~1.2 s of a note
                float coarticulation = 0.6f;  // 0..1: vowels lean toward the next vowel before it arrives (across consonants too)
                float medial_vot     = 0.f;   // 0..1: how much voiceless stops that follow a vowel directly get the SHORT (speech) VOT. 0 = sung: every syllable onset
                                              //       keeps the full VOT (a real singer measured 33.7 ms between vowels, same as word-initial speech); say() uses 1
                float vc_transition  = 1.f;   // 0..1: the vowel starts moving toward the next consonant's locus about 8 ms before the closure
                                              //       (measured: real speech shows this in the last ~10 ms of the vowel, mostly after back vowels); 0 = V2
                float prevoice_initial = 0.f; // 0..1: voicing murmur before a voiced stop that starts after a rest. 0 = none, with a short
                                              //       voicing lag after the release (as in measured word-initial /b d g/); 1 = the V2 behaviour
            };

            // Per-note override container: the effective Style for the note is already
            // baked in at note creation time; these are note-specific extras.
            struct NoteExtras {
                Curve bend;          // semitones vs seconds from note start
                double glide_from = -1.0;  // explicit portamento source pitch (MIDI), <0 = auto
            };

            // Name -> Style member, shared by score files, persona files and lanes.
            inline float Style::* style_field(const std::string& k) {
                static const std::map<std::string, float Style::*> m = {
                    {"brightness",&Style::brightness},{"breathiness",&Style::breathiness},{"tension",&Style::tension},
                    {"softness",&Style::softness},{"power",&Style::power},{"resonance",&Style::resonance},{"femininity",&Style::femininity},
                    {"vibrato_depth",&Style::vibrato_depth},{"vibrato_rate",&Style::vibrato_rate},{"vibrato_delay",&Style::vibrato_delay},
                    {"vibrato_fade",&Style::vibrato_fade},{"portamento",&Style::portamento},{"pitch_response",&Style::pitch_response},
                    {"overshoot",&Style::overshoot},{"scoop",&Style::scoop},{"exit_fall",&Style::exit_fall},{"drift",&Style::drift},
                    {"micro",&Style::micro},{"consonant_strength",&Style::consonant_strength},{"consonant_duration",&Style::consonant_duration},
                    {"vowel_clarity",&Style::vowel_clarity},{"legato",&Style::legato},{"attack_sharpness",&Style::attack_sharpness},
                    {"release",&Style::release},{"volume",&Style::volume},{"velocity_response",&Style::velocity_response},
                    {"compression",&Style::compression},{"dynamic_timbre",&Style::dynamic_timbre},
                    {"formant_shift",&Style::formant_shift},{"mouth_open",&Style::mouth_open},{"growl",&Style::growl},
                    {"tremor",&Style::tremor},{"tremor_rate",&Style::tremor_rate},{"fry",&Style::fry},{"air",&Style::air},
                    {"reg",&Style::reg},{"register",&Style::reg},{"reg_auto",&Style::reg_auto},{"glottal_onset",&Style::glottal_onset},
                    {"pitch_loud",&Style::pitch_loud},{"pitch_snap",&Style::pitch_snap},{"undershoot",&Style::undershoot},
                    {"porta_interval",&Style::porta_interval},{"exit_fall_min",&Style::exit_fall_min},
                    {"bidakuon",&Style::bidakuon},{"n_nasalize",&Style::n_nasalize},{"mumble",&Style::mumble},
                    {"transition_speed",&Style::transition_speed},{"vv_glide",&Style::vv_glide},{"devoice_max",&Style::devoice_max},
                    {"mora_weighting",&Style::mora_weighting},{"glottal_final",&Style::glottal_final},{"grit",&Style::grit},{"natural",&Style::natural},{"human",&Style::natural},
                    {"vib_am",&Style::vib_am},{"vib_shape",&Style::vib_shape},{"vib_accel",&Style::vib_accel},{"coarticulation",&Style::coarticulation},{"prevoice_initial",&Style::prevoice_initial},{"vc_transition",&Style::vc_transition},{"medial_vot",&Style::medial_vot} };
                auto it = m.find(k); return it == m.end() ? nullptr : it->second;
            }

            // Deterministic smooth noise in [-1,1]: value noise sampled at `rate` Hz (cubic-eased). Same (seed, t) = same value.
            inline float vnoise(uint32_t seed, double t, double rate) {
                auto h = [seed](int64_t i) { uint32_t x = static_cast<uint32_t>(i) * 0x9E3779B1u ^ seed * 0x85EBCA6Bu; x ^= x >> 15; x *= 0x2C1B3C6Du; x ^= x >> 12; x *= 0x297A2D39u; x ^= x >> 15;
                                             return static_cast<float>(x & 0xFFFFFF) / 8388608.f - 1.f; };
                const double x = t * rate, fl = std::floor(x); const int64_t i = static_cast<int64_t>(fl);
                double f = x - fl; f = f * f * (3.0 - 2.0 * f);
                return h(i) + (h(i + 1) - h(i)) * static_cast<float>(f);
            }

            inline float clamp01(float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); }

        }
    }
}

#!/usr/bin/env python3
"""Generates the additional genre presets, their MIDI pairings, both registries and the extra drum kits.

Edit the table of g(...) entries below and run `python3 gen_genres.py` to add or change a genre. Outputs (all under wauvio/):
  ext/genre/genre_presets_extra.hpp, ext/genre/genre_registry.hpp, midi/genre/genre_midi_presets_extra.hpp,
  midi/genre/genre_midi_registry.hpp, ext/instruments/percussion/kits_extra.hpp
Then run ext/tools/gen_catalog.py if you added kit classes.
"""
import pathlib, re
R = pathlib.Path(__file__).resolve().parents[2]   # the wauvio/ folder
ART = lambda a: 'wauvio::music::Articulation::' + a

# ---------------- profile dict -> C++ ----------------
def f(x): 
    s = ('%.4f' % x).rstrip('0').rstrip('.')
    return s if ('.' in s) else s + '.0'
def prof_lines(var, d, fl=False):
    L = []
    def a(s): L.append('%s.%s' % (var, s))
    for k, v in d.items():
        if   k == 'vel':   a('velocity_scale = %s;' % f(v))
        elif k == 'floor': a('velocity_floor = %s;' % f(v))
        elif k == 'var':   a('velocity_variance = %s;' % f(v))
        elif k == 'accent':a('accent_pattern = {%s};' % ', '.join(f(x) for x in v))
        elif k == 'swing': a('swing_amount = %s;' % f(v))
        elif k == 'sub':   a('swing_subdivision = %d;' % v)
        elif k == 'human': a('timing_humanize_sec = %s;' % f(v))
        elif k == 'quant': a('quantize_grid_fraction = %s;' % f(v))
        elif k == 'offset':a('timing_offset_sec = %s;' % f(v))
        elif k == 'down':  a('downbeat_deemphasis = %s;' % f(v))
        elif k == 'thin':  a('thin_probability = %s;' % f(v))
        elif k == 'shift': a('register_shift_semitones = %d;' % v)
        elif k == 'art':   a('force_articulation = true;'); a('articulation = %s;' % ART(v))
        elif k == 'gate_fb': a('fallback_gate_ratio = %s;' % f(v))
        elif k == 'gate':  a('gate_ratio = %s;' % f(v))
        elif k == 'compress': a('velocity_compress = %s;' % f(v))
        elif k == 'cut':   a('filter_cutoff_mult = %s;' % f(v))
        elif k == 'detune':a('detune_cents_delta = %s;' % f(v))
        elif k == 'width': a('stereo_width_override = %s;' % f(v))
        elif k == 'noise': a('noise_mix_delta = %s;' % f(v))
        elif k == 'wobble':a('wobble_amount = %s;' % f(v))
        elif k == 'reverb':a('want_reverb = true;'); a('reverb_room = %sf;' % f(v[0])); a('reverb_wet = %sf;' % f(v[1])); a('reverb_damping = %sf;' % f(v[2] if len(v) > 2 else 0.5))
        elif k == 'chorus':a('want_chorus = true;'); a('chorus_rate_hz = %s;' % f(v[0])); a('chorus_depth_ms = %s;' % f(v[1])); a('chorus_wet = %sf;' % f(v[2]))
        elif k == 'delay': a('want_delay = true;'); a('delay_time_ms = %s;' % f(v[0])); a('delay_feedback = %sf;' % f(v[1])); a('delay_wet = %sf;' % f(v[2]))
        elif k == 'dist':  a('want_distortion = true;'); a('distortion_drive = %sf;' % f(v))
        elif k == 'dist_fam': a('distortion_family = wauvio::music::genre::Family::%s;' % v)
        elif k == 'gt':    a('groove_timing = {%s};' % ', '.join(f(x) for x in v))
        elif k == 'gv':    a('groove_velocity = {%s};' % ', '.join(f(x) for x in v))
        elif k == 'no_reverb': a('want_reverb = false;')
        elif k == 'no_dist':   a('want_distortion = false;')
        elif k == 'no_delay':  a('want_delay = false;')
        else: raise KeyError(k)
    return L

G = []   # genre specs
def g(id, name, family, desc, bpm, tempo, d, bass=None, perc=None, aliases=(), kit=None, progs=None):
    G.append(dict(id=id, name=name, family=family, desc=desc, bpm=bpm, tempo=tempo, d=d, bass=bass, perc=perc, aliases=list(aliases), kit=kit, progs=progs or {}))

GUITAR_DIST = lambda x: dict(dist=x, dist_fam='Guitar')
# ===================== Pop & Rock =====================
g('pop','Pop','Pop & Rock','Clean, quantized, compressed and bright with a polished hall.',(95,130),1.0,
  dict(human=0.004,var=0.06,accent=[1,.8,.92,.8],quant=0.25,compress=0.35,reverb=(0.35,0.2,0.5)),
  bass=dict(cut=0.9), kit='StudioKit')
g('punk','Punk','Pop & Rock','Fast, raw and tight: downstroke energy and little polish.',(160,200),1.12,
  dict(human=0.003,accent=[1,.9,.95,.9],art='Accent',gate_fb=0.8,**GUITAR_DIST(2.6)), kit='RockKit',
  progs={26:'OverdrivenElectricGuitar',27:'OverdrivenElectricGuitar',29:'OverdrivenElectricGuitar',30:'DistortedElectricGuitar'})
g('pop_punk','Pop Punk','Pop & Rock','Fast and bright with big choruses: driven but clean enough to sing over.',(150,190),1.08,
  dict(human=0.003,accent=[1,.88,.95,.88],art='Accent',gate_fb=0.85,reverb=(0.3,0.12,0.5),**GUITAR_DIST(1.8)),
  aliases=['pop-punk'], kit='RockKit', progs={27:'CleanElectricGuitar',29:'OverdrivenElectricGuitar'})
g('hard_rock','Hard Rock','Pop & Rock','Heavy riffs, big backbeat, a roomy amp sound.',(100,150),1.0,
  dict(human=0.005,accent=[.9,1,.9,1],art='Accent',gate_fb=0.9,reverb=(0.3,0.15,0.5),**GUITAR_DIST(2.8)),
  bass=dict(art='Marcato',no_dist=True), kit='RockKit',
  progs={26:'OverdrivenElectricGuitar',27:'OverdrivenElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar',18:'HammondOrgan'})
g('prog_rock','Progressive Rock','Pop & Rock','Long-form, expressive and spacious, with rich keyboards and strings.',(80,140),0.97,
  dict(human=0.007,var=0.1,accent=[1,.8,.9,.85],reverb=(0.45,0.22,0.45),chorus=(0.4,2.0,0.2)),
  aliases=['prog','progressive'], kit='RockKit', progs={48:'Mellotron',49:'Mellotron',4:'RhodesMarkI',81:'SawLead'})
g('grunge','Grunge','Pop & Rock','Sludgy, loose and loud-quiet: fuzzy guitars and a slightly dragging beat.',(90,130),0.95,
  dict(human=0.008,var=0.1,accent=[.9,1,.9,1],cut=0.85,reverb=(0.3,0.15,0.5),**GUITAR_DIST(3.0)),
  bass=dict(no_dist=True,cut=0.9), kit='RockKit', progs={26:'DistortedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar'})
g('indie_rock','Indie Rock','Pop & Rock','Jangly, a bit loose and lo-fi, with chorus and a roomy sound.',(100,150),1.0,
  dict(human=0.008,var=0.1,accent=[.95,1,.92,1],reverb=(0.4,0.25,0.5),chorus=(0.5,1.5,0.2),**GUITAR_DIST(1.5)),
  aliases=['indie','alternative'], kit='StudioKit', progs={27:'CleanElectricGuitar',29:'OverdrivenElectricGuitar'})
g('shoegaze','Shoegaze','Pop & Rock','Walls of reverb and chorus, washed-out and wide.',(80,130),0.9,
  dict(human=0.008,var=0.08,cut=0.8,width=0.9,reverb=(0.85,0.5,0.6),chorus=(0.4,3.0,0.45),**GUITAR_DIST(2.4)),
  kit='RockKit', progs={26:'DistortedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',89:'WarmPad'})
g('post_rock','Post-Rock','Pop & Rock','Slow builds, delay-soaked clean guitars and wide dynamics.',(70,130),0.95,
  dict(human=0.006,var=0.1,accent=[1,.85,.92,.85],reverb=(0.7,0.35,0.5),delay=(375.0,0.4,0.3)),
  kit='StudioKit', progs={27:'CleanElectricGuitar',26:'CleanElectricGuitar'})
g('emo','Emo','Pop & Rock','Earnest, driving and slightly raw, with chorus-tinged guitars.',(120,180),1.04,
  dict(human=0.005,var=0.08,accent=[1,.85,.95,.85],reverb=(0.3,0.14,0.5),chorus=(0.5,1.2,0.12),**GUITAR_DIST(1.9)),
  kit='RockKit', progs={27:'CleanElectricGuitar',29:'OverdrivenElectricGuitar'})
g('new_wave','New Wave','Pop & Rock','Angular, dry and synthetic, with chorus and a tight pulse.',(110,160),1.02,
  dict(human=0.002,quant=0.25,accent=[1,.85,.95,.85],chorus=(0.6,1.5,0.25),reverb=(0.25,0.1,0.5)),
  kit='VintageKit', progs={80:'SquareLead',81:'SawLead',38:'SynthBass',39:'SynthBass'})
g('surf_rock','Surf Rock','Pop & Rock','Drenched spring reverb, tremolo-bright guitars and a galloping beat.',(130,170),1.1,
  dict(human=0.005,accent=[.9,1,.9,1],reverb=(0.7,0.4,0.3),chorus=(5.5,0.8,0.3),cut=1.15),
  kit='VintageKit', progs={26:'CleanElectricGuitar',27:'CleanElectricGuitar'})
g('psychedelic_rock','Psychedelic Rock','Pop & Rock','Swirling chorus, long delays and a loose, dreamy pulse.',(90,140),0.95,
  dict(human=0.01,var=0.1,reverb=(0.6,0.3,0.4),chorus=(0.35,3.5,0.4),delay=(420.0,0.4,0.25),wobble=0.05),
  aliases=['psychedelia','psych'], kit='VintageKit', progs={16:'HammondOrgan',18:'FarfisaOrgan',104:'ElectricSitar'})
# ===================== Metal =====================
g('power_metal','Power Metal','Metal','Fast, melodic and bright: galloping rhythm and soaring leads.',(150,200),1.15,
  dict(human=0.002,quant=0.25,accent=[.9,1,.9,1],art='Accent',gate_fb=0.9,reverb=(0.3,0.15,0.5),**GUITAR_DIST(3.0)),
  bass=dict(no_dist=True,cut=0.9), kit='RockKit', progs={26:'DistortedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar',48:'StringEnsemble'})
g('thrash_metal','Thrash Metal','Metal','Very fast palm-muted chugging, aggressive and precise.',(170,230),1.2,
  dict(human=0.002,quant=0.25,accent=[1,.85,.9,.85],art='Accent',gate_fb=0.8,**GUITAR_DIST(3.4)),
  bass=dict(no_dist=True,cut=0.85), aliases=['thrash'], kit='RockKit', progs={26:'PalmMutedElectricGuitar',27:'PalmMutedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar'})
g('death_metal','Death Metal','Metal','Brutal low tunings, blast-beat precision and heavy distortion.',(160,240),1.2,
  dict(human=0.0015,quant=0.125,accent=[.9,1,.9,1],art='Accent',gate_fb=0.8,cut=0.85,**GUITAR_DIST(4.0)),
  bass=dict(no_dist=True,cut=0.75), kit='RockKit', progs={26:'DistortedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar'})
g('black_metal','Black Metal','Metal','Thin, icy tremolo guitars in a wash of cold reverb.',(140,220),1.2,
  dict(human=0.003,quant=0.125,accent=[1,.85,.9,.85],cut=1.15,reverb=(0.55,0.3,0.4),**GUITAR_DIST(3.6)),
  bass=dict(no_dist=True), kit='RockKit', progs={26:'DistortedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar'})
g('doom_metal','Doom Metal','Metal','Slow, massive and dragging: detuned weight and long decays.',(40,80),0.6,
  dict(human=0.01,var=0.08,accent=[1,.8,.9,.8],cut=0.8,reverb=(0.5,0.25,0.5),**GUITAR_DIST(3.4)),
  bass=dict(no_dist=True,cut=0.7), kit='RockKit', progs={26:'DistortedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar'})
g('metalcore','Metalcore','Metal','Chugging breakdowns and melodic leads, tightly quantized.',(100,190),1.05,
  dict(human=0.0015,quant=0.25,accent=[1,.8,.9,.8],art='Accent',gate_fb=0.8,**GUITAR_DIST(3.5)),
  bass=dict(no_dist=True,cut=0.8), kit='RockKit', progs={26:'PalmMutedElectricGuitar',27:'DistortedElectricGuitar',29:'DistortedElectricGuitar',30:'DistortedElectricGuitar'})
# ===================== Roots & Folk =====================
g('country','Country','Roots & Folk','Easy train-beat rhythm, bright twang and a dry-ish room.',(90,140),1.0,
  dict(human=0.008,var=0.08,swing=0.12,accent=[1,.8,.95,.8],gate_fb=0.9,reverb=(0.3,0.15,0.5)),
  bass=dict(art='Staccato',gate_fb=0.85), kit='StudioKit', progs={24:'SteelStringGuitar',25:'SteelStringGuitar',26:'CleanElectricGuitar',27:'CleanElectricGuitar',110:'FolkFiddle',105:'Banjo'})
g('bluegrass','Bluegrass','Roots & Folk','Fast acoustic picking with offbeat mandolin chops.',(120,190),1.15,
  dict(human=0.006,var=0.1,accent=[.8,1,.8,1],art='Staccato',gate_fb=0.85,reverb=(0.25,0.1,0.5)),
  bass=dict(art='Staccato'), kit='FolkKit', progs={24:'SteelStringGuitar',25:'SteelStringGuitar',105:'Banjo',110:'FolkFiddle',32:'UprightBassPluck',33:'UprightBassPluck'})
g('folk','Folk','Roots & Folk','Loose, warm and human: acoustic instruments in a small room.',(70,120),0.95,
  dict(human=0.01,var=0.1,accent=[1,.8,.9,.8],reverb=(0.4,0.2,0.5)),
  kit='FolkKit', progs={24:'SteelStringGuitar',25:'SteelStringGuitar',110:'FolkFiddle',22:'Harmonica'})
g('celtic','Celtic','Roots & Folk','Lilting dance rhythms with fiddle, whistle and pipes.',(90,170),1.1,
  dict(human=0.008,var=0.08,swing=0.2,accent=[1,.8,.9,.8],reverb=(0.4,0.2,0.5)),
  aliases=['irish'], kit='FolkKit', progs={110:'FolkFiddle',109:'GreatHighlandBagpipe',73:'IrishWhistle',24:'SteelStringGuitar',25:'SteelStringGuitar'})
g('gospel','Gospel','Roots & Folk','Swung, soulful backbeat with organ and a big room.',(70,130),1.0,
  dict(human=0.01,var=0.1,swing=0.3,accent=[.9,1,.9,1],reverb=(0.5,0.25,0.5),compress=0.2),
  kit='StudioKit', progs={16:'HammondOrgan',17:'HammondOrgan',18:'HammondOrgan',19:'ChurchOrgan'})
g('ska','Ska','Roots & Folk','Upbeat skank with a walking bass and punchy horns.',(150,200),1.2,
  dict(human=0.004,accent=[.7,1,.7,1],down=0.35,gate_fb=0.8,reverb=(0.25,0.1,0.5)),
  bass=dict(down=0.0,accent=[1,.9,1,.9]), kit='StudioKit', progs={56:'Trumpet',57:'Trombone',65:'AltoSaxophone',66:'TenorSaxophone',27:'CleanElectricGuitar'})
g('rockabilly','Rockabilly','Roots & Folk','Shuffling slapback groove with an upright bass.',(120,200),1.1,
  dict(human=0.01,var=0.1,swing=0.45,accent=[.9,1,.9,1],reverb=(0.3,0.15,0.5),delay=(120.0,0.15,0.3)),
  kit='VintageKit', progs={26:'CleanElectricGuitar',27:'CleanElectricGuitar',32:'UprightBassPluck',33:'UprightBassPluck'})
g('polka','Polka','Roots & Folk','Bouncy oom-pah two-beat with accordion.',(110,150),1.2,
  dict(human=0.006,accent=[1,.6,.9,.6],art='Staccato',gate_fb=0.8,reverb=(0.3,0.12,0.5)),
  bass=dict(art='Staccato'), kit='FolkKit', progs={21:'Accordion',23:'Accordion',58:'Tuba',110:'FolkFiddle'})
# ===================== Jazz & Blues =====================
g('swing','Swing','Jazz & Blues','Big-band swing: a strong two-and-four and a walking bass.',(120,200),1.05,
  dict(swing=0.62,human=0.01,var=0.1,accent=[.85,1,.85,1],reverb=(0.35,0.15,0.5)),
  aliases=['big band','bigband'], kit='JazzKit', progs={56:'Trumpet',57:'Trombone',65:'AltoSaxophone',66:'TenorSaxophone',67:'BaritoneSaxophone',32:'UprightBassPluck',33:'UprightBassPluck'})
g('bebop','Bebop','Jazz & Blues','Fast, angular and heavily swung with loose small-combo timing.',(180,300),1.3,
  dict(swing=0.6,human=0.01,var=0.12,accent=[.85,1,.85,1],reverb=(0.3,0.12,0.5)),
  perc=dict(thin=0.1), kit='JazzKit', progs={32:'UprightBassPluck',33:'UprightBassPluck',65:'AltoSaxophone',66:'TenorSaxophone',56:'Trumpet'})
g('latin_jazz','Latin Jazz','Jazz & Blues','Afro-Cuban straight eighths with jazz harmony and syncopation.',(110,220),1.1,
  dict(human=0.008,var=0.08,accent=[1,.8,.9,.8],reverb=(0.35,0.15,0.5)),
  kit='LatinKit', progs={32:'UprightBassPluck',33:'UprightBassPluck',56:'Trumpet',65:'AltoSaxophone'})
g('smooth_jazz','Smooth Jazz','Jazz & Blues','Glossy, relaxed and polished, with warm electric keys.',(70,115),0.9,
  dict(swing=0.2,human=0.006,var=0.06,compress=0.3,reverb=(0.5,0.25,0.5),chorus=(0.4,2.0,0.2)),
  kit='StudioKit', progs={4:'RhodesMarkI',5:'RhodesMarkII',65:'SopranoSaxophone',66:'TenorSaxophone',27:'CleanElectricGuitar'})
# ===================== Soul, Funk & R&B =====================
g('soul','Soul','Soul, Funk & R&B','Deep pocket, warm organ and rhythm-section feel.',(70,120),0.95,
  dict(swing=0.25,human=0.01,var=0.08,accent=[.9,1,.9,1],reverb=(0.35,0.2,0.5)),
  kit='StudioKit', progs={16:'HammondOrgan',4:'Wurlitzer200A',5:'Wurlitzer200A'})
g('motown','Motown','Soul, Funk & R&B','Tambourine-driven four-on-the-snare with a melodic bass.',(100,130),1.0,
  dict(swing=0.1,human=0.007,accent=[.8,1,.8,1],compress=0.3,reverb=(0.4,0.2,0.4)),
  kit='VintageKit', progs={4:'Wurlitzer200A',33:'PickBass',34:'PickBass',48:'StringEnsemble'})
g('disco','Disco','Soul, Funk & R&B','Four on the floor, octave bass, strings and a shimmering hi-hat.',(110,130),1.05,
  dict(swing=0.05,human=0.002,quant=0.25,accent=[1,.85,.95,.85],compress=0.3,reverb=(0.3,0.15,0.5),chorus=(0.5,1.5,0.15)),
  bass=dict(art='Staccato',gate_fb=0.8,no_reverb=True), perc=dict(accent=[1,.7,.9,.7],no_reverb=True), kit='StudioKit',
  progs={36:'SlapBass',37:'SlapBass',48:'StringEnsemble',61:'BrassSection'})
g('rnb','R&B','Soul, Funk & R&B','Smooth, laid-back and polished, with a late snare and a deep bass.',(60,100),0.9,
  dict(swing=0.15,sub=4,human=0.004,quant=0.25,compress=0.4,reverb=(0.35,0.2,0.5)),
  bass=dict(cut=0.85), perc=dict(gt=[0,0,0,0, 0,0,0,0.12, 0,0,0,0, 0,0,0.1,0],human=0.0), aliases=['rnb','rhythm and blues','contemporary r&b'],
  kit='Kit808', progs={4:'RhodesMarkII',5:'RhodesMarkII',38:'SubBass',39:'SubBass',89:'WarmPad'})
# ===================== Hip-Hop =====================
g('hip_hop','Hip-Hop','Hip-Hop','Boom-bap: swung sixteenths, a heavy kick and a slightly lazy snare.',(80,100),0.92,
  dict(swing=0.4,sub=4,human=0.006,var=0.08,compress=0.3,cut=0.85,reverb=(0.25,0.12,0.5)),
  bass=dict(cut=0.8), perc=dict(gt=[0,0,0,0, 0,0,0,0.1, 0,0,0,0, 0,0,0,0.1],var=0.1), aliases=['hiphop','hip hop','boom bap','rap'],
  kit='VintageKit', progs={38:'SubBass',39:'SubBass',33:'SubBass'})
g('trap','Trap','Hip-Hop','Half-time feel, rolling hi-hats and a booming 808 bass.',(130,170),0.85,
  dict(quant=0.125,human=0.0,accent=[1,.7,.85,.7],compress=0.4,reverb=(0.3,0.15,0.5)),
  bass=dict(cut=0.7,dist=1.4,dist_fam='Synth'), perc=dict(var=0.18,gv=[1,.7,.85,.7,.95,.7,.85,.7,1,.7,.85,.7,.95,.7,.85,.7]),
  kit='TrapKit', progs={38:'SubBass',39:'SubBass',33:'SubBass',80:'SquareLead',81:'SawLead'})
g('lofi_hip_hop','Lo-fi Hip-Hop','Hip-Hop','Dusty, dull and wobbly: swung, loose and warm.',(70,90),0.85,
  dict(swing=0.5,sub=4,human=0.012,var=0.12,cut=0.6,noise=0.02,wobble=0.06,reverb=(0.45,0.25,0.6),chorus=(0.3,3.0,0.2)),
  aliases=['lofi','lo-fi','lofi hip hop','chillhop'], kit='LoFiKit', progs={0:'FeltPiano',1:'FeltPiano',4:'RhodesMarkI',5:'RhodesMarkI',33:'SubBass'})
# ===================== Electronic =====================
g('uk_garage','UK Garage','Electronic','Skippy shuffled two-step with a bouncy bass.',(128,140),1.05,
  dict(swing=0.45,sub=4,human=0.002,quant=0.25,reverb=(0.3,0.15,0.5)),
  bass=dict(cut=1.0), aliases=['garage','2-step','two step','2step'], kit='ElectronicKit', progs={38:'SubBass',39:'SubBass',33:'SubBass'})
g('breakbeat','Breakbeat','Electronic','Chopped, swung breaks with a lively, human-sounding kit.',(120,140),1.05,
  dict(swing=0.35,sub=4,human=0.004,var=0.1,accent=[1,.8,.9,.8],reverb=(0.25,0.12,0.5)),
  perc=dict(var=0.14), aliases=['breaks'], kit='VintageKit', progs={38:'SynthBass',39:'SynthBass'})
g('jungle','Jungle','Electronic','Frantic chopped breaks over a deep, rolling sub.',(160,180),1.3,
  dict(swing=0.3,sub=4,human=0.003,var=0.1,reverb=(0.3,0.15,0.5)),
  bass=dict(cut=0.7), perc=dict(var=0.14), kit='ElectronicKit', progs={38:'ReeseBass',39:'SubBass',33:'SubBass'})
g('electro','Electro','Electronic','Robotic 808 grooves with vocoder-era synth stabs.',(110,135),1.0,
  dict(quant=0.25,human=0.0,accent=[1,.8,.9,.8],chorus=(0.5,1.5,0.2),delay=(300.0,0.3,0.2)),
  kit='Kit808', progs={38:'SynthBass',39:'SynthBass',80:'SawLead',81:'SawLead'})
g('synthwave','Synthwave','Electronic','Neon retro: gated reverb, chorus-drenched pads and a pulsing bass.',(80,118),0.95,
  dict(quant=0.25,human=0.0,accent=[1,.85,.95,.85],reverb=(0.6,0.35,0.5),chorus=(0.8,2.5,0.35),delay=(400.0,0.35,0.3)),
  bass=dict(cut=1.1,no_delay=True), aliases=['outrun','retrowave'], kit='ElectronicKit', progs={38:'SynthBass',39:'SynthBass',80:'SawLead',81:'SupersawLead',89:'PolySynth',90:'PolySynth'})
g('vaporwave','Vaporwave','Electronic','Slowed, washed-out and hazy, like a tape left in the sun.',(60,90),0.7,
  dict(swing=0.1,human=0.004,cut=0.7,width=0.9,reverb=(0.75,0.5,0.6),chorus=(0.3,3.0,0.45)),
  kit='LoFiKit', progs={4:'RhodesMarkI',5:'RhodesMarkI',89:'WarmPad',90:'WarmPad'})
g('synthpop','Synthpop','Electronic','Catchy, clean synth lines and a crisp drum machine.',(105,130),1.05,
  dict(quant=0.25,human=0.0,accent=[1,.85,.95,.85],compress=0.35,chorus=(0.5,1.5,0.25),reverb=(0.4,0.2,0.5)),
  kit='Kit808', progs={81:'SawLead',38:'SynthBass',39:'SynthBass',90:'PolySynth'})
g('italo_disco','Italo Disco','Electronic','Arpeggiating bass, bright synths and a drum-machine four-on-the-floor.',(115,130),1.08,
  dict(quant=0.25,human=0.0,accent=[1,.85,.95,.85],chorus=(0.6,1.8,0.3),delay=(280.0,0.3,0.2),reverb=(0.35,0.18,0.5)),
  kit='ElectronicKit', progs={38:'SynthBass',39:'SynthBass',80:'SquareLead',81:'SawLead'})
g('eurobeat','Eurobeat','Electronic','Relentless, bright and fast: octave-bouncing bass and screaming synth leads.',(150,170),1.35,
  dict(quant=0.25,human=0.0,accent=[1,.8,.95,.8],compress=0.4,cut=1.2,reverb=(0.3,0.15,0.5)),
  bass=dict(cut=1.0,art='Staccato',gate_fb=0.85), kit='Kit909', progs={33:'SynthBass',38:'SynthBass',39:'SynthBass',80:'SupersawLead',81:'SupersawLead'})
g('future_bass','Future Bass','Electronic','Wide, bright supersaw chords with swung, sidechain-like bounce.',(140,160),0.95,
  dict(swing=0.2,sub=4,quant=0.25,human=0.0,compress=0.4,detune=10.0,width=0.85,reverb=(0.5,0.3,0.5),chorus=(0.5,2.5,0.3)),
  kit='TrapKit', progs={80:'SupersawLead',81:'SupersawLead',89:'SupersawLead',38:'SubBass',39:'SubBass'})
g('big_room','Big Room EDM','Electronic','Festival EDM: huge drops, compressed and very wide.',(126,132),1.05,
  dict(quant=0.25,human=0.0,accent=[1,.8,.95,.8],compress=0.5,detune=10.0,width=0.9,reverb=(0.45,0.25,0.5)),
  aliases=['edm','big room','festival'], kit='Kit909', progs={80:'SupersawLead',81:'SupersawLead',38:'SynthBass',39:'SynthBass'})
g('psytrance','Psytrance','Electronic','Driving rolling bass and twisting, hypnotic leads.',(140,150),1.2,
  dict(quant=0.125,human=0.0,accent=[1,.8,.9,.8],wobble=0.1,reverb=(0.4,0.2,0.5)),
  bass=dict(cut=0.8,wobble=0.0), aliases=['psy','goa'], kit='Kit909', progs={38:'SynthBass',39:'SynthBass',80:'SawLead'})
g('happy_hardcore','Happy Hardcore','Electronic','Frantic, euphoric and bright: piano stabs over a pounding kick.',(160,190),1.5,
  dict(quant=0.125,human=0.0,accent=[1,.75,.9,.75],cut=1.15,reverb=(0.3,0.15,0.5)),
  aliases=['hardcore rave','rave'], kit='Kit909', progs={80:'SupersawLead',81:'SupersawLead',38:'SynthBass',39:'SynthBass'})
g('industrial','Industrial','Electronic','Cold, mechanical and abrasive: distortion and metallic noise.',(90,140),1.0,
  dict(quant=0.25,human=0.0,accent=[1,.8,.9,.8],noise=0.03,reverb=(0.45,0.3,0.4),dist=2.8),
  kit='IndustrialKit', progs={38:'SynthBass',39:'SynthBass',80:'SawLead'})
g('downtempo','Downtempo','Electronic','Slow, hazy and deep, for chilling out.',(60,100),0.8,
  dict(swing=0.15,human=0.008,cut=0.8,reverb=(0.6,0.35,0.5),chorus=(0.3,2.5,0.25)),
  aliases=['chillout','chill','trip hop','trip-hop'], kit='LoFiKit', progs={4:'RhodesMarkI',89:'WarmPad',38:'SubBass'})
g('idm','IDM / Glitch','Electronic','Fractured, intricate rhythms with stuttering delays.',(90,160),1.0,
  dict(quant=0.125,human=0.0,thin=0.2,var=0.2,reverb=(0.4,0.2,0.5),delay=(160.0,0.45,0.3)),
  aliases=['idm','glitch','intelligent dance music'], kit='ElectronicKit', progs={38:'SynthBass',80:'SquareLead'})
# ===================== Orchestral =====================
g('baroque','Baroque','Orchestral','Terraced dynamics, crisp articulation and a church acoustic.',(70,140),1.0,
  dict(human=0.006,var=0.06,accent=[1,.7,.85,.7],art='Staccato',gate_fb=0.85,reverb=(0.55,0.25,0.4)),
  kit='OrchestralPercussionKit', progs={0:'Harpsichord',1:'Harpsichord',6:'Harpsichord',19:'ChurchOrgan',7:'Clavichord'})
g('romantic','Romantic','Orchestral','Rubato, wide dynamic swells and a warm hall.',(50,120),0.95,
  dict(human=0.014,var=0.12,accent=[1,.75,.9,.75],art='Legato',gate_fb=0.98,reverb=(0.62,0.3,0.4)),
  kit='OrchestralPercussionKit')
g('film_score','Film Score','Orchestral','Cinematic and wide: long phrases, big reverb and soft attacks.',(60,130),0.9,
  dict(human=0.008,var=0.08,accent=[1,.8,.9,.8],art='Legato',gate_fb=0.98,width=0.8,reverb=(0.7,0.4,0.4)),
  aliases=['cinematic','soundtrack','score'], kit='OrchestralPercussionKit', progs={48:'FullStringSection',49:'FullStringSection',61:'BrassSection'})
g('epic_orchestral','Epic Orchestral','Orchestral','Trailer-style power: marcato strings, huge percussion and brass.',(80,140),1.0,
  dict(human=0.003,var=0.06,accent=[1,.8,.92,.8],art='Marcato',gate_fb=0.9,compress=0.3,width=0.85,reverb=(0.65,0.35,0.4)),
  perc=dict(accent=[1,.7,.9,.7],art='Accent'), aliases=['epic','trailer'], kit='OrchestralPercussionKit', progs={48:'FullStringSection',61:'BrassSection',60:'FrenchHornEnsemble'})
g('medieval','Medieval','Orchestral','Modal, drone-based and plain: lutes, flutes and hand drums.',(70,130),0.95,
  dict(human=0.012,var=0.1,accent=[1,.8,.9,.8],cut=0.85,reverb=(0.5,0.22,0.5)),
  aliases=['renaissance','early music'], kit='FolkKit', progs={24:'Lute',25:'Lute',73:'SopranoRecorder',74:'SopranoRecorder',109:'Crumhorn',110:'HurdyGurdy'})
g('march','March','Orchestral','Steady, accented two-beat for brass and snare.',(100,130),1.1,
  dict(human=0.002,quant=0.125,accent=[1,.7,.9,.7],art='Marcato',gate_fb=0.85,reverb=(0.3,0.12,0.5)),
  aliases=['marching'], kit='MarchingKit', progs={56:'Trumpet',57:'Trombone',58:'Tuba',60:'FrenchHorn',72:'Piccolo'})
g('waltz','Waltz','Orchestral','Gentle three-beat sway: a strong first beat then two light ones.',(80,180),1.0,
  dict(human=0.01,var=0.08,accent=[1,.65,.65],art='Legato',gate_fb=0.95,reverb=(0.45,0.2,0.5)),
  kit='OrchestralPercussionKit')
g('lullaby','Lullaby','Orchestral','Soft, slow and rocking, with music-box tones.',(50,90),0.7,
  dict(vel=0.75,human=0.01,var=0.06,cut=0.7,art='Legato',gate_fb=0.98,reverb=(0.5,0.3,0.5)),
  kit='OrchestralPercussionKit', progs={0:'MusicBox',8:'MusicBox',10:'MusicBox'})
# ===================== Latin & Caribbean =====================
g('salsa','Salsa','Latin & Caribbean','Driving clave-based syncopation with brass and congas.',(160,220),1.1,
  dict(human=0.008,var=0.08,accent=[1,.8,.9,.8],reverb=(0.3,0.15,0.5)),
  kit='LatinKit', progs={56:'Trumpet',57:'Trombone',32:'UprightBassPluck',33:'UprightBassPluck'})
g('samba','Samba','Latin & Caribbean','Busy sixteenth-note percussion with a lilting swing.',(90,110),1.15,
  dict(swing=0.2,sub=4,human=0.008,var=0.1,accent=[1,.8,.9,.8],reverb=(0.3,0.15,0.5)),
  kit='SambaKit', progs={24:'NylonGuitar',25:'NylonGuitar',26:'NylonGuitar'})
g('tango','Tango','Latin & Caribbean','Sharp, dramatic accents and sudden pauses.',(100,140),1.0,
  dict(human=0.01,var=0.1,accent=[1,.6,.8,.6],art='Marcato',gate_fb=0.85,reverb=(0.4,0.2,0.5)),
  kit='OrchestralPercussionKit', progs={21:'Bandoneon',23:'Bandoneon',40:'SoloViolin',24:'ClassicalGuitar'})
g('flamenco','Flamenco','Latin & Caribbean','Fiery rasgueado and a tight compás with handclap accents.',(90,200),1.1,
  dict(human=0.009,var=0.1,accent=[1,.7,.9,.7],gate_fb=0.85,reverb=(0.35,0.15,0.5)),
  kit='LatinKit', progs={24:'ClassicalGuitar',25:'ClassicalGuitar',26:'ClassicalGuitar'})
g('cumbia','Cumbia','Latin & Caribbean','A gentle, lopsided shuffle with güira-like percussion.',(80,100),1.0,
  dict(swing=0.08,human=0.008,accent=[1,.7,.9,.7],reverb=(0.3,0.12,0.5)),
  kit='LatinKit', progs={32:'UprightBassPluck',33:'UprightBassPluck'})
g('bachata','Bachata','Latin & Caribbean','Romantic, syncopated guitar with a bongo-and-güira bounce.',(110,140),1.0,
  dict(human=0.007,var=0.08,accent=[1,.8,.9,.8],reverb=(0.3,0.15,0.5),chorus=(0.5,1.5,0.15)),
  kit='LatinKit', progs={24:'SteelStringGuitar',25:'SteelStringGuitar',26:'CleanElectricGuitar',27:'CleanElectricGuitar'})
g('dancehall','Dancehall','Latin & Caribbean','Sparse, bass-heavy digital riddims.',(85,105),0.9,
  dict(quant=0.25,human=0.0,accent=[1,.6,.8,.6],compress=0.4,reverb=(0.3,0.12,0.5)),
  bass=dict(cut=0.8), kit='Kit808', progs={38:'SubBass',39:'SubBass',33:'SubBass'})
g('reggaeton','Reggaeton','Latin & Caribbean','The dembow beat: a kick on every beat and a syncopated snare.',(88,100),0.95,
  dict(quant=0.25,human=0.0,accent=[1,.6,.9,.6],compress=0.4,reverb=(0.3,0.12,0.5)),
  bass=dict(cut=0.75), kit='TrapKit', progs={38:'SubBass',39:'SubBass',33:'SubBass'})
# ===================== World =====================
g('bollywood','Bollywood','World','Lush strings and sitar over lively dholak rhythm.',(90,150),1.05,
  dict(human=0.008,var=0.08,accent=[1,.8,.9,.8],reverb=(0.5,0.3,0.5),chorus=(0.4,2.0,0.25)),
  kit='IndianKit', progs={104:'Sitar',48:'FullStringSection',110:'Sarangi',111:'Shehnai'})
g('afrobeat','Afrobeat','World','Interlocking polyrhythms, a repeating bass vamp and horn riffs.',(100,130),1.05,
  dict(swing=0.12,sub=4,human=0.008,var=0.08,accent=[.9,1,.9,1],reverb=(0.3,0.12,0.5)),
  kit='AfroKit', progs={56:'Trumpet',65:'AltoSaxophone',66:'TenorSaxophone',27:'CleanElectricGuitar'})
# ===================== Japanese Pop & Game =====================
g('j_pop','J-Pop','Japanese Pop & Game','Bright, tightly produced pop with busy melodic lines.',(120,180),1.05,
  dict(human=0.003,quant=0.25,accent=[1,.85,.95,.85],compress=0.4,cut=1.1,reverb=(0.35,0.18,0.5)),
  aliases=['jpop','j pop'], kit='StudioKit', progs={26:'CleanElectricGuitar',27:'CleanElectricGuitar',29:'OverdrivenElectricGuitar'})
g('city_pop','City Pop','Japanese Pop & Game','Glossy 80s groove: slap bass, e-piano and chorus-soaked guitars.',(100,125),1.0,
  dict(swing=0.1,human=0.004,var=0.06,accent=[.9,1,.9,1],compress=0.3,reverb=(0.45,0.25,0.5),chorus=(0.5,2.0,0.3)),
  aliases=['citypop'], kit='StudioKit', progs={4:'RhodesMarkII',5:'RhodesMarkII',36:'SlapBass',37:'SlapBass',61:'BrassSection',48:'StringEnsemble'})
g('anime_rock','Anime Rock','Japanese Pop & Game','Fast, bright anime-opening rock: tight drums and soaring leads.',(150,200),1.15,
  dict(human=0.002,quant=0.25,accent=[1,.85,.95,.85],art='Accent',gate_fb=0.9,compress=0.3,cut=1.1,reverb=(0.3,0.15,0.5),**GUITAR_DIST(2.2)),
  bass=dict(no_dist=True), aliases=['anime','anison'], kit='RockKit', progs={26:'OverdrivenElectricGuitar',27:'OverdrivenElectricGuitar',29:'OverdrivenElectricGuitar',81:'SawLead'})
g('vocaloid','Vocaloid Pop','Japanese Pop & Game','Fast, machine-tight electropop built around a synthetic voice.',(140,190),1.2,
  dict(human=0.0,quant=0.25,accent=[1,.85,.95,.85],compress=0.4,cut=1.2,width=0.8,reverb=(0.35,0.18,0.5)),
  aliases=['vocaloid','vocaloid rock','miku'], kit='ElectronicKit', progs={80:'SupersawLead',81:'SupersawLead',38:'SynthBass',26:'OverdrivenElectricGuitar'})
g('sixteen_bit','16-Bit','Japanese Pop & Game','Console-era soundtrack: sampled-synth timbres with an echo-heavy mix.',(100,180),1.0,
  dict(quant=0.25,human=0.0,accent=[1,.85,.95,.85],reverb=(0.45,0.25,0.5),delay=(240.0,0.35,0.25)),
  aliases=['16 bit','snes','sixteen-bit','super nintendo'], kit='LoFiKit', progs={80:'SquareLead',81:'SawLead',38:'SynthBass',89:'WarmPad'})
g('ballad','Ballad','Pop & Rock','Slow, tender and sustained, with a lot of room.',(55,90),0.8,
  dict(human=0.008,var=0.1,art='Legato',gate_fb=0.97,reverb=(0.5,0.3,0.5)),
  kit='StudioKit')

# ---------------- existing presets: registry metadata ----------------
EXIST = [  # id, family, aliases, bpm
 ('classical','Orchestral',['classic'],(50,160)),('jazz','Jazz & Blues',[],(80,200)),('rock','Pop & Rock',[],(90,160)),
 ('metal','Metal',['heavy metal'],(110,220)),('house','Electronic',[],(118,130)),('techno','Electronic',[],(125,150)),
 ('drum_and_bass','Electronic',['dnb','drum and bass','drum n bass','drumnbass',"drum 'n' bass"],(160,180)),
 ('chiptune','Japanese Pop & Game',['8-bit','8bit','chip','chip tune'],(100,180)),('ambient','Electronic',[],(50,90)),
 ('touhou','Japanese Pop & Game',['doujin','shmup'],(140,200)),('funk','Soul, Funk & R&B',[],(90,120)),('blues','Jazz & Blues',[],(60,120)),
 ('reggae','Latin & Caribbean',[],(60,90)),('trance','Electronic',[],(128,145)),('dubstep','Electronic',[],(138,142)),
 ('bossa_nova','Latin & Caribbean',['bossa'],(100,150)),('hardcore','Electronic',['gabber'],(160,200)),('speedcore','Electronic',[],(220,300)),
]
# fix names the registry needs for existing presets
EXIST_NAMES = {'drum_and_bass':'Drum & Bass','bossa_nova':'Bossa Nova'}

def cname(s): return s
# ---------------- ext presets file ----------------
out = ['#pragma once','// Additional genre presets (see genre_registry.hpp for lookup by name). GENERATED by gen_genres.py.','','#include "genre_types.hpp"','','namespace wauvio {','    namespace music {','        namespace genre {','            namespace presets {','']
for s in G:
    out.append('                inline GenrePresetPtr %s() {' % s['id'])
    out.append('                    auto p = std::make_shared<GenrePreset>();')
    out.append('                    p->name = "%s";' % s['name'])
    out.append('                    p->description = "%s";' % s['desc'].replace('"','\\"'))
    out.append('                    p->family = "%s";' % s['family'])
    if s['aliases']: out.append('                    p->aliases = {%s};' % ', '.join('"%s"' % a for a in s['aliases']))
    out.append('                    p->bpm_min = %s; p->bpm_max = %s;' % (f(s['bpm'][0]), f(s['bpm'][1])))
    if s['tempo'] != 1.0: out.append('                    p->tempo_scale = %s;' % f(s['tempo']))
    out.append('')
    out.append('                    RoleProfile d;')
    out += ['                    ' + l for l in prof_lines('d', s['d'])]
    out.append('                    p->default_profile = d;')
    for key, setter in (('bass','set_bass'),('perc','set_percussion')):
        if s[key] is not None:
            var = 'bass' if key == 'bass' else 'perc'
            out.append(''); out.append('                    RoleProfile %s = d;' % var)
            out += ['                    ' + l for l in prof_lines(var, s[key])]
            out.append('                    p->%s(%s);' % (setter, var))
    out.append(''); out.append('                    return p;'); out.append('                }'); out.append('')
out += ['            }','        }','    }','}']
(R/'ext/genre/genre_presets_extra.hpp').write_text('\n'.join(out)+'\n',encoding='utf-8')

# ---------------- ext registry ----------------
reg = ['#pragma once','// Genre registry: look presets up by name, list them, group them by family. GENERATED by gen_genres.py.','//   auto p = wauvio::music::genre::find("drum and bass");      // case, spaces and punctuation do not matter; aliases work','//   for (auto& g : wauvio::music::genre::all()) ...             // every built-in preset','//   wauvio::music::genre::add(my_preset);                       // your own presets join the same lookup','',
 '#include "genre_presets.hpp"','#include "genre_presets_extra.hpp"','#include "genre_blend.hpp"','','#include <algorithm>','#include <cctype>','#include <map>','#include <string>','#include <vector>','',
 'namespace wauvio {','    namespace music {','        namespace genre {','','            namespace detail {',
 '                // "Drum & Bass" -> "drumbass", "drum and bass" -> "drumandbass" (both are registered), "Lo-fi" -> "lofi"',
 '                inline std::string normalize_name(const std::string& s) {','                    std::string o; for (char c : s) if (std::isalnum(static_cast<unsigned char>(c))) o += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));','                    return o;','                }',
 '                inline std::vector<GenrePresetPtr>& registry_storage() {','                    static std::vector<GenrePresetPtr> v = [] {','                        std::vector<GenrePresetPtr> r;',
 '                        auto with_meta = [&](GenrePresetPtr base, const char* fam, std::vector<std::string> aliases, double lo, double hi) {','                            auto p = std::make_shared<GenrePreset>(*base); p->family = fam; p->aliases = std::move(aliases); p->bpm_min = lo; p->bpm_max = hi; r.push_back(p);','                        };']
for id, fam, al, bpm in EXIST:
    reg.append('                        with_meta(presets::%s(), "%s", {%s}, %s, %s);' % (id, fam, ', '.join('"%s"'%a for a in al), f(bpm[0]), f(bpm[1])))
for s in G: reg.append('                        r.push_back(presets::%s());' % s['id'])
reg += ['                        return r;','                    }();','                    return v;','                }','            }','',
 '            inline const std::vector<GenrePresetPtr>& all() { return detail::registry_storage(); }','',
 '            // Adds (or replaces, if the name is already taken) a preset in the registry.',
 '            inline void add(GenrePresetPtr preset) {','                if (!preset) return;','                auto& v = detail::registry_storage(); const std::string key = detail::normalize_name(preset->name);',
 '                for (auto& p : v) if (detail::normalize_name(p->name) == key) { p = preset; return; }','                v.push_back(std::move(preset));','            }','',
 '            // Finds a preset by name or alias; nullptr if there is none.',
 '            inline GenrePresetPtr find(const std::string& name) {','                const std::string key = detail::normalize_name(name);',
 '                for (auto& p : all()) if (detail::normalize_name(p->name) == key) return p;',
 '                for (auto& p : all()) for (auto& a : p->aliases) if (detail::normalize_name(a) == key) return p;','                return nullptr;','            }','',
 '            inline std::vector<std::string> names() { std::vector<std::string> n; for (auto& p : all()) n.push_back(p->name); return n; }',
 '            inline std::vector<std::string> families() { std::vector<std::string> f; for (auto& p : all()) if (std::find(f.begin(), f.end(), p->family) == f.end()) f.push_back(p->family); return f; }',
 '            inline std::vector<GenrePresetPtr> in_family(const std::string& family) { std::vector<GenrePresetPtr> r; for (auto& p : all()) if (p->family == family) r.push_back(p); return r; }',
 '            // Presets whose typical tempo range contains `bpm`.',
 '            inline std::vector<GenrePresetPtr> for_bpm(double bpm) { std::vector<GenrePresetPtr> r; for (auto& p : all()) if (p->bpm_max > 0.0 && bpm >= p->bpm_min && bpm <= p->bpm_max) r.push_back(p); return r; }','',
 '        }','    }','}']
(R/'ext/genre/genre_registry.hpp').write_text('\n'.join(reg)+'\n',encoding='utf-8')

# ---------------- midi presets + registry ----------------
def midi_preset(fn, base_expr, kit, progs):
    L = ['                inline MidiGenrePresetPtr %s() {' % fn, '                    auto p = std::make_shared<MidiGenrePreset>(%s);' % base_expr]
    if kit: L.append('                    p->percussion_kit = [] { return std::make_shared<instruments::%s>(); };' % kit)
    for prog, cls in sorted(progs.items()): L.append('                    p->melodic_program_overrides[%d] = [] { return std::make_shared<instruments::%s>(); };' % (prog, cls))
    L += ['                    return p;','                }','']; return L
m = ['#pragma once','// MIDI side of the additional genres: each pairs a genre preset with a drum kit and General MIDI program overrides. GENERATED by gen_genres.py.','',
     '#include "genre_midi_presets.hpp"','#include "../../ext/genre/genre_presets_extra.hpp"','#include "../../ext/instruments/percussion/kits_extra.hpp"','#include "../../ext/instruments/synthesized/synths_extra.hpp"','#include "../../ext/instruments/keyboards/keyboards_extra.hpp"','#include "../../ext/instruments/guitars/plucked_extra.hpp"','#include "../../ext/instruments/guitars/guitars.hpp"','#include "../../ext/instruments/keyboards/keyboards.hpp"','#include "../../ext/instruments/strings/strings.hpp"','#include "../../ext/instruments/strings/strings_extra.hpp"','#include "../../ext/instruments/brass/brass.hpp"','#include "../../ext/instruments/woodwinds/woodwinds.hpp"','#include "../../ext/instruments/woodwinds/woodwinds_extra.hpp"','#include "../../ext/instruments/woodwinds/saxophones.hpp"','#include "../../ext/instruments/world/world.hpp"','#include "../../ext/instruments/percussion/tuned.hpp"','',
     'namespace wauvio {','    namespace midi {','        namespace genre {','            namespace presets {','']
for s in G: m += midi_preset(s['id'], 'wauvio::music::genre::presets::%s()' % s['id'], s['kit'], s['progs'])
m += ['            }','        }','    }','}']
(R/'midi/genre/genre_midi_presets_extra.hpp').write_text('\n'.join(m)+'\n',encoding='utf-8')

mr = ['#pragma once','// MIDI genre registry: the same names as the ext registry, but each result also carries its drum kit and instrument choices. GENERATED by gen_genres.py.','//   auto p = wauvio::midi::genre::find("hip hop");            // nullptr if unknown','//   music.setGenre(wauvio::midi::genre::find("jazz"));','//   music.setGenre(wauvio::midi::genre::with_strength(p, 0.5));   // half-strength genre','',
 '#include "genre_midi_presets_extra.hpp"','#include "../../ext/genre/genre_registry.hpp"','','#include <functional>','#include <string>','#include <vector>','',
 'namespace wauvio {','    namespace midi {','        namespace genre {','','            namespace detail {','                inline std::vector<std::pair<std::string, std::function<MidiGenrePresetPtr()>>>& midi_table() {',
 '                    static std::vector<std::pair<std::string, std::function<MidiGenrePresetPtr()>>> t = {']
for id, fam, al, bpm in EXIST: mr.append('                        { "%s", [] { return presets::%s(); } },' % (EXIST_NAMES.get(id, id.replace('_',' ').title()), id))
for s in G: mr.append('                        { "%s", [] { return presets::%s(); } },' % (s['name'], s['id']))
mr += ['                    };','                    return t;','                }','            }','',
 '            // Finds a genre by name or alias (case, spaces and punctuation ignored) and returns it with its kit and instruments.',
 '            // A preset added to the ext registry without a MIDI counterpart is returned with just its base profile.',
 '            inline MidiGenrePresetPtr find(const std::string& name) {',
 '                auto base = wauvio::music::genre::find(name);','                if (!base) return nullptr;',
 '                const std::string key = wauvio::music::genre::detail::normalize_name(base->name);',
 '                for (auto& e : detail::midi_table()) if (wauvio::music::genre::detail::normalize_name(e.first) == key) { auto p = std::make_shared<MidiGenrePreset>(*e.second()); p->base = base; return p; }',
 '                return std::make_shared<MidiGenrePreset>(base);','            }','',
 '            inline std::vector<MidiGenrePresetPtr> all() { std::vector<MidiGenrePresetPtr> r; for (auto& b : wauvio::music::genre::all()) r.push_back(find(b->name)); return r; }','',
 '            // The same genre at a fraction of its strength (1 = unchanged, 0 = none); kit and instrument choices are kept at 0.5 and above.',
 '            inline MidiGenrePresetPtr with_strength(const MidiGenrePresetPtr& preset, double strength) {',
 '                if (!preset || strength >= 1.0) return preset;','                auto p = std::make_shared<MidiGenrePreset>(*preset);','                p->base = wauvio::music::genre::scaled(preset->base, strength);',
 '                if (strength < 0.5) { p->percussion_kit = nullptr; p->melodic_program_overrides.clear(); }','                return p;','            }','',
 '        }','    }','}']
(R/'midi/genre/genre_midi_registry.hpp').write_text('\n'.join(mr)+'\n',encoding='utf-8')

# ---------------- kits ----------------
K = '''#pragma once
#include "../../core/model_instruments.hpp"
#include "kits.hpp"
#include "percussion_extra.hpp"

namespace wauvio {
    namespace instruments {
        using namespace wauvio::music;

        // 808-style trap kit: long sub kick, snappy snare and clap, tight hats.
        class TrapKit : public DrumKit {
        public:
            TrapKit() : DrumKit("Trap Kit") {
                auto kick = std::make_shared<Kick>();
                kick->recipe.fixed_pitch_hz = 42.0; kick->recipe.envelope.decay = 0.9; kick->recipe.pitch_attack_cents = -1500.0;
                map(36, kick);
                auto snare = std::make_shared<Snare>(); snare->recipe.filter_cutoff = 5200.0; map(38, snare);
                auto clap = std::make_shared<Snare>(); clap->recipe.filter_cutoff = 7000.0; clap->recipe.noise_mix = 0.95; clap->recipe.envelope.decay = 0.18; map(39, clap);
                map(42, std::make_shared<ClosedHiHat>());
                map(46, std::make_shared<OpenHiHat>());
                map(49, std::make_shared<CrashCymbal>());
                map(56, std::make_shared<Cowbell>());
            }
        };

        class LatinKit : public DrumKit {
        public:
            LatinKit() : DrumKit("Latin Kit") {
                map(36, std::make_shared<Kick>());
                map(38, std::make_shared<Rimshot>());
                map(42, std::make_shared<ClosedHiHat>());
                map(46, std::make_shared<OpenHiHat>());
                map(49, std::make_shared<CrashCymbal>());
                map(56, std::make_shared<Cowbell>());
                map(60, std::make_shared<Bongo>()); map(61, std::make_shared<Bongo>());
                map(62, std::make_shared<Conga>()); map(63, std::make_shared<Conga>()); map(64, std::make_shared<Conga>());
                map(65, std::make_shared<Timbales>()); map(66, std::make_shared<Timbales>());
                map(67, std::make_shared<Agogo>()); map(68, std::make_shared<Agogo>());
                map(69, std::make_shared<Cabasa>()); map(70, std::make_shared<Maracas>());
                map(73, std::make_shared<Guiro>()); map(74, std::make_shared<Guiro>());
                map(75, std::make_shared<Claves>());
            }
        };

        class SambaKit : public DrumKit {
        public:
            SambaKit() : DrumKit("Samba Kit") {
                map(36, std::make_shared<Surdo>());
                map(38, std::make_shared<Repinique>());
                map(40, std::make_shared<Pandeiro>());
                map(54, std::make_shared<Pandeiro>());
                map(60, std::make_shared<Bongo>());
                map(67, std::make_shared<Agogo>()); map(68, std::make_shared<Agogo>());
                map(69, std::make_shared<Cabasa>()); map(70, std::make_shared<Maracas>());
                map(73, std::make_shared<Guiro>()); map(75, std::make_shared<Claves>());
                map(78, std::make_shared<Cuica>()); map(79, std::make_shared<Cuica>());
            }
        };

        class FolkKit : public DrumKit {
        public:
            FolkKit() : DrumKit("Folk Kit") {
                map(36, std::make_shared<Bodhran>());
                auto rim = std::make_shared<Bodhran>(); rim->recipe.fixed_pitch_hz = 150.0; rim->recipe.filter_cutoff = 2400.0; map(38, rim);
                map(42, std::make_shared<Shaker>());
                map(46, std::make_shared<Shaker>());
                map(54, std::make_shared<Tambourine>());
                map(49, std::make_shared<FingerCymbals>());
                map(56, std::make_shared<Woodblock>());
            }
        };

        class IndianKit : public DrumKit {
        public:
            IndianKit() : DrumKit("Indian Percussion Kit") {
                map(36, std::make_shared<Dhol>());
                map(38, std::make_shared<Dholak>());
                map(40, std::make_shared<Mridangam>());
                map(41, std::make_shared<Ghatam>());
                map(42, std::make_shared<Kanjira>());
                map(60, std::make_shared<Tabla>()); map(61, std::make_shared<Tabla>());
                map(54, std::make_shared<Daf>());
                map(69, std::make_shared<Cabasa>());
            }
        };

        class AfroKit : public DrumKit {
        public:
            AfroKit() : DrumKit("Afro Kit") {
                map(36, std::make_shared<Udu>());
                map(38, std::make_shared<Djembe>());
                map(42, std::make_shared<ClosedHiHat>());
                map(56, std::make_shared<Cowbell>());
                map(62, std::make_shared<Conga>()); map(63, std::make_shared<Conga>()); map(64, std::make_shared<Conga>());
                map(67, std::make_shared<Agogo>()); map(68, std::make_shared<Agogo>());
                map(70, std::make_shared<Shaker>());
                map(75, std::make_shared<Claves>());
            }
        };

        class MarchingKit : public DrumKit {
        public:
            MarchingKit() : DrumKit("Marching Kit") {
                map(36, std::make_shared<ConcertBassDrum>());
                map(38, std::make_shared<MarchingSnare>());
                map(40, std::make_shared<FieldDrum>());
                map(41, std::make_shared<TenorDrum>()); map(43, std::make_shared<TenorDrum>()); map(45, std::make_shared<TenorDrum>());
                map(49, std::make_shared<CrashCymbal>());
                map(51, std::make_shared<SuspendedCymbal>());
            }
        };

    }
}
'''
(R/'ext/instruments/percussion/kits_extra.hpp').write_text(K,encoding='utf-8')
print(len(G),'new genres')

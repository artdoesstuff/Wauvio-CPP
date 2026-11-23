import subprocess, sys, numpy as np, pandas as pd
sys.path.insert(0,'/tmp/song/an'); from common import *
SING='/tmp/b/sing_score_final'
def render_notes(extra, tag, lens=(0.7,1.0,1.4), pitches=(62,65,67,69,72,74), reps=2):
    sc="voice yune\ntempo 120\n"+extra; 
    for r in range(reps):
        for L in lens:
            for p in pitches: sc+=f"note {p} {int(L*960)} あ\nrest 480\n"
    open(f'/tmp/song/an/{tag}.txt','w').write(sc); subprocess.run([SING,f'/tmp/song/an/{tag}.txt',f'/tmp/song/an/{tag}.wav'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
    t,m,I=pitch_track(f'/tmp/song/an/{tag}.wav'); rows=[]
    for a,b in segment_runs(t,m):
        r=vib_metrics(t,m,I,a,b)
        if r: rows.append(r)
    return pd.DataFrame(rows)
def summ(df,tag):
    v=df[df.has]; 
    if len(v)==0: print(f"  {tag}: no vibrato found in {len(df)} notes"); return
    d=v.dropna(subset=['rate_early','rate_late'])
    print(f"  {tag:46s} n={len(v):2d}/{len(df):2d}  rate {v.rate.median():.2f} | depth {v.peak_st.median():.2f} | delay {v['delay'].median():.2f} full {v['full'].median():.2f} s | drift {100*((d.rate_late/d.rate_early)-1).median():+5.1f}% | rising {v.rise_frac.median():.3f} | AM {v.am_pp_db.median():.2f} dB lag {v.am_lag_ms.median():.0f} ms")
if __name__=='__main__':
    print("ENGINE (same estimators), Yune, notes after rests:")
    summ(render_notes("natural 0\nauto_expression off\n",'ev_plain'),"V3 defaults, natural 0, auto_expression off")
    summ(render_notes("",'ev_default'),"V3 defaults as shipped (natural .3, auto on)")
    summ(render_notes("natural 0\nauto_expression off\nstyle vib_shape 0\nstyle vib_am 0\nstyle vib_accel 0\n",'ev_v2like'),"V2-like (shape/am/accel 0)")
    d=pd.read_pickle('/tmp/song/an/vib2.pkl'); v=d[d.has]; dd=v.dropna(subset=['rate_early','rate_late'])
    print(f"\n  SINGER (JSUT-song)                              n={len(v):3d}/{len(d):3d}  rate {v.rate.median():.2f} | depth {v.peak_st.median():.2f} | delay {v['delay'].median():.2f} full {v['full'].median():.2f} s | drift {100*((dd.rate_late/dd.rate_early)-1).median():+5.1f}% | rising {v.rise_frac.median():.3f} | AM {v.am_pp_db.median():.2f} dB lag {v.am_lag_ms.median():.0f} ms")

import subprocess, sys, numpy as np
sys.path.insert(0,'/tmp/song/an'); from common import *
SING='/tmp/b/sing_score_final'
def spread(extra,tag,n=24):
    sc="voice yune\ntempo 120\nstyle vibrato_depth 0\n"+extra
    for i in range(n): sc+=f"note 64 360 あ\nrest 120\n"
    open(f'/tmp/song/an/{tag}.txt','w').write(sc); subprocess.run([SING,f'/tmp/song/an/{tag}.txt',f'/tmp/song/an/{tag}.wav'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
    t,m,I=pitch_track(f'/tmp/song/an/{tag}.wav'); runs=segment_runs(t,m,min_len=0.15)
    med=[]; on=[]
    for a,b in runs:
        x=m[a:b+1]; x=x[~np.isnan(x)]
        if len(x)<30: continue
        med.append(np.median(x[len(x)//4:])); on.append(t[a])
    med=np.array(med); on=np.array(on); cents=(med-np.median(med))*100
    d=np.diff(on)*1000-500
    return cents.std(),d.std(),len(med)
print("engine pitch spread across identical notes (sd, cents) and inter-onset timing sd (ms), same Praat pipeline")
for lvl,ae in ((0,False),(0.3,True),(0.6,True),(1.0,True)):
    c,tm,n=spread(f"natural {lvl}\nauto_expression {'on' if ae else 'off'}\n",f'nat_{lvl}')
    print(f"  natural {lvl:3}: pitch sd {c:5.1f} cents, onset-interval sd {tm:5.1f} ms  (n={n})")
print("  SINGER: note pitch sd 25 cents around the nearest semitone; onset deviation from the click about 30 ms (upper bound)")

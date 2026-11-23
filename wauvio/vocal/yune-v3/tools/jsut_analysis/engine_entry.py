import subprocess, sys, numpy as np, pandas as pd
sys.path.insert(0,'/tmp/song/an'); from common import *; from notes import smooth_notes
SING='/tmp/b/sing_score_final'
def entry_stats(t,m,runs,notes):
    ent=[]
    for (a,b) in notes:
        dur=t[b]-t[a]
        if dur<0.30: continue
        x=m[a:b+1]
        if np.isnan(x).mean()>0.15: continue
        x=np.interp(np.arange(len(x)),np.where(~np.isnan(x))[0],x[~np.isnan(x)]); xs=movavg(x,30)
        target=np.median(xs[int(0.4*len(xs)):int(0.9*len(xs))])
        if a in {r[0] for r in runs}: ent.append((np.median(x[2:12])-target,np.median(x[12:24])-target))
    return np.array(ent)
def port_stats(t,m,notes):
    out=[]
    for (a1,b1),(a2,b2) in zip(notes[:-1],notes[1:]):
        if a2!=b1+1 or (t[b1]-t[a1])<0.25 or (t[b2]-t[a2])<0.25: continue
        x=m[a1:b2+1]
        if np.isnan(x).mean()>0.1: continue
        x=np.interp(np.arange(len(x)),np.where(~np.isnan(x))[0],x[~np.isnan(x)]); xs=movavg(x,12)
        n1=b1-a1+1; p1=np.median(xs[max(0,n1-30):n1-8]); p2=np.median(xs[n1+10:n1+40]); step=p2-p1
        if abs(step)<1.0 or abs(step)>7: continue
        seg=(xs[max(0,n1-40):n1+40]-p1)/step; lo=np.where(seg>=0.15)[0]; hi=np.where(seg>=0.85)[0]
        if len(lo) and len(hi) and hi[0]>=lo[0]:
            out.append((abs(step),(hi[0]-lo[0])/FS,(np.max(xs[n1:n1+60]*np.sign(step))-p2*np.sign(step))/abs(step)))
    return np.array(out)
def run(extra,tag):
    sc="voice yune\ntempo 120\n"+extra
    for p in (62,65,67,69,72,64,66,68):                                 # entries after rests
        sc+=f"note {p} 960 あ\nrest 480\n"
    for base,step in ((62,2),(65,3),(67,-2),(70,-4),(60,4),(64,2),(69,-3),(66,2),(72,-3),(61,3),(63,2),(68,-2),(65,-3),(62,3),(71,-2),(60,2),(67,3),(64,-2)):   # legato pairs
        sc+=f"note {base} 720 あ\nnote {base+step} 720 ー\nrest 480\n"
    open(f'/tmp/song/an/{tag}.txt','w').write(sc); subprocess.run([SING,f'/tmp/song/an/{tag}.txt',f'/tmp/song/an/{tag}.wav'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,check=True)
    t,m,I=pitch_track(f'/tmp/song/an/{tag}.wav'); runs=segment_runs(t,m); notes=[]
    for a,b in runs: notes+=smooth_notes(m,t,a,b)
    return entry_stats(t,m,runs,notes),port_stats(t,m,notes)
if __name__=='__main__':
    print("pitch entry (first 10-60 ms vs the steady pitch, semitones) and legato transitions (15%->85% time, overshoot)")
    for extra,tag in (("natural 0\nauto_expression off\n","en_plain"),("","en_default"),("natural 0\nauto_expression off\nstyle scoop 0.1\nstyle overshoot 0.1\n","en_lowscoop")):
        e,p=run(extra,tag)
        print(f"  {tag:12s} entry median {np.median(e[:,0]) if len(e) else float('nan'):+.2f} st (n={len(e)}) | legato: n={len(p)}  transition {1000*np.median(p[:,1]) if len(p) else float('nan'):3.0f} ms  overshoot {100*np.median(p[:,2]) if len(p) else float('nan'):3.0f}% of the step")
    print("  SINGER       entry median -0.06 st (n=488) | legato: n=99  transition  55 ms  overshoot  16% of the step")

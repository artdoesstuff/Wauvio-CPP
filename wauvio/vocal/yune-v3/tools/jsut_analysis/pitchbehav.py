import pickle, numpy as np, pandas as pd
from scipy.ndimage import median_filter
D=pickle.load(open('/tmp/song/an/notes.pkl','rb')); vib=pd.read_pickle('/tmp/song/an/vib.pkl'); FS=200.0
def movavg(x,w): return np.convolve(np.pad(x,(w//2,w-1-w//2),mode='edge'),np.ones(w)/w,'valid')
# ---------- (A) vibrato onset delay, notes starting after an unvoiced gap vs legato continuations ----------
import importlib, sys
rows=[]
for f,d in D.items():
    t=d['t']; runs=d['runs']; starts={a:i for i,(a,b) in enumerate(runs)}
    for ni,(a,b) in enumerate(d['notes']):
        rows.append((f.split('/')[-1][:3],round(t[a],3),a in starts))
meta=pd.DataFrame(rows,columns=['song','t0','init'])
# match to vib table by (song, order): rebuild the same filtering as vibrato.py
sel=[]
for f,d in D.items():
    t=d['t']; m=d['m']
    for ni,(a,b) in enumerate(d['notes']):
        dur=t[b]-t[a]
        if dur<0.55: continue
        x=m[a+8:b-8+1]
        if np.isnan(x).mean()>0.1: continue
        sel.append((f.split('/')[-1][:3],a in {r[0] for r in d['runs']}))
vib=vib.copy(); vib['init']=[s[1] for s in sel]
v=vib[vib.has]
print("(A) vibrato onset (envelope reaches 50% / 90%) by how the note starts")
for nm,s in (('after an unvoiced gap (syllable onset)',v[v.init]),('legato continuation',v[~v.init])):
    print(f"   {nm:40s} n={len(s):3d}: 50% at {s['delay'].median():.2f} s (IQR {s['delay'].quantile(.25):.2f}-{s['delay'].quantile(.75):.2f}), 90% at {s['full'].median():.2f} s; vibrato present in {100*vib[vib.init==(nm[0]=='a')].has.mean():.0f}% of such notes")
# ---------- (B) pitch entry after a gap: first 10-60 ms vs the note's steady pitch ----------
ent=[]; port=[]; dev=[]
for f,d in D.items():
    t=d['t']; m=d['m']; runs={a for a,b in d['runs']}
    for ni,(a,b) in enumerate(d['notes']):
        dur=t[b]-t[a]
        if dur<0.30: continue
        x=m[a:b+1]; 
        if np.isnan(x).mean()>0.15: continue
        x=np.interp(np.arange(len(x)),np.where(~np.isnan(x))[0],x[~np.isnan(x)]); xs=movavg(x,30)       # 150 ms smoothing removes vibrato
        target=np.median(xs[int(0.4*len(xs)):int(0.9*len(xs))])
        if a in runs:
            e0=np.median(x[2:12])-target; e1=np.median(x[12:24])-target       # 10-60 ms and 60-120 ms after voicing starts
            conv=np.where(np.abs(xs-target)<0.3)[0]; ent.append((e0,e1,conv[0]/FS if len(conv) else np.nan,dur))
        dev.append(target)
ent=np.array(ent)
print("\n(B) pitch entry at syllable onsets after a gap (n=%d, notes >= 0.3 s): offset from the steady pitch"%len(ent))
print("   first 10-60 ms:  median %+.2f st (IQR %+.2f..%+.2f)   share starting flat (below the target): %.0f%%"%(np.median(ent[:,0]),np.percentile(ent[:,0],25),np.percentile(ent[:,0],75),100*np.mean(ent[:,0]<0)))
print("   60-120 ms:       median %+.2f st (IQR %+.2f..%+.2f)"%(np.median(ent[:,1]),np.percentile(ent[:,1],25),np.percentile(ent[:,1],75)))
print("   time until the smoothed pitch is within 0.3 st of the target: median %.2f s"%np.nanmedian(ent[:,2]))
# ---------- (C) legato transitions: duration and overshoot ----------
for f,d in D.items():
    t=d['t']; m=d['m']; N=d['notes']
    for (a1,b1),(a2,b2) in zip(N[:-1],N[1:]):
        if a2!=b1+1 or (t[b1]-t[a1])<0.25 or (t[b2]-t[a2])<0.25: continue
        x=m[a1:b2+1]
        if np.isnan(x).mean()>0.1: continue
        x=np.interp(np.arange(len(x)),np.where(~np.isnan(x))[0],x[~np.isnan(x)]); xs=movavg(x,12)
        n1=b1-a1+1; p1=np.median(xs[max(0,n1-30):n1-8]); p2=np.median(xs[n1+10:n1+40]); step=p2-p1
        if abs(step)<1.0 or abs(step)>7: continue
        seg=(xs[max(0,n1-40):n1+40]-p1)/step
        lo=np.where(seg>=0.15)[0]; hi=np.where(seg>=0.85)[0]
        if len(lo) and len(hi) and hi[0]>=lo[0]:
            over=(np.max(xs[n1:n1+60]*np.sign(step))-p2*np.sign(step))/abs(step)
            port.append((abs(step),(hi[0]-lo[0])/FS,over))
port=np.array(port)
print("\n(C) legato note-to-note transitions (n=%d, steps 1-7 st): 15%%->85%% of the step"%len(port))
for lo,hi in ((1,2.5),(2.5,4.5),(4.5,7.1)):
    s=port[(port[:,0]>=lo)&(port[:,0]<hi)]
    if len(s): print(f"   step {lo}-{hi} st: n={len(s):3d}  transition time median {np.median(s[:,1])*1000:4.0f} ms (IQR {np.percentile(s[:,1],25)*1000:3.0f}-{np.percentile(s[:,1],75)*1000:3.0f})   overshoot beyond target median {np.median(s[:,2])*100:4.0f}% of the step")
# ---------- (D) intonation: deviation of note pitch from the equal-tempered grid ----------
dev=np.array(dev); off=np.median((dev-np.round(dev)))   # tuning offset of the whole corpus
c=((dev-np.round(dev)-off)*100)
print("\n(D) intonation (n=%d note pitches, smoothed over vibrato): reference tuning offset %+.0f cents; spread around the nearest semitone: sd %.0f cents, MAD-sd %.0f cents, 90%% within +-%.0f cents"%(len(dev),off*100,c.std(),1.4826*np.median(np.abs(c-np.median(c))),np.percentile(np.abs(c),90)))

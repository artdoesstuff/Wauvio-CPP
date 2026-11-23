import glob, numpy as np, parselmouth, pickle
from scipy.ndimage import median_filter
FILES=sorted(glob.glob('/tmp/song/jsut-song_ver1/child_song/wav/*.wav'))
def track(f):
    s=parselmouth.Sound(f); p=s.to_pitch_ac(time_step=0.005,pitch_floor=120,pitch_ceiling=1000,very_accurate=True,silence_threshold=0.03,voicing_threshold=0.5)
    f0=p.selected_array['frequency'].copy(); t=p.xs(); v=f0>0
    m=np.full_like(f0,np.nan); m[v]=69+12*np.log2(f0[v]/440)
    # fix octave jumps: replace points > 6 semitones from a 150 ms running median with the nearest octave shift
    mm=m.copy(); idx=np.where(v)[0]
    if len(idx)>5:
        med=median_filter(np.where(v,m,np.nanmedian(m)),size=31)
        for i in idx:
            d=m[i]-med[i]
            if abs(d)>6: mm[i]=m[i]-12*np.round(d/12)
    # intensity (dB) per 5 ms
    inten=s.to_intensity(minimum_pitch=120,time_step=0.005); ti=inten.xs(); iv=inten.values[0]
    ii=np.interp(t,ti,iv)
    return s,t,mm,ii
def voiced_runs(m,t,min_gap=0.04,min_len=0.06):
    v=~np.isnan(m); runs=[]; i=0; n=len(m)
    while i<n:
        if v[i]:
            j=i
            while j+1<n:
                if v[j+1]: j+=1
                else:
                    k=j+1
                    while k<n and not v[k]: k+=1
                    if k<n and (t[k]-t[j])<=min_gap: j=k
                    else: break
            if t[j]-t[i]>=min_len: runs.append((i,j))
            i=j+1
        else: i+=1
    return runs
def smooth_notes(m,t,i0,j0):
    """split a voiced run into notes at pitch plateaus (steps >= 0.7 semitone in a 100 ms median-smoothed contour)"""
    seg=m[i0:j0+1].copy(); x=np.interp(np.arange(len(seg)),np.where(~np.isnan(seg))[0],seg[~np.isnan(seg)])
    sm=median_filter(x,size=21)
    notes=[]; start=0; ref=np.median(sm[:12]) if len(sm)>12 else sm[0]
    for k in range(1,len(sm)):
        if abs(sm[k]-ref)>=0.7:
            # confirm the step persists for >=60 ms
            if k+12<len(sm) and abs(np.median(sm[k:k+12])-ref)>=0.7 and (k-start)>=12:
                notes.append((i0+start,i0+k-1)); start=k; ref=np.median(sm[k:k+12])
        else: ref=0.97*ref+0.03*sm[k] if False else ref
    notes.append((i0+start,j0)); return notes
if __name__=='__main__':
    out={}
    for f in FILES:
        s,t,m,ii=track(f); runs=voiced_runs(m,t); notes=[]
        for (a,b) in runs: notes+=smooth_notes(m,t,a,b)
        out[f]=dict(t=t,m=m,inten=ii,runs=runs,notes=notes,dur=s.duration)
        print(f.split('/')[-1][:3],"voiced runs",len(runs),"notes",len(notes))
    pickle.dump(out,open('/tmp/song/an/notes.pkl','wb'))

import numpy as np, parselmouth
from scipy.signal import butter, sosfiltfilt, find_peaks, hilbert
FS=200.0
bp=butter(2,[3.5,8.5],'bandpass',fs=FS,output='sos')
def movavg(x,w): return np.convolve(np.pad(x,(w//2,w-1-w//2),mode='edge'),np.ones(w)/w,'valid')
def pitch_track(path):
    s=parselmouth.Sound(path); p=s.to_pitch_ac(time_step=0.005,pitch_floor=120,pitch_ceiling=1000,very_accurate=True,silence_threshold=0.03,voicing_threshold=0.5)
    f0=p.selected_array['frequency'].copy(); t=p.xs(); m=np.full_like(f0,np.nan); v=f0>0; m[v]=69+12*np.log2(f0[v]/440)
    inten=s.to_intensity(minimum_pitch=120,time_step=0.005); ii=np.interp(t,inten.xs(),inten.values[0])
    return t,m,ii
def vib_metrics(t,m,inten,a,b):
    """a,b: first/last index of one note (voiced). Returns dict of vibrato measures or None."""
    dur=t[b]-t[a]
    if dur<0.5: return None
    x=m[a:b+1].copy()
    if np.isnan(x).mean()>0.1: return None
    x=np.interp(np.arange(len(x)),np.where(~np.isnan(x))[0],x[~np.isnan(x)])
    y=x-movavg(x,50)                       # slow trend out (0.25 s)
    yy=y[8:-8]                             # drop 40 ms at each end
    yb=sosfiltfilt(bp,yy); rms=yb.std()
    w=np.hanning(len(yy)); N=1<<14; S=np.abs(np.fft.rfft(yy*w,N))**2; fr=np.fft.rfftfreq(N,1/FS)
    band=(fr>=3.5)&(fr<=8.5); pk=fr[band][np.argmax(S[band])]
    strength=S[np.abs(fr-pk)<=0.8].sum()/S[(fr>=1)&(fr<=15)].sum()
    r=dict(dur=dur,peak_st=np.sqrt(2)*rms,rate=pk,strength=strength,has=bool((np.sqrt(2)*rms>=0.10)&(strength>=0.30)))
    if not r['has']: return r
    # cycle-by-cycle amplitude (no zero-phase filtering): peaks/troughs of the lightly smoothed detrended contour
    ys=movavg(y,3); pks,_=find_peaks(ys,distance=int(FS*0.09),prominence=0.03); trs,_=find_peaks(-ys,distance=int(FS*0.09),prominence=0.03)
    amps=[]; tt=[]
    for p0 in pks:
        prev=trs[trs<p0]; 
        if len(prev): amps.append((ys[p0]-ys[prev[-1]])/2); tt.append((0.5*(p0+prev[-1]))/FS)
    for q0 in trs:
        prev=pks[pks<q0]
        if len(prev): amps.append((ys[prev[-1]]-ys[q0])/2); tt.append((0.5*(q0+prev[-1]))/FS)
    o=np.argsort(tt); amps=np.array(amps)[o]; tt=np.array(tt)[o]
    if len(amps)>=4:
        ref=np.median(amps[len(amps)//2:])
        ok=np.where(amps>=0.5*ref)[0]; r['delay']=tt[ok[0]] if len(ok) else np.nan
        ok9=[i for i in range(len(amps)-1) if amps[i]>=0.9*ref and amps[i+1]>=0.9*ref]; r['full']=tt[ok9[0]] if ok9 else np.nan
        r['amp_ref']=ref
    # rate drift from the intervals between successive peaks
    if len(pks)>=6:
        per=np.diff(pks)/FS; tm=(pks[1:]+pks[:-1])/2/FS; h=len(per)//3
        if h>=2: r['rate_early']=1/np.median(per[:h]); r['rate_late']=1/np.median(per[-h:])
    # shape: share of each cycle spent rising
    fl=[]
    for p0 in pks:
        prev=trs[trs<p0]; nxt=trs[trs>p0]
        if len(prev) and len(nxt): fl.append((p0-prev[-1])/(nxt[0]-prev[-1]))
    r['rise_frac']=np.median(fl) if len(fl)>=3 else np.nan
    iv=inten[a+8:b-8+1]
    if len(iv)==len(yb) and not np.isnan(iv).any() and iv.std()>0:
        z=sosfiltfilt(bp,iv-movavg(iv,50)); r['am_pp_db']=2*np.sqrt(2)*z.std()
        cc=[np.corrcoef(yb[max(0,-L):len(yb)-max(0,L)],z[max(0,L):len(z)-max(0,-L)])[0,1] for L in range(-40,41)]
        r['am_lag_ms']=(int(np.argmax(cc))-40)/FS*1000; r['am_corr']=max(cc)
    return r
def segment_runs(t,m,min_gap=0.04,min_len=0.06):
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

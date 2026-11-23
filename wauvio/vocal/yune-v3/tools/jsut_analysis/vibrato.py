import pickle, numpy as np
from scipy.signal import butter, sosfiltfilt, hilbert, find_peaks
D=pickle.load(open('/tmp/song/an/notes.pkl','rb')); FS=200.0
bp=butter(2,[3.5,8.5],'bandpass',fs=FS,output='sos')
def movavg(x,w): 
    k=np.ones(w)/w; return np.convolve(np.pad(x,(w//2,w-1-w//2),mode='edge'),k,'valid')
rows=[]
for f,d in D.items():
    t=d['t']; m=d['m']; I=d['inten']
    for ni,(a,b) in enumerate(d['notes']):
        dur=t[b]-t[a]
        if dur<0.55: continue
        a2,b2=a+8,b-8                                   # trim 40 ms at both ends
        x=m[a2:b2+1].copy()
        if np.isnan(x).mean()>0.1: continue
        x=np.interp(np.arange(len(x)),np.where(~np.isnan(x))[0],x[~np.isnan(x)])
        y=x-movavg(x,60)                                # remove the slow trend (0.3 s)
        yb=sosfiltfilt(bp,y); rms=yb.std()
        # spectrum peak in 3.5-8.5 Hz
        w=np.hanning(len(y)); N=1<<14; S=np.abs(np.fft.rfft(y*w,N))**2; fr=np.fft.rfftfreq(N,1/FS)
        band=(fr>=3.5)&(fr<=8.5); pk=fr[band][np.argmax(S[band])]
        near=(np.abs(fr-pk)<=0.8); strength=S[near].sum()/S[(fr>=1.0)&(fr<=15)].sum()
        has=(np.sqrt(2)*rms>=0.10)&(strength>=0.30)
        rec=dict(song=f.split('/')[-1][:3],dur=dur,rms=rms,peak_st=np.sqrt(2)*rms,rate=pk,strength=strength,has=has,midi=np.median(x))
        if has:
            an=hilbert(yb); amp=np.abs(an); ph=np.unwrap(np.angle(an)); inst=np.gradient(ph)*FS/(2*np.pi)
            inst_s=movavg(inst,20); ampS=movavg(amp,20); n=len(amp)
            # onset delay: first time the smoothed envelope reaches 50% of its median over the note
            ref=np.median(ampS[n//3:]) if n>30 else np.median(ampS)
            idx=np.where(ampS>=0.5*ref)[0]; rec['delay']=(idx[0]/FS+0.04) if len(idx) else np.nan
            rec['full']=(np.where(ampS>=0.9*ref)[0][0]/FS+0.04) if np.any(ampS>=0.9*ref) else np.nan
            if n>=120:
                rec['rate_early']=np.median(inst_s[int(0.2*n):int(0.4*n)]); rec['rate_late']=np.median(inst_s[int(0.7*n):int(0.9*n)])
            # skew: share of each cycle spent rising (trough -> peak)
            pks,_=find_peaks(yb,distance=int(FS/10)); trs,_=find_peaks(-yb,distance=int(FS/10)); fr_list=[]
            for p0 in pks:
                prev=trs[trs<p0]; nxt=trs[trs>p0]
                if len(prev) and len(nxt): fr_list.append((p0-prev[-1])/(nxt[0]-prev[-1]))
            rec['rise_frac']=np.median(fr_list) if len(fr_list)>=3 else np.nan
            # amplitude vibrato: intensity (dB) band-passed, correlation with pitch band
            iv=I[a2:b2+1]
            if not np.isnan(iv).any() and iv.std()>0:
                z=sosfiltfilt(bp,iv-movavg(iv,60)); rec['am_pp_db']=2*np.sqrt(2)*z.std()
                cc=[np.corrcoef(yb[max(0,-L):len(yb)-max(0,L)],z[max(0,L):len(z)-max(0,-L)])[0,1] for L in range(-40,41)]
                L=int(np.argmax(cc))-40; rec['am_lag_ms']=L/FS*1000; rec['am_corr']=max(cc)
        rows.append(rec)
import pandas as pd
df=pd.DataFrame(rows); df.to_pickle('/tmp/song/an/vib.pkl')
print("notes >= 0.55 s analysed:",len(df),"| with clear vibrato:",int(df.has.sum()),"(%.0f%%)"%(100*df.has.mean()))
for lo,hi in ((0.55,0.9),(0.9,1.3),(1.3,2.0),(2.0,9)):
    s=df[(df.dur>=lo)&(df.dur<hi)]; print(f"   note length {lo:.2f}-{hi:.2f} s: n={len(s):4d}, vibrato in {100*s.has.mean():3.0f}%")
v=df[df.has]
print("\nvibrato notes (n=%d):"%len(v))
print("  rate Hz        median %.2f  IQR %.2f-%.2f   (5-95%%: %.2f-%.2f)"%(v.rate.median(),v.rate.quantile(.25),v.rate.quantile(.75),v.rate.quantile(.05),v.rate.quantile(.95)))
print("  depth (peak st) median %.2f  IQR %.2f-%.2f   (5-95%%: %.2f-%.2f)"%(v.peak_st.median(),v.peak_st.quantile(.25),v.peak_st.quantile(.75),v.peak_st.quantile(.05),v.peak_st.quantile(.95)))
print("  onset delay s  (envelope reaches 50%%)  median %.2f IQR %.2f-%.2f ; reaches 90%% at median %.2f s"%(v['delay'].median(),v['delay'].quantile(.25),v['delay'].quantile(.75),v['full'].median()))
r=v.dropna(subset=['rate_early','rate_late']); print("  rate drift: early %.2f Hz -> late %.2f Hz (median change %+.2f Hz = %+.1f%%, n=%d)"%(r.rate_early.median(),r.rate_late.median(),(r.rate_late-r.rate_early).median(),100*((r.rate_late/r.rate_early)-1).median(),len(r)))
q=v.dropna(subset=['rise_frac']); print("  share of cycle spent rising: median %.3f (0.5 = symmetric), IQR %.3f-%.3f, n=%d"%(q.rise_frac.median(),q.rise_frac.quantile(.25),q.rise_frac.quantile(.75),len(q)))
a=v.dropna(subset=['am_pp_db']); print("  amplitude vibrato: median %.2f dB peak-to-peak (IQR %.2f-%.2f); intensity follows pitch with lag %.0f ms (median; corr %.2f, n=%d)"%(a.am_pp_db.median(),a.am_pp_db.quantile(.25),a.am_pp_db.quantile(.75),a.am_lag_ms.median(),a.am_corr.median(),len(a)))
print("  depth vs rate corr %.2f ; depth vs note length corr %.2f"%(np.corrcoef(v.peak_st,v.rate)[0,1],np.corrcoef(v.peak_st,v.dur)[0,1]))

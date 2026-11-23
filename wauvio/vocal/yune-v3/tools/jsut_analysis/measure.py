import sys, csv, numpy as np, parselmouth
from parselmouth.praat import call
from scipy.signal import butter, sosfilt
sys.path.insert(0,'/tmp/jsut/an'); from labs import load_all
V=set('aiueo'); STOP={'b','d','g','p','t','k'}; VOICELESS={'p','t','k'}
def hf_env(x,sr):
    sos=butter(4,2500,'highpass',fs=sr,output='sos'); h=sosfilt(sos,x); hop=int(0.001*sr)
    n=len(h)//hop; e=np.sqrt((h[:n*hop].reshape(n,hop)**2).mean(1)); return 20*np.log10(e+1e-7)   # dB per ms
def process(u):
    snd=parselmouth.Sound(u['wav']); sr=snd.sampling_frequency; x=snd.values[0]
    env=hf_env(x,sr)
    fm=snd.to_formant_burg(time_step=0.002,max_number_of_formants=5,maximum_formant=5500,window_length=0.015,pre_emphasis_from=50)
    pp=call(snd,"To PointProcess (periodic, cc)",90,600); npulse=call(pp,"Get number of points")
    pulses=np.array([call(pp,"Get time from index",i+1) for i in range(npulse)]) if npulse else np.array([])
    pit=snd.to_pitch(time_step=0.005,pitch_floor=90,pitch_ceiling=600)
    sg=u['segs']; rows=[]
    for i in range(1,len(sg)-2):
        cs,ce,c=sg[i]; vs,ve,v=sg[i+1]
        if c not in STOP or v not in V: continue
        if ve-vs<0.045: continue
        prev=sg[i-1][2]; init=prev in('sil','pau'); nxt=sg[i+2][2]
        # release burst: strongest HF rise within the last 90 ms of the consonant label and 20 ms beyond
        lo=int((ce-0.09)*1000); hi=int((ce+0.02)*1000)
        if lo<2 or hi>=len(env)-2: continue
        d=np.diff(env[lo:hi]); k=int(np.argmax(d)); rise=d[k]; tb=(lo+k+1)/1000.0
        conf=rise>=8.0
        if not conf: tb=ce                       # fall back to the label boundary
        # first glottal pulse at/after the release (and any prevoicing run before it)
        after=pulses[(pulses>=tb-0.0005)&(pulses<tb+0.12)]; tfp=after[0] if len(after) else np.nan
        before=pulses[(pulses>=tb-0.12)&(pulses<tb-0.0005)]
        prev_ms=0.0
        if len(before):
            run=before[::-1]; start=run[0]
            for a,b in zip(run[:-1],run[1:]):
                if a-b>0.015: break
                start=b
            if tb-run[0]<0.015: prev_ms=(tb-start)*1000      # voicing running right up to the release
        if np.isnan(tfp): continue
        vot=(tfp-tb)*1000
        t_on=(tfp if c in VOICELESS else tb)+0.006         # where "onset" formants are read
        if c not in VOICELESS and prev_ms>0: t_on=tb+0.006
        tm=0.5*(vs+ve)
        def F(t,n):
            val=fm.get_value_at_time(n,t); return val if val==val else np.nan
        f0=pit.get_value_at_time(tm); f0=f0 if f0==f0 else np.nan
        # median of 3 readings across the middle third of the vowel
        # fixed-time onset readings (same definition for voiced and voiceless): release + 6 ms and + 12 ms
        r6=[F(tb+0.006,n) for n in (1,2,3)]; r12=[F(tb+0.012,n) for n in (2,3)]
        # preceding vowel (VCV carry-over): its midpoint F2/F1
        pv=sg[i-1][2] if sg[i-1][2] in V else ''; pf1=pf2=np.nan; pdur=np.nan
        if pv:
            ps,pe=sg[i-1][0],sg[i-1][1]; pm=0.5*(ps+pe); pdur=(pe-ps)*1000
            pf2=np.nanmedian([F(t,2) for t in (pm-(pe-ps)/6,pm,pm+(pe-ps)/6)]); pf1=np.nanmedian([F(t,1) for t in (pm-(pe-ps)/6,pm,pm+(pe-ps)/6)])
        # VC transition: F2/F3 of the PREVIOUS vowel shortly before its offset (= consonant label start cs)
        vo=[np.nan]*6
        if pv:
            vo=[F(cs-0.020,2),F(cs-0.008,2),F(cs-0.003,2),F(cs-0.020,3),F(cs-0.008,3),F(cs-0.003,3)]
        mids=[tm-(ve-vs)/6,tm,tm+(ve-vs)/6]
        rows.append(dict(sub=u['sub'],name=u['name'],place=c,vowel=v,init=int(init),prev=prev,next=nxt,conf=int(conf),
            f1_r6=r6[0],f2_r6=r6[1],f3_r6=r6[2],f2_r12=r12[0],f3_r12=r12[1],pvowel=pv,vo_f2_20=vo[0],vo_f2_8=vo[1],vo_f2_3=vo[2],vo_f3_20=vo[3],vo_f3_8=vo[4],vo_f3_3=vo[5],pf1=pf1,pf2=pf2,pdur_ms=pdur,clo_ms=(tb-cs)*1000,vdur_ms=(ve-vs)*1000,vot_ms=vot,prevoice_ms=prev_ms,f0=f0,
            f1_on=F(t_on,1),f2_on=F(t_on,2),f3_on=F(t_on,3),
            f1_mid=np.nanmedian([F(t,1) for t in mids]),f2_mid=np.nanmedian([F(t,2) for t in mids]),f3_mid=np.nanmedian([F(t,3) for t in mids])))
    return rows
if __name__=='__main__':
    U=load_all(); lim=int(sys.argv[1]) if len(sys.argv)>1 else len(U)
    allrows=[]
    for j,u in enumerate(U[:lim]):
        allrows+=process(u)
        if j%100==0: print(j,len(allrows),flush=True)
    keys=list(allrows[0].keys())
    with open('/tmp/jsut/an/tokens.csv','w',newline='') as f:
        w=csv.DictWriter(f,keys); w.writeheader(); w.writerows(allrows)
    print('tokens',len(allrows))

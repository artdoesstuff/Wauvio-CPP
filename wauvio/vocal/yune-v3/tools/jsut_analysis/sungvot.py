import pickle, numpy as np, pandas as pd, parselmouth
from scipy.signal import butter, sosfilt
D=pickle.load(open('/tmp/song/an/notes.pkl','rb')); rows=[]
for f,d in D.items():
    s=parselmouth.Sound(f); x=s.values[0]; sr=s.sampling_frequency
    sos=butter(4,2500,'highpass',fs=sr,output='sos'); h=sosfilt(sos,x); hop=int(0.001*sr); n=len(h)//hop
    env=20*np.log10(np.sqrt((h[:n*hop].reshape(n,hop)**2).mean(1))+1e-8)          # dB per ms
    t=d['t']; runs=d['runs']
    for i in range(1,len(runs)):
        te=t[runs[i-1][1]]; ts=t[runs[i][0]]; gap=ts-te
        if not(0.04<=gap<=0.30): continue
        lo=int((te+0.005)*1000); hi=int(ts*1000)
        if hi-lo<20: continue
        seg=env[lo:hi]; dd=np.diff(seg); k=int(np.argmax(dd)); rise=dd[k]
        # a stop: quiet closure before the burst (>= 15 ms below the burst level) and a clear rise
        tb=(lo+k+1)/1000; pre=env[max(lo,lo+k-15):lo+k]; post=env[lo+k+1:lo+k+8]
        if rise<10 or len(pre)<8 or np.max(post)-np.median(pre)<20: continue
        # HF level well above the floor through the gap tail means fricative/affricate: require energy to fall again before voicing OR gap short
        rows.append(dict(song=f.split('/')[-1][:3],gap=gap*1000,closure=(tb-te)*1000,vot=(ts-tb)*1000,rise=rise))
df=pd.DataFrame(rows); df.to_pickle('/tmp/song/an/sungvot.pkl')
d2=df[(df.vot>3)&(df.vot<90)&(df.closure>12)]
print("sung stop-like gaps: found %d (VOT 3-90 ms, closure >12 ms: %d)"%(len(df),len(d2)))
print("  VOT (burst -> voicing):   median %.1f ms, IQR %.1f-%.1f, 5-95%% %.1f-%.1f"%(d2.vot.median(),d2.vot.quantile(.25),d2.vot.quantile(.75),d2.vot.quantile(.05),d2.vot.quantile(.95)))
print("  closure (silent, offset->burst): median %.1f ms, IQR %.1f-%.1f"%(d2.closure.median(),d2.closure.quantile(.25),d2.closure.quantile(.75)))
sp=pd.read_csv('/tmp/jsut/an/tokens.csv'); sp=sp[(sp.place.isin(['p','t','k']))&(sp.conf==1)&(sp.vot_ms>3)&(sp.vot_ms<90)]
for nm,s in (('speech, medial (pooled p/t/k)',sp[sp.init==0]),('speech, word-initial (pooled)',sp[sp.init==1])):
    print(f"  {nm:34s} VOT median {s.vot_ms.median():.1f} ms (IQR {s.vot_ms.quantile(.25):.1f}-{s.vot_ms.quantile(.75):.1f}), n={len(s)}")
print("  engine (V3): pooled medial about 16-17 ms assuming equal p/t/k frequency weights of speech:")
w=sp[sp.init==0].place.value_counts(normalize=True); print("   speech medial place mix:",w.round(2).to_dict(), "-> engine medial VOT weighted mean %.1f ms"%(w.get('p',0)*13+w.get('t',0)*17+w.get('k',0)*28))

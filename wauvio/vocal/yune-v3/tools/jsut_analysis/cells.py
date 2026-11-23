import pandas as pd, numpy as np
GRP={'labial':['b'],'alveolar':['d','t'],'velar':['g','k']}
def cells(d,nm,formant):
    on,mid={2:('f2_r6','f2_mid'),3:('f3_r6','f3_mid')}[formant]; out={}
    for v in 'aiueo':
        s=d[(d.place.isin(GRP[nm]))&(d.vowel==v)]
        r=(s[on]/s[mid]).values
        if len(r)>=10:
            m=np.median(r); mad=1.4826*np.median(np.abs(r-m))+1e-6; r=r[np.abs(r-m)<3.5*mad]
            out[v]=(float(np.median(r)),len(r))
    return out
def load(which):
    d=pd.read_csv('/tmp/jsut/an/tokens_clean2.csv').dropna(subset=['f3_r6','f3_mid'])
    d=d[(d.f3_mid>1800)&(d.f3_mid<4200)&(d.f3_r6>1500)&(d.f3_r6<4500)]
    if which=='fit': return d[d['sub']!='voiceactress100']
    if which=='val': return d[d['sub']=='voiceactress100']
    return d

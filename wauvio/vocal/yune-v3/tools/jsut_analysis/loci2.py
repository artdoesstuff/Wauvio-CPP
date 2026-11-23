import warnings; warnings.filterwarnings('ignore')
import pandas as pd, numpy as np
from scipy.stats import theilslopes
import statsmodels.api as sm
rng=np.random.default_rng(2)
d=pd.read_csv('/tmp/jsut/an/tokens.csv').dropna(subset=['f2_r6','f2_mid','f3_r6','f3_mid']).reset_index(drop=True)
keep=np.ones(len(d),bool)
for v in 'aiueo':
    m=d.vowel==v; med=d.loc[m,'f2_mid'].median(); mad=1.4826*np.median(np.abs(d.loc[m,'f2_mid']-med))
    keep&=~(m&(np.abs(d.f2_mid-med)>3.5*mad))
d=d[keep&(np.abs(d.f2_r6-d.f2_mid)<1500)&(d.f2_r6>400)&(d.f2_r6<3800)].reset_index(drop=True)
print("tokens kept:",len(d))
def ts(x,y):
    s,c,_,_=theilslopes(y,x); r2=np.corrcoef(x,y)[0,1]**2; return s,c,r2
def boot(x,y,n=300):
    o=np.array([theilslopes(y[i],x[i])[:2] for i in (rng.integers(0,len(x),len(x)) for _ in range(n))]); return np.percentile(o[:,0],[2.5,97.5]),np.percentile(o[:,1],[2.5,97.5])
print("\n(1) F2 locus equation, onset = release + 6 ms (same definition voiced/voiceless), Theil-Sen")
print("place     n   slope [95% CI]      intercept [95% CI]    r2")
R1={}
for p in 'bpdtgk':
    s_=d[d.place==p]; x,y=s_.f2_mid.values,s_.f2_r6.values
    s,c,r2=ts(x,y); (sl,sh),(cl,ch)=boot(x,y); R1[p]=(s,c,r2,len(s_))
    print(f"  {p}   {len(s_):5d}  {s:5.2f} [{sl:4.2f},{sh:4.2f}]   {c:6.0f} [{cl:5.0f},{ch:5.0f}]   {r2:4.2f}")
print("\n(2) with VCV carry-over: F2_onset = a*F2_vowel + b*F2_prevVowel + c   (robust Huber fit; tokens preceded by a vowel)")
print("place     n     a (clean locus slope)   b (carry-over)    c       r2")
R2={}
for p in 'bpdtgk':
    s_=d[(d.place==p)&d.pf2.notna()]
    X=sm.add_constant(np.column_stack([s_.f2_mid,s_.pf2])); m=sm.RLM(s_.f2_r6.values,X,M=sm.robust.norms.HuberT()).fit()
    c,a,b=m.params; pred=X@m.params; r2=1-((s_.f2_r6-pred)**2).sum()/((s_.f2_r6-s_.f2_r6.mean())**2).sum()
    R2[p]=(a,b,c,len(s_)); print(f"  {p}   {len(s_):5d}        {a:5.2f}               {b:5.2f}        {c:6.0f}   {r2:4.2f}")
print("\n(3) pooled by place, voiced+voiceless together (do they agree at release+6 ms?)")
for nm,pair in (('labial','bp'),('alveolar','dt'),('velar','gk')):
    a=R1[pair[0]]; b=R1[pair[1]]; print(f"  {nm:9s} voiced {a[0]:.2f}/{a[1]:.0f}   voiceless {b[0]:.2f}/{b[1]:.0f}")
print("\n(4) the speaker's vowel space (mid F2, median):",{v:int(d[d.vowel==v].f2_mid.median()) for v in 'aiueo'})
d.to_csv('/tmp/jsut/an/tokens_clean2.csv',index=False)

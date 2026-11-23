import re, subprocess, numpy as np, sys
from scipy.optimize import minimize
sys.path.insert(0,'/tmp/jsut/an'); from cells import *
PROBE='/tmp/b/locus_probe'; BASE='/tmp/le/lit.wvc'; ORIG='/tmp/le/cur.wvc'
NAME={'labial':['b'],'alveolar':['d','t'],'velar':['g','k']}
fitJ={(nm,f):cells(load('fit'),nm,f) for nm in GRP for f in (2,3)}
valJ={(nm,f):cells(load('val'),nm,f) for nm in GRP for f in (2,3)}
def probe(vals):
    t=open(BASE).read()
    for nm,v in vals.items(): t=re.sub(rf"^locus {nm} .*$",f"locus {nm} {v[0]:.4f} {v[1]:.1f} {v[2]:.4f} {v[3]:.1f}",t,flags=re.M)
    open('/tmp/jsut/an/p.wvc','w').write(t)
    out=subprocess.run([PROBE,'/tmp/jsut/an/p.wvc','6'],capture_output=True,text=True).stdout
    E={}
    for l in out.splitlines():
        ph=l[0]; f2=re.search(r"F2 on:(.*?)\| F3 on:(.*?)\|",l); 
        for f,blk in ((2,f2.group(1)),(3,f2.group(2))):
            for tok in blk.split():
                v,rest=tok.split('=',1); on,mid=[float(x) for x in rest.split('/')]; E[(ph,f,v)]=on/mid
    return E
def eng_cells(E,nm,f): return {v:np.mean([E[(p,f,v)] for p in NAME[nm]]) for v in 'aiueo'}
def sse(E,nm,f,J):
    e=eng_cells(E,nm,f); return sum(np.sqrt(n)*(e[v]-r)**2 for v,(r,n) in J[(nm,f)].items())
def rms(E,nm,f,J):
    e=eng_cells(E,nm,f); c=J[(nm,f)]; w=sum(np.sqrt(n) for r,n in c.values())
    return np.sqrt(sum(np.sqrt(n)*(e[v]-r)**2 for v,(r,n) in c.items())/w)
cur={l.split()[1]:[float(v) for v in l.split()[2:6]] for l in open(BASE) if l.startswith('locus ')}
new={k:list(v) for k,v in cur.items()}
for nm in ('labial','alveolar','velar'):
    for f,idx in ((2,(0,1)),(3,(2,3))):
        def obj(p):
            if not(0<=p[0]<=1.3): return 1e3
            d={k:list(v) for k,v in new.items()}; d[nm][idx[0]]=p[0]; d[nm][idx[1]]=p[1]
            return sse(probe(d),nm,f,fitJ)
        x0=[new[nm][idx[0]],new[nm][idx[1]]]
        r=minimize(obj,x0,method='Nelder-Mead',options=dict(xatol=0.003,fatol=1e-5,maxiter=160,initial_simplex=[x0,[min(1.2,x0[0]+0.15),x0[1]],[x0[0],x0[1]+300]]))
        new[nm][idx[0]],new[nm][idx[1]]=float(r.x[0]),float(r.x[1])
Ec,En=probe(cur),probe(new)
Or={k:[float(v) for v in l.split()[2:6]] for l in open(ORIG) if l.startswith('locus ') for k in [l.split()[1]]}
Eo=probe(Or)
print("weighted RMS error of engine onset ratio vs JSUT cell medians (lower = closer)")
print("                      FIT set (onomatopee+loanword)             HELD-OUT (voiceactress100)")
print("place     formant   hand-set  literature  JSUT-fit     hand-set  literature  JSUT-fit")
for nm in NAME:
    for f in (2,3):
        print(f"{nm:9s}   F{f}      {rms(Eo,nm,f,fitJ):6.3f}    {rms(Ec,nm,f,fitJ):6.3f}    {rms(En,nm,f,fitJ):6.3f}       {rms(Eo,nm,f,valJ):6.3f}    {rms(Ec,nm,f,valJ):6.3f}    {rms(En,nm,f,valJ):6.3f}")
print("\nper-cell F2 (onset/vowel): JSUT fit-set | JSUT held-out | engine literature-fit (shipped) | engine JSUT-fit")
for nm in NAME:
    e0,e1=eng_cells(Ec,nm,2),eng_cells(En,nm,2)
    for v in 'aiueo':
        a=fitJ[(nm,2)].get(v); b=valJ[(nm,2)].get(v)
        print(f"  {nm:8s} {v}:  {a[0] if a else float('nan'):5.2f} (n={a[1] if a else 0:3d}) | {b[0] if b else float('nan'):5.2f} (n={b[1] if b else 0:3d}) | {e0[v]:5.2f} | {e1[v]:5.2f}")
print("\nlocus lines:")
for nm in ('labial','alveolar','palatal','velar'): print("locus %s %.3f %.0f %.3f %.0f"%(nm,*new[nm]))
import json; json.dump(new,open('/tmp/jsut/an/new_loci.json','w'))

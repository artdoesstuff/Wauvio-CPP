import glob, os, re, collections
ROOT='/tmp/jsut'
def parse_lab(path):
    segs=[]
    for line in open(path,encoding='utf8'):
        p=line.split()
        if len(p)<3: continue
        s,e=int(p[0])/1e7,int(p[1])/1e7
        m=re.match(r"[^\^]*\^[^-]*-([^+]*)\+",p[2]); ph=m.group(1) if m else '?'
        segs.append((s,e,ph))
    return segs
def load_all():
    utts=[]
    for sub in ('loanword128','onomatopee300','voiceactress100'):
        for lab in sorted(glob.glob(f'{ROOT}/{sub}/lab/*.lab')):
            name=os.path.basename(lab)[:-4]
            utts.append(dict(sub=sub,name=name,wav=f'{ROOT}/{sub}/wav/{name}.wav',segs=parse_lab(lab)))
    return utts
if __name__=='__main__':
    U=load_all(); print(len(U),'utterances')
    cnt=collections.Counter(); 
    for u in U:
        for s,e,ph in u['segs']: cnt[ph]+=1
    print('phoneme inventory (count):'); print(' '.join(f'{k}:{v}' for k,v in cnt.most_common()))
    V=set('aiueo'); STOP={'b','d','g','p','t','k'}
    cells=collections.defaultdict(int); init=collections.defaultdict(int)
    for u in U:
        sg=u['segs']
        for i in range(1,len(sg)-1):
            c=sg[i][2]; v=sg[i+1][2]
            if c in STOP and v in V:
                cells[(c,v)]+=1
                if sg[i-1][2] in ('sil','pau'): init[(c,v)]+=1
    print('\nstop+vowel tokens (all / after pause):')
    for c in 'bdgptk': print(' ',c,'  '.join(f'{v}:{cells[(c,v)]}/{init[(c,v)]}' for v in 'aiueo'))
    tot=sum(e-s for u in U for s,e,ph in u['segs'] if ph not in('sil','pau'))
    print('\nspeech time in labels: %.1f min'%(tot/60))

# JSUT analysis scripts (research, not part of the C++ library)

These produced the measurements behind the V3 consonant changes (see ../../RESEARCH_NOTES.md). They need the JSUT
subsets loanword128, onomatopee300 and voiceactress100 (wav + `.lab` from github.com/r9y9/jsut-lab) and
`pip install praat-parselmouth scipy pandas statsmodels`. The audio is not included and must not be redistributed.

Paths are hard-coded to `/tmp/jsut` (data) and `/tmp/jsut/an` (outputs): edit `ROOT` in `labs.py` and the paths at the
top of the other scripts. Order: `measure.py` (token table) -> `loci2.py` (regression view) -> `cells.py` + `fit3.py`
(per-cell fit of the engine's loci; needs `tools/locus_probe` built and the voice files named at the top of `fit3.py`).

## JSUT-song scripts (singing)

Data: JSUT-song (27 `.wav`, same singer, 100 bpm click, no lyrics), assumed at `/tmp/song/jsut-song_ver1/child_song/wav`
and outputs in `/tmp/song/an` (edit paths). Order: `notes.py` (pitch tracking + note segmentation) -> `vibrato.py`,
`pitchbehav.py`, `sungvot.py` (measurements) ; `common.py` holds the shared estimators and `engine_vib.py`,
`engine_entry.py`, `engine_nat.py` apply the *same* estimators to renders of the engine (needs `sing_score` built at
`/tmp/b/sing_score_final`) so the comparison is free of analysis bias.

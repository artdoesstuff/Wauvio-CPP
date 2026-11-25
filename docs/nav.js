/* Wauvio docs — sidebar, theme and page helpers.
   The sidebar is built here (not in every page), and its scroll position, open groups and
   search text are remembered between pages so it does not reset when you navigate. */
(function () {
  'use strict';
  var NAV = [{"group": "Overview", "items": [["index.html", "Wauvio documentation"], ["getting-started.html", "Getting started"], ["architecture.html", "Architecture"], ["showcases.html", "Showcases"]]}, {"group": "Guides & examples", "items": [["guide-concepts.html", "Concepts for beginners"], ["guide-core.html", "Core guide"], ["guide-music.html", "Music layer guide"], ["guide-fx.html", "FX guide"], ["guide-midi.html", "MIDI & SoundFont guide"], ["guide-sfx.html", "SFX guide"], ["guide-vocal.html", "Vocal guide"]]}, {"group": "Core engine (wauvio/core)", "items": [["core.html", "Core overview"], ["core-common.html", "common.hpp — buffers, config, panning"], ["core-osc.html", "osc.hpp — oscillators and noise"], ["core-env.html", "env.hpp — envelopes"], ["core-mod.html", "mod.hpp — LFO and automation"], ["core-filter.html", "filter.hpp — filters and EQ"], ["core-synth.html", "synth.hpp — FM and polyphony"], ["core-fx-basic.html", "fx_basic.hpp — basic effects"], ["core-theory.html", "theory.hpp — notes and frequencies"], ["core-seq.html", "seq.hpp — notes, tracks, sequencing"], ["core-io.html", "io.hpp — WAV export and playback"]]}, {"group": "Music layer (wauvio/ext)", "items": [["ext.html", "Music layer overview"], ["ext-core.html", "Core types — notes, instruments, arrangements"], ["ext-modeling.html", "Modeled instruments"], ["ext-sampling.html", "Sampling"], ["ext-render.html", "Rendering and WAV output"], ["ext-genre.html", "Genre system"], ["ext-instruments.html", "Instrument catalog"]]}, {"group": "FX suite (wauvio/fx)", "items": [["fx.html", "FX suite overview"], ["fx-core.html", "Effect, Chain and DSP utilities"], ["fx-eq-dynamics.html", "EQ and dynamics"], ["fx-saturation.html", "Saturation and bitcrushing"], ["fx-modulation-delay-stereo.html", "Modulation, delay and stereo"], ["fx-reverb.html", "Reverb"], ["fx-mastering.html", "Loudness and mastering"], ["fx-mixer.html", "Mixer and bridges"]]}, {"group": "MIDI layer (wauvio/midi)", "items": [["midi.html", "MIDI layer overview"], ["midi-core.html", "MIDI file model"], ["midi-resolution.html", "Instrument resolution and General MIDI map"], ["midi-track.html", "Loading, rendering and writing"], ["midi-genre.html", "MIDI genre system"]]}, {"group": "SoundFont (wauvio/sf2)", "items": [["sf2.html", "SoundFont (SF2) support"]]}, {"group": "SFX engine (wauvio/sfx)", "items": [["sfx.html", "SFX engine overview"], ["sfx-recipes.html", "Recipes, trajectories and modulators"], ["sfx-engine.html", "Render engine"], ["sfx-presets.html", "Presets, Effect and Layered"], ["sfx-params.html", "Parameter dictionary"], ["sfx-tool.html", "wauvio_sfx_tool (command line)"], ["sfx-examples.html", "Examples"]]}, {"group": "SFX preset catalog", "items": [["sfx-catalog.html", "Catalog index"], ["sfx-cat-risers.html", "Risers & Sweeps"], ["sfx-cat-impacts.html", "Impacts & Transitions"], ["sfx-cat-glitch.html", "Glitch & Digital"], ["sfx-cat-textures.html", "Textures & Atmospheres"], ["sfx-cat-bass.html", "Bass FX"], ["sfx-cat-ear-candy.html", "Ear Candy & Ornaments"], ["sfx-cat-vocal.html", "Vocal Production FX"], ["sfx-cat-rhythmic.html", "Rhythmic FX"], ["sfx-cat-percussion.html", "Synthetic Percussion & Cymbals"], ["sfx-cat-experimental.html", "Experimental & Extreme"], ["sfx-cat-tonal.html", "Musical Motion & Tonal FX"], ["sfx-cat-reverses.html", "Reverses, Tails & Transitions"]]}, {"group": "Vocal engine (wauvio/vocal)", "items": [["vocal.html", "Vocal engine overview"], ["vocal-v3.html", "v3 — Singer API"], ["vocal-v3-style.html", "v3 — Style, curves and lanes"], ["vocal-lyrics.html", "Lyric input (kana, romaji, phonemes)"], ["vocal-v3-expression.html", "v3 — Expression, ornaments and events"], ["vocal-v3-voices.html", "v3 — Voices, personas and voicebanks"], ["vocal-v3-synthesis.html", "v3 — Synthesis pipeline"], ["vocal-v3-ensemble-fx.html", "v3 — Ensemble, FX chain and mic"], ["vocal-tools.html", "Vocal tools"], ["vocal-v2.html", "v2 — differences from v1"], ["vocal-v1.html", "v1 — the original singer"]]}];
  var KEY_OPEN = 'wauvio-docs:open', KEY_SCROLL = 'wauvio-docs:scroll', KEY_Q = 'wauvio-docs:q', KEY_THEME = 'wauvio-docs:theme';
  function get(s, k) { try { return s.getItem(k); } catch (e) { return null; } }
  function set(s, k, v) { try { s.setItem(k, v); } catch (e) {} }
  var page = (location.pathname.split('/').pop() || 'index.html') || 'index.html';

  /* ---- theme (applied before first paint by the inline head script as well) ---- */
  function applyTheme(t) { if (t) document.documentElement.setAttribute('data-theme', t); else document.documentElement.removeAttribute('data-theme'); }
  function currentTheme() {
    var t = document.documentElement.getAttribute('data-theme');
    return t || (window.matchMedia && matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light');
  }

  /* ---- top bar ---- */
  var layout = document.querySelector('.layout');
  var bar = document.createElement('header');
  bar.className = 'topbar';
  bar.innerHTML = '<button class="iconbtn" id="menu-btn" aria-label="Toggle navigation">☰</button>' +
    '<a class="brand" href="index.html"><span class="logo"></span>Wauvio Docs</a><span class="spacer"></span>' +
    '<button class="iconbtn" id="theme-btn" aria-label="Toggle theme"></button>';
  document.body.insertBefore(bar, layout);
  var themeBtn = document.getElementById('theme-btn');
  function paintThemeBtn() { themeBtn.textContent = currentTheme() === 'dark' ? '☀' : '☾'; }
  themeBtn.addEventListener('click', function () {
    var t = currentTheme() === 'dark' ? 'light' : 'dark';
    applyTheme(t); set(localStorage, KEY_THEME, t); paintThemeBtn();
  });
  paintThemeBtn();
  document.getElementById('menu-btn').addEventListener('click', function () { document.body.classList.toggle('menu'); });

  /* ---- sidebar ---- */
  var side = document.getElementById('sidebar');
  var saved = {};
  try { saved = JSON.parse(get(localStorage, KEY_OPEN) || '{}'); } catch (e) {}
  var html = '<div class="search"><input id="q" type="search" placeholder="Filter pages…" aria-label="Filter pages" autocomplete="off"></div>';
  NAV.forEach(function (g, gi) {
    var hasCur = g.items.some(function (it) { return it[0] === page; });
    var open = hasCur || saved[g.group] === true || (saved[g.group] === undefined && gi === 0);
    html += '<div class="grp' + (open ? ' open' : '') + '" data-g="' + g.group.replace(/"/g, '&quot;') + '"><button type="button" aria-expanded="' + open + '">' + esc(g.group) + '</button><ul>';
    g.items.forEach(function (it) {
      html += '<li><a href="' + it[0] + '"' + (it[0] === page ? ' class="cur" aria-current="page"' : '') + '>' + esc(it[1]) + '</a></li>';
    });
    html += '</ul></div>';
  });
  html += '<div class="nores" id="nores">No matching pages.</div>';
  side.innerHTML = html;
  function esc(s) { return String(s).replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;'); }

  side.addEventListener('click', function (e) {
    var b = e.target.closest('.grp > button');
    if (b) {
      var g = b.parentNode; g.classList.toggle('open');
      b.setAttribute('aria-expanded', g.classList.contains('open'));
      saved[g.getAttribute('data-g')] = g.classList.contains('open');
      set(localStorage, KEY_OPEN, JSON.stringify(saved));
      return;
    }
    if (e.target.closest('a')) { saveOpen(); set(sessionStorage, KEY_SCROLL, String(side.scrollTop)); }
  });
  /* remember the real open/closed state of every group, so the next page lays out identically */
  function saveOpen() {
    if (q && q.value.trim()) return;   // while filtering, groups are force-opened: do not record that
    side.querySelectorAll('.grp').forEach(function (g) { saved[g.getAttribute('data-g')] = g.classList.contains('open'); });
    set(localStorage, KEY_OPEN, JSON.stringify(saved));
  }

  /* ---- persistent scroll ---- */
  var stored = parseInt(get(sessionStorage, KEY_SCROLL), 10);
  var cur = side.querySelector('a.cur');
  if (!isNaN(stored)) side.scrollTop = stored;
  if (cur) {   // keep the current page visible if it is outside the restored viewport
    var top = cur.offsetTop, h = side.clientHeight;
    if (top < side.scrollTop || top + cur.offsetHeight > side.scrollTop + h) side.scrollTop = Math.max(0, top - h / 3);
  }
  var tick = null;
  side.addEventListener('scroll', function () {
    if (tick) return; tick = setTimeout(function () { tick = null; set(sessionStorage, KEY_SCROLL, String(side.scrollTop)); }, 120);
  });
  window.addEventListener('pagehide', function () { saveOpen(); set(sessionStorage, KEY_SCROLL, String(side.scrollTop)); });

  /* ---- filter (text persists too) ---- */
  var q = document.getElementById('q'), nores = document.getElementById('nores');
  function filter() {
    var t = q.value.trim().toLowerCase(), any = false;
    side.querySelectorAll('.grp').forEach(function (g) {
      var shown = 0;
      g.querySelectorAll('li').forEach(function (li) {
        var m = !t || li.textContent.toLowerCase().indexOf(t) >= 0;
        li.classList.toggle('hide', !m); if (m) shown++;
      });
      g.style.display = shown ? '' : 'none';
      if (t) g.classList.add('open'); else g.classList.toggle('open', saved[g.getAttribute('data-g')] === true || !!g.querySelector('a.cur'));
      if (shown) any = true;
    });
    nores.style.display = any ? 'none' : 'block';
    set(sessionStorage, KEY_Q, q.value);
  }
  q.value = get(sessionStorage, KEY_Q) || '';
  q.addEventListener('input', filter);
  if (q.value) filter();
  document.addEventListener('keydown', function (e) {
    if (e.key === '/' && document.activeElement !== q && !/input|textarea/i.test(document.activeElement.tagName)) { e.preventDefault(); q.focus(); }
  });

  /* ---- page helpers: heading anchors, copy buttons, on-this-page ---- */
  var content = document.querySelector('.content');
  var heads = content ? content.querySelectorAll('h2[id], h3[id], h4[id]') : [];
  content && content.querySelectorAll('h2, h3, h4').forEach(function (h) {
    if (h.closest('.showcase')) return;   // card titles are not page sections
    if (!h.id) h.id = h.textContent.toLowerCase().replace(/[^a-z0-9]+/g, '-').replace(/^-|-$/g, '');
    var a = document.createElement('a'); a.className = 'anchor'; a.href = '#' + h.id; a.textContent = '#'; a.setAttribute('aria-label', 'Link to this section');
    h.insertBefore(a, h.firstChild);
  });
  content && content.querySelectorAll('pre').forEach(function (pre) {
    var b = document.createElement('button'); b.className = 'copy'; b.type = 'button'; b.textContent = 'Copy';
    b.addEventListener('click', function () {
      var text = pre.querySelector('code') ? pre.querySelector('code').innerText : pre.innerText;
      (navigator.clipboard ? navigator.clipboard.writeText(text) : Promise.reject()).then(function () { b.textContent = 'Copied'; }, function () { b.textContent = 'Press Ctrl+C'; });
      setTimeout(function () { b.textContent = 'Copy'; }, 1400);
    });
    pre.appendChild(b);
  });
  var wide = false;
  content && content.querySelectorAll('table').forEach(function (t) {
    var w = document.createElement('div'); w.className = 'tw'; t.parentNode.insertBefore(w, t); w.appendChild(t);
    if (t.querySelectorAll('thead th').length >= 6) wide = true;
  });
  if (wide) content.classList.add('wide');
  var toc = document.getElementById('toc');
  if (toc && content) {
    var hs = Array.prototype.filter.call(content.querySelectorAll('h2'), function (h) { return h.id; });
    if (hs.length > 1) {
      var t = '<h5>On this page</h5>';
      hs.forEach(function (h) { t += '<a href="#' + h.id + '">' + esc(h.textContent.replace(/^#/, '')) + '</a>'; });
      toc.innerHTML = t;
      var links = toc.querySelectorAll('a');
      if ('IntersectionObserver' in window) {
        var io = new IntersectionObserver(function (es) {
          es.forEach(function (en) {
            if (en.isIntersecting) { links.forEach(function (l) { l.classList.toggle('act', l.getAttribute('href') === '#' + en.target.id); }); }
          });
        }, { rootMargin: '-70px 0px -70% 0px' });
        hs.forEach(function (h) { io.observe(h); });
      }
    } else toc.style.display = 'none';
    if (wide) toc.style.display = 'none';
  }
  /* ---- showcases: <article class="showcase" data-yt="VIDEO_ID"> becomes a lazy YouTube embed ---- */
  document.querySelectorAll('.showcase[data-yt]').forEach(function (el) {
    var id = el.getAttribute('data-yt').trim(), start = parseInt(el.getAttribute('data-start'), 10);
    if (!/^[A-Za-z0-9_-]{6,20}$/.test(id)) return;
    var src = 'https://www.youtube-nocookie.com/embed/' + id + '?rel=0' + (start > 0 ? '&start=' + start : '');
    var box = document.createElement('div'); box.className = 'video';
    var f = document.createElement('iframe');
    f.src = src; f.loading = 'lazy'; f.title = (el.querySelector('h3') || {}).textContent || 'YouTube video';
    f.allow = 'accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share';
    f.referrerPolicy = 'strict-origin-when-cross-origin'; f.setAttribute('allowfullscreen', '');
    box.appendChild(f); el.insertBefore(box, el.firstChild);
    var info = el.querySelector('.info');
    if (info && !info.querySelector('.watch')) {
      var a = document.createElement('p'); a.className = 'watch';
      a.innerHTML = '<a href="https://www.youtube.com/watch?v=' + id + '" target="_blank" rel="noopener">Watch on YouTube ↗</a>';
      info.appendChild(a);
    }
  });
  document.querySelectorAll('.showcase-grid').forEach(function (g) {
    var cards = Array.prototype.slice.call(g.querySelectorAll('.showcase'));
    if (!cards.length) { var p = document.createElement('p'); p.className = 'showcase-empty'; p.textContent = 'Videos coming soon.'; g.parentNode.insertBefore(p, g.nextSibling); g.style.display = 'none'; return; }
    /* tag filter built from the cards' tags (shown only when there is something to filter) */
    var tags = {};
    cards.forEach(function (c) {
      var mine = [];
      c.querySelectorAll('.tags span').forEach(function (t) { var k = t.textContent.trim().toLowerCase(); if (k) { mine.push(k); tags[k] = (tags[k] || 0) + 1; } });
      c.setAttribute('data-tags', '|' + mine.join('|') + '|');
    });
    var names = Object.keys(tags).sort();
    if (names.length < 2) return;
    var bar = document.createElement('div'); bar.className = 'filters';
    function chip(label, key) {
      var b = document.createElement('button'); b.type = 'button'; b.textContent = label; b.setAttribute('data-k', key);
      if (key === '') b.className = 'on';
      b.addEventListener('click', function () {
        bar.querySelectorAll('button').forEach(function (x) { x.classList.toggle('on', x === b); });
        cards.forEach(function (c) { c.style.display = (!key || c.getAttribute('data-tags').indexOf('|' + key + '|') >= 0) ? '' : 'none'; });
      });
      bar.appendChild(b);
    }
    chip('All', '');
    names.forEach(function (n) { chip(n, n); });
    g.parentNode.insertBefore(bar, g);
  });
  document.addEventListener('click', function (e) { if (e.target.closest('nav.side a')) document.body.classList.remove('menu'); });
})();
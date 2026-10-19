// PlayGuard's website (site/README.md): the language and theme menus of the
// header, and the home page's first visit in the browser's language.
// The theme is kept as the web simulator keeps it ('pg-theme'), so both agree.
'use strict';

(function () {
  const store = {
    get: (k) => { try { return localStorage.getItem(k); } catch { return null; } },
    set: (k, v) => { try { localStorage.setItem(k, v); } catch { /* storage refused */ } },
  };
  const dark = matchMedia('(prefers-color-scheme: dark)');
  const theme = document.getElementById('site-theme');
  const show = () => {
    const choice = theme.value;
    const shown = choice === 'auto' ? (dark.matches ? 'dark' : 'light') : choice;
    document.documentElement.dataset.theme = shown;
    // The browser's bar the colour of the header (as theme.js).
    for (const m of document.querySelectorAll('meta[name="theme-color"]')) m.content = shown === 'dark' ? '#111d31' : '#ffffff';
  };
  const kept = store.get('pg-theme');
  theme.value = kept === 'light' || kept === 'dark' ? kept : 'auto';
  theme.addEventListener('change', () => { store.set('pg-theme', theme.value); show(); });
  dark.addEventListener('change', show);

  // The home page's tour: it plays by itself unless the visitor asked for
  // less motion or less data, or is on a phone; the button plays or pauses it.
  const video = document.querySelector('.hero-video video');
  const toggle = document.querySelector('.video-toggle');
  if (video && toggle) {
    const label = () => {
      toggle.classList.toggle('playing', !video.paused);
      toggle.setAttribute('aria-label', video.paused ? toggle.dataset.play : toggle.dataset.pause);
    };
    video.addEventListener('play', label);
    video.addEventListener('pause', label);
    toggle.addEventListener('click', () => { if (video.paused) video.play().catch(() => {}); else video.pause(); });
    toggle.hidden = false;
    label();
    const still = matchMedia('(prefers-reduced-motion: reduce)').matches || navigator.connection?.saveData
      || matchMedia('(max-width: 860px)').matches;
    if (!still) video.play().catch(() => {});
  }

  // On a phone or a small tablet (the header on two rows): the header out
  // of the way while scrolling down, back when scrolling up. The way back
  // to the top, far down any page.
  const phone = matchMedia('(max-width: 1080px)');
  const header = document.querySelector('.top');
  const toTop = document.querySelector('.to-top');
  let lastY = scrollY;
  let ticking = false;
  const scrolled = () => {
    ticking = false;
    const y = Math.max(0, scrollY);
    // Not while a field of the header is in use, nor for a few pixels.
    if (Math.abs(y - lastY) < 8) return;
    const away = phone.matches && y > lastY && y > header.offsetHeight && !header.contains(document.activeElement);
    header.classList.toggle('away', away);
    toTop.hidden = y < innerHeight * 2;
    lastY = y;
  };
  addEventListener('scroll', () => { if (!ticking) { ticking = true; requestAnimationFrame(scrolled); } }, { passive: true });
  phone.addEventListener('change', () => header.classList.remove('away'));

  // The header's links on a phone, a row to scroll: its edge fades while
  // there are more of them to scroll to.
  const nav = document.querySelector('.nav');
  const fade = () => nav.classList.toggle('more', nav.scrollLeft + nav.clientWidth < nav.scrollWidth - 2);
  nav.addEventListener('scroll', fade, { passive: true });
  addEventListener('resize', fade);
  fade();

  // The gallery on a phone, a row to scroll: a dot for each screen, the one
  // in view lit.
  const gallery = document.querySelector('.gallery');
  if (gallery && 'IntersectionObserver' in window) {
    const figures = [...gallery.querySelectorAll('figure')];
    const dots = document.createElement('div');
    dots.className = 'gallery-dots';
    dots.setAttribute('aria-hidden', 'true');
    dots.append(...figures.map(() => document.createElement('span')));
    gallery.after(dots);
    const sync = () => { dots.hidden = gallery.scrollWidth <= gallery.clientWidth; };
    addEventListener('resize', sync);
    sync();
    const seen = new IntersectionObserver((entries) => {
      for (const e of entries) {
        if (e.isIntersecting) {
          for (const d of dots.children) d.classList.remove('on');
          dots.children[figures.indexOf(e.target)].classList.add('on');
        }
      }
    }, { root: gallery, threshold: 0.6 });
    for (const f of figures) seen.observe(f);
  }

  // On a phone, the documentation's pages are a row to scroll: this one in view.
  const current = document.querySelector('.doc-nav [aria-current]');
  const pages = current?.closest('.doc-list');
  if (pages && pages.scrollWidth > pages.clientWidth) {
    pages.scrollLeft = current.offsetLeft - (pages.clientWidth - current.offsetWidth) / 2;
  }

  // The documentation's search: docs/search.json (the language's pages, a
  // section an entry), fetched on the first key. Every word typed must be in
  // the section; one in its heading counts more.
  const search = document.querySelector('.doc-search');
  if (search) {
    const input = search.querySelector('input');
    const list = search.querySelector('.doc-results');
    const indexUrl = new URL(input.dataset.index, location.href);
    let index = null;
    const fold = (s) => s.toLowerCase().normalize('NFD').replace(/[\u0300-\u036f]/g, '');
    const show = () => {
      const words = fold(input.value).split(/\s+/).filter(Boolean);
      list.replaceChildren();
      list.hidden = !words.length || !index;
      if (list.hidden) return;
      const hits = [];
      for (const e of index) {
        const heading = fold(`${e.p} ${e.h}`);
        const text = fold(e.t);
        if (!words.every((w) => heading.includes(w) || text.includes(w))) continue;
        hits.push([words.reduce((n, w) => n + (heading.includes(w) ? 3 : 1), 0), e, text.indexOf(words[0])]);
      }
      hits.sort((a, b) => b[0] - a[0]);
      for (const [, e, at] of hits.slice(0, 8)) {
        const a = document.createElement('a');
        a.href = new URL(`../${e.u}`, indexUrl).href;
        const title = document.createElement('strong');
        title.textContent = e.h ? `${e.p} › ${e.h}` : e.p;
        const extract = document.createElement('span');
        const from = Math.max(0, at - 40);
        extract.textContent = (from ? '…' : '') + e.t.slice(from, from + 120) + '…';
        a.append(title, extract);
        const li = document.createElement('li');
        li.append(a);
        list.append(li);
      }
      if (!hits.length) {
        const li = document.createElement('li');
        li.className = 'none';
        li.textContent = input.dataset.none;
        list.append(li);
      }
    };
    input.addEventListener('input', () => {
      if (index) return show();
      fetch(indexUrl).then((r) => r.json()).then((data) => { index = data; show(); }).catch(() => {});
    });
    input.addEventListener('keydown', (e) => {
      if (e.key === 'Escape') { input.value = ''; show(); }
      if (e.key === 'Enter') list.querySelector('a')?.click();
    });
    search.hidden = false;
  }

  const language = document.getElementById('site-language');
  language.addEventListener('change', () => {
    store.set('pg-site-lang', language.selectedOptions[0].dataset.lang);
    location.href = language.value + location.hash;
  });

  const options = [...language.options].map((o) => [o.dataset.lang, o.value]);
  const find = (code) => options.find(([c]) => c.toLowerCase() === code.toLowerCase());
  // The visitor's language: the one chosen before, else the browser's when
  // the site has it (null: English).
  const visitor = () => {
    const chosen = store.get('pg-site-lang');
    if (chosen) return find(chosen);
    for (const tag of navigator.languages || [navigator.language]) {
      const t = tag.toLowerCase();
      const [base, region = ''] = t.split('-');
      let code = null;
      if (base === 'en') return null;
      if (base === 'fr') code = region === 'ca' ? 'fr-CA' : 'fr';
      else if (base === 'es') code = !region || region === 'es' ? 'es' : 'es-419';
      else if (base === 'pt') code = region === 'br' ? 'pt-BR' : 'pt';
      else if (base === 'zh') code = ['tw', 'hk', 'mo', 'hant'].includes(region) || t.includes('hant') ? 'zh-Hant' : 'zh-Hans';
      else if (['de', 'it', 'nl', 'ru', 'ja', 'ko'].includes(base)) code = base;
      if (code) return find(code);
    }
    return null;
  };

  // The page not found (404.html, at any address): its text in the language
  // of the address' folder (/fr/…), else the visitor's.
  if (document.body.classList.contains('not-found-page')) {
    const folder = options.find(([c, v]) => c !== 'en-US' && location.pathname.toLowerCase().startsWith(new URL(v, location.href).pathname.toLowerCase()));
    const code = (folder || visitor() || ['en-US'])[0];
    const block = document.querySelector(`.not-found[data-lang="${code}"]`);
    if (block) {
      for (const b of document.querySelectorAll('.not-found')) b.hidden = b !== block;
      document.documentElement.lang = block.lang;
      document.title = `${block.querySelector('h1').textContent} · PlayGuard`;
    }
    return;
  }

  // The English home page (the site's root), on a first visit from elsewhere:
  // the browser's language when the site has it, unless one was chosen before.
  const html = document.documentElement;
  if (html.dataset.lang !== 'en-US' || !document.body.classList.contains('home')) return;
  if (document.referrer && new URL(document.referrer).origin === location.origin) return;
  const target = visitor();
  if (target && target[0] !== 'en-US') location.replace(target[1] + location.hash);
})();

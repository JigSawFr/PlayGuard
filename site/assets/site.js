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
    document.documentElement.dataset.theme = choice === 'auto' ? (dark.matches ? 'dark' : 'light') : choice;
  };
  const kept = store.get('pg-theme');
  theme.value = kept === 'light' || kept === 'dark' ? kept : 'auto';
  theme.addEventListener('change', () => { store.set('pg-theme', theme.value); show(); });
  dark.addEventListener('change', show);

  const language = document.getElementById('site-language');
  language.addEventListener('change', () => {
    store.set('pg-site-lang', language.selectedOptions[0].dataset.lang);
    location.href = language.value + location.hash;
  });

  // The English home page (the site's root), on a first visit from elsewhere:
  // the browser's language when the site has it, unless one was chosen before.
  const html = document.documentElement;
  if (html.dataset.lang !== 'en-US' || !document.body.classList.contains('home')) return;
  if (document.referrer && new URL(document.referrer).origin === location.origin) return;
  const options = [...language.options].map((o) => [o.dataset.lang, o.value]);
  const find = (code) => options.find(([c]) => c.toLowerCase() === code.toLowerCase());
  const chosen = store.get('pg-site-lang');
  let target = chosen ? find(chosen) : null;
  if (!chosen) {
    for (const tag of navigator.languages || [navigator.language]) {
      const t = tag.toLowerCase();
      const [base, region = ''] = t.split('-');
      let code = null;
      if (base === 'en') break;
      if (base === 'fr') code = region === 'ca' ? 'fr-CA' : 'fr';
      else if (base === 'es') code = !region || region === 'es' ? 'es' : 'es-419';
      else if (base === 'pt') code = region === 'br' ? 'pt-BR' : 'pt';
      else if (base === 'zh') code = ['tw', 'hk', 'mo', 'hant'].includes(region) || t.includes('hant') ? 'zh-Hant' : 'zh-Hans';
      else if (['de', 'it', 'nl', 'ru', 'ja', 'ko'].includes(base)) code = base;
      if (code) { target = find(code); break; }
    }
  }
  if (target && target[0] !== 'en-US') location.replace(target[1] + location.hash);
})();

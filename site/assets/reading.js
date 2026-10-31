// The documentation read on a light ground, when chosen before (site.js's
// "Light reading" button), set before the first paint so the page does not
// flash dark first. A file of its own: the pages' Content-Security-Policy
// runs no inline script.
(function () {
  var light = null;
  try { light = localStorage.getItem('pg-reading'); } catch (e) { /* storage refused */ }
  if (light === 'light') document.documentElement.dataset.reading = 'light';
})();

// The language chosen before boreale-site's lang.js, kept as "pg-site-lang"
// (a code or folder: "fr-CA", "pt-BR", "en-US"): moved once to
// "boreale-lang", the menu's data-lang (the folder, "en-US" for English),
// then removed. Here rather than in site.js: this file runs in <head> before
// lang.js, which reads the choice.
// To delete after 2027-01-31 (workbench plans/playguard/suivi.md).
(function () {
  try {
    var old = localStorage.getItem('pg-site-lang');
    if (old === null) return;
    var f = old.toLowerCase();
    f = f === 'en' || f === 'en-us' ? 'en-US' : f;
    // Only a folder lang.js knows; anything else is dropped.
    var known = ['en-US', 'fr', 'fr-ca', 'de', 'es', 'es-419', 'it', 'nl', 'pt', 'pt-br', 'ru', 'ja', 'ko',
      'zh-hans', 'zh-hant'];
    if (known.indexOf(f) >= 0 && localStorage.getItem('boreale-lang') === null) localStorage.setItem('boreale-lang', f);
    localStorage.removeItem('pg-site-lang');
  } catch (e) { /* storage refused: nothing kept */ }
})();

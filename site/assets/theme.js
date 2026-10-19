// The website's theme before the first paint (site.js keeps it up to date):
// the one chosen on an earlier visit, here or in the simulator, else the
// system's. A file of its own, not inline: the pages' Content-Security-Policy
// runs no inline script.
(function () {
  var c = null;
  try { c = localStorage.getItem('pg-theme'); } catch (e) { /* storage refused */ }
  if (c !== 'light' && c !== 'dark') c = matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  document.documentElement.dataset.theme = c;
  // The browser's bar the colour of the header, in the theme chosen.
  var bar = c === 'dark' ? '#111d31' : '#ffffff';
  var metas = document.querySelectorAll('meta[name="theme-color"]');
  for (var i = 0; i < metas.length; i++) metas[i].content = bar;
})();

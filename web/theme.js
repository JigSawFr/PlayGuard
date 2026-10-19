// The simulator's theme before the first paint (simulator.js keeps it up to
// date): the URL's, else the one chosen on an earlier visit, else the
// system's. A file of its own, not inline: the page's Content-Security-Policy
// runs no inline script.
(function () {
  var choice = new URLSearchParams(location.search).get('theme');
  if (choice !== 'light' && choice !== 'dark') {
    try { choice = localStorage.getItem('pg-theme'); } catch (e) { choice = null; }
  }
  // The choice (the header's icon on a phone), then the theme it gives.
  document.documentElement.dataset.themeChoice = choice === 'light' || choice === 'dark' ? choice : 'auto';
  if (choice !== 'light' && choice !== 'dark') choice = matchMedia('(prefers-color-scheme: dark)').matches ? 'dark' : 'light';
  document.documentElement.dataset.theme = choice;
})();

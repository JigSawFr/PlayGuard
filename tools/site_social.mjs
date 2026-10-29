// The website's preview image in each language (og:image, 1280x640, the
// size of the repository's social preview, branding/social-preview.svg), in
// Boréale's look: the night, an aurora, the mark, the site's title and the
// console's dashboard (dark, as the site shows it) in that language, written as
// <shots-dir>/<language>/social.png next to tools/site_screenshots.mjs'
// screenshots, for tools/build_site.py (SITE_SCREENSHOTS=<shots-dir>). A
// language without its dashboard screenshot gets none: the site keeps the
// repository's preview for it.
// Needs Node and Playwright (Chromium); fonts-noto-cjk for Japanese, Korean
// and Chinese (web.yml installs it).
//   node tools/site_social.mjs <shots-dir> [language...]
import { chromium } from 'playwright';
import { existsSync, readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const [shots, ...only] = process.argv.slice(2);
if (!shots) {
  console.error('usage: node tools/site_social.mjs <shots-dir> [language...]');
  process.exit(2);
}
const LANGUAGES = ['en-US', 'fr', 'fr-CA', 'de', 'es', 'es-419', 'it', 'nl', 'pt', 'pt-BR', 'ru', 'ja', 'ko', 'zh-Hans', 'zh-Hant'];
const escape = (s) => s.replace(/\{\{[^}]*\}\}/g, '').replace(/[&<>"]/g, (c) => `&#${c.charCodeAt(0)};`);
// The page is set, not loaded: its files as data: URLs.
const data = (file, type) => `data:${type};base64,${readFileSync(file).toString('base64')}`;
const sora = data(join(root, 'site/assets/fonts/sora.woff2'), 'font/woff2');
const figtree = data(join(root, 'site/assets/fonts/figtree.woff2'), 'font/woff2');
const mono = data(join(root, 'site/assets/fonts/jetbrains-mono.woff2'), 'font/woff2');
const mark = data(join(root, 'branding/mark.svg'), 'image/svg+xml');
// The night's stars, the same at each run.
let seed = 7;
const random = () => (seed = (seed * 16807) % 2147483647) / 2147483647;
const stars = Array.from({ length: 70 }, () =>
  `<i style="left:${(random() * 1280).toFixed(0)}px;top:${(random() * 300).toFixed(0)}px;opacity:${(random() * 0.6 + 0.2).toFixed(2)}"></i>`).join('');

const browser = await chromium.launch({ executablePath: process.env.CHROMIUM_PATH || undefined });
const page = await browser.newPage({ viewport: { width: 1280, height: 640 }, deviceScaleFactor: 1 });
let count = 0;
for (const lang of only.length ? only : LANGUAGES) {
  // The dashboard as the site shows it, in the console's dark theme.
  const dashboard = [join(shots, lang, 'dashboard-dark.webp'), join(shots, lang, 'dashboard.webp')].find(existsSync);
  if (!dashboard) continue;
  const t = JSON.parse(readFileSync(join(root, 'site', 'i18n', `${lang}.json`), 'utf8'));
  // The title's end in the aurora's gradient, as on the home page (hero_em).
  let title = escape(t.hero_title);
  const em = t.hero_em && escape(t.hero_em);
  if (em && title.includes(em)) title = title.replace(em, `<em>${em}</em>`);
  // Boréale's look (boreale/branding/README.md): the night, an aurora, the
  // mark and the name, one image of the console.
  await page.setContent(`<!doctype html><html lang="${lang}"><head><meta charset="utf-8"><style>
@font-face { font-family: Sora; src: url("${sora}") format("woff2"); font-weight: 100 900; }
@font-face { font-family: Figtree; src: url("${figtree}") format("woff2"); font-weight: 300 900; }
@font-face { font-family: "JetBrains Mono"; src: url("${mono}") format("woff2"); font-weight: 100 800; }
html, body { margin: 0; width: 1280px; height: 640px; overflow: hidden; color: #e8eef7; background: #0b1424;
  font-family: Figtree, "Noto Sans", "Noto Sans CJK JP", sans-serif; }
body { position: relative; }
.sky { position: absolute; inset: 0; filter: blur(34px);
  background: radial-gradient(70% 34% at 30% 22%, rgb(0 255 204 / 0.30), transparent 70%),
    radial-gradient(60% 30% at 70% 14%, rgb(51 187 255 / 0.28), transparent 70%),
    radial-gradient(55% 28% at 88% 6%, rgb(138 92 255 / 0.30), transparent 70%); }
.stars i { position: absolute; width: 2px; height: 2px; border-radius: 50%; background: #e8eef7; }
.by { position: absolute; top: 34px; right: 40px; padding: 10px 18px; border: 1px solid #22324f; border-radius: 999px;
  background: rgb(14 26 51 / 0.85); font: 400 20px Figtree, sans-serif; color: #a9b6cc; }
.by b { font-family: Sora, sans-serif; color: #e8eef7; }
.text { position: absolute; left: 72px; top: 0; bottom: 0; width: 560px; display: flex; flex-direction: column; justify-content: center; }
.brand { display: flex; align-items: center; gap: 18px; font: 700 46px Sora, sans-serif; letter-spacing: -1px; }
.brand span::after { content: ""; display: block; width: 72px; height: 6px; margin-top: 6px; border-radius: 3px;
  background: linear-gradient(90deg, #00ffcc, #33bbff 60%, #8a5cff); }
h1 { font: 600 44px/1.15 Sora, "Noto Sans CJK JP", sans-serif; margin: 36px 0 18px; letter-spacing: -1px; text-wrap: balance; }
h1 em { font-style: normal; background: linear-gradient(100deg, #00ffcc, #33bbff 55%, #8a5cff); -webkit-background-clip: text; background-clip: text; color: transparent; }
p { font: 500 21px/1.45 "JetBrains Mono", "Noto Sans CJK JP", monospace; color: #a9b6cc; margin: 0; }
img.shot { position: absolute; left: 676px; top: 150px; width: 680px; border-radius: 14px; border: 1px solid rgb(232 238 247 / 0.14);
  box-shadow: 0 30px 80px -20px rgb(0 0 0 / 0.8), 0 0 0 6px rgb(14 26 51 / 0.6), 0 0 90px -20px rgb(51 187 255 / 0.45); }
</style></head><body>
<div class="sky"></div><div class="stars">${stars}</div>
<div class="by">by <b>Boréale</b></div>
<div class="text">
  <div class="brand"><img src="${mark}" width="64" height="64" alt=""><span>PlayGuard</span></div>
  <h1>${title}</h1>
  <p>${escape(t.hero_sub)}</p>
</div>
<img class="shot" src="${data(dashboard, 'image/webp')}" alt="">
</body></html>`, { waitUntil: 'load' });
  await page.evaluate(() => document.fonts.ready);
  // A title too long for its lines: smaller until the text fits above the bottom.
  await page.evaluate(() => {
    const h1 = document.querySelector('h1');
    const text = document.querySelector('.text');
    const tall = () => text.lastElementChild.getBoundingClientRect().bottom - text.firstElementChild.getBoundingClientRect().top;
    for (let size = 44; size > 30 && tall() > 520; size -= 2) h1.style.fontSize = `${size - 2}px`;
  });
  await page.screenshot({ path: join(shots, lang, 'social.png'), type: 'png' });
  count++;
}
await browser.close();
console.log(`${count} preview images in ${shots}`);

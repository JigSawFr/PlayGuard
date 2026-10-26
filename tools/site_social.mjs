// The website's preview image in each language (og:image, 1280x640, the
// size of the repository's social preview, branding/social-preview.svg):
// the site's title and the console's dashboard in that language, written as
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
const mark = data(join(root, 'branding/mark.svg'), 'image/svg+xml');

const browser = await chromium.launch({ executablePath: process.env.CHROMIUM_PATH || undefined });
const page = await browser.newPage({ viewport: { width: 1280, height: 640 }, deviceScaleFactor: 1 });
let count = 0;
for (const lang of only.length ? only : LANGUAGES) {
  const dashboard = join(shots, lang, 'dashboard.webp');
  if (!existsSync(dashboard)) continue;
  const t = JSON.parse(readFileSync(join(root, 'site', 'i18n', `${lang}.json`), 'utf8'));
  const features = ['f_timer_title', 'f_restrictions_title', 'f_activity_title', 'f_console_title'].map((k) => escape(t[k])).join(' · ');
  await page.setContent(`<!doctype html><html lang="${lang}"><head><meta charset="utf-8"><style>
@font-face { font-family: Sora; src: url("${sora}") format("woff2"); font-weight: 100 900; }
html, body { margin: 0; width: 1280px; height: 640px; overflow: hidden; color: #e8eef7;
  background: radial-gradient(110% 90% at 0% 100%, rgb(0 255 204 / 0.14), transparent 60%),
    radial-gradient(90% 80% at 100% 0%, rgb(138 92 255 / 0.16), transparent 60%), linear-gradient(#0a1020, #141b2e);
  font-family: Sora, "Noto Sans", "Noto Sans CJK JP", sans-serif; }
body { border-top: 8px solid; border-image: linear-gradient(90deg, #00ffcc, #33bbff 55%, #8a5cff) 1; box-sizing: border-box; position: relative; }
.text { position: absolute; left: 72px; top: 8px; bottom: 0; width: 560px; display: flex; flex-direction: column; justify-content: center; }
.brand { display: flex; align-items: center; gap: 16px; font-size: 44px; font-weight: 700; letter-spacing: -1px; }
.brand b { color: inherit; }
h1 { font-size: 46px; line-height: 1.18; font-weight: 700; margin: 40px 0 20px; letter-spacing: -0.5px; text-wrap: balance; }
p { font-size: 25px; line-height: 1.4; color: #a9b6cc; margin: 0 0 26px; }
.features { font-size: 19px; color: #33bbff; }
img.shot { position: absolute; left: 676px; top: 140px; width: 680px; border-radius: 14px;
  box-shadow: 0 24px 60px rgba(0, 0, 0, .55); border: 1px solid #1f3355; }
</style></head><body>
<div class="text">
  <div class="brand"><img src="${mark}" width="56" height="56" alt=""><span>Play<b>Guard</b></span></div>
  <h1>${escape(t.hero_title)}</h1>
  <p>${escape(t.hero_sub)}</p>
  <div class="features">${features}</div>
</div>
<img class="shot" src="${data(dashboard, 'image/webp')}" alt="">
</body></html>`, { waitUntil: 'load' });
  await page.evaluate(() => document.fonts.ready);
  // A title too long for its lines: smaller until the text fits above the bottom.
  await page.evaluate(() => {
    const h1 = document.querySelector('h1');
    const text = document.querySelector('.text');
    const tall = () => text.lastElementChild.getBoundingClientRect().bottom - text.firstElementChild.getBoundingClientRect().top;
    for (let size = 46; size > 30 && tall() > 520; size -= 2) h1.style.fontSize = `${size - 2}px`;
  });
  await page.screenshot({ path: join(shots, lang, 'social.png'), type: 'png' });
  count++;
}
await browser.close();
console.log(`${count} preview images in ${shots}`);

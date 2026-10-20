// Renders branding/*.svg to the raster files the build and the stores use:
//   icon.jpg (256x256, NRO icon), images/store/icon.png (256x150),
//   images/store/banner.png (848x208), images/social-preview.png (1280x640, the
//   repository's social preview, uploaded by hand in Settings › General),
//   resources/img/wordmark.png (720x176, transparent: the loading screen's),
//   images/apple-touch-icon.png (180x180: an iPhone's home screen, the website's and the simulator's),
//   images/icon-192.png and images/icon-512.png (Android's home screen and splash, from both manifests;
//   maskable as they are: the logo is inside the middle 80 % Android may crop to).
// and images/splash/ (what an iPhone shows while the simulator, opened from its
// home screen, starts: SPLASH below, each iPhone's screen in both orientations;
// web/index.html links each with its media query).
// Needs Node + Playwright (Chromium); loads the Sora font from Google Fonts
// (the splash screens: site/assets/fonts/sora.woff2).
//   node tools/render_branding.mjs            everything
//   node tools/render_branding.mjs splash     the splash screens only
import { chromium } from 'playwright';
import { execFileSync } from 'node:child_process';
import { readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const jobs = [
  { svg: 'branding/icon.svg',       out: 'icon.jpg',                w: 256, h: 256, type: 'jpeg' },
  { svg: 'branding/store-icon.svg', out: 'images/store/icon.png',   w: 256, h: 150, type: 'png' },
  { svg: 'branding/banner.svg',     out: 'images/store/banner.png', w: 848, h: 208, type: 'png' },
  { svg: 'branding/social-preview.svg', out: 'images/social-preview.png', w: 1280, h: 640, type: 'png' },
  { svg: 'branding/wordmark.svg',   out: 'resources/img/wordmark.png', w: 720, h: 176, type: 'png', transparent: true },
  { svg: 'branding/icon.svg',       out: 'images/apple-touch-icon.png', w: 180, h: 180, type: 'png' },
  { svg: 'branding/icon.svg',       out: 'images/icon-192.png',     w: 192, h: 192, type: 'png' },
  { svg: 'branding/icon.svg',       out: 'images/icon-512.png',     w: 512, h: 512, type: 'png' },
];

// Each iPhone's screen in CSS pixels and its pixel ratio (iOS needs the
// exact one, or shows a blank screen): the ones iOS 16 and later run on.
export const SPLASH = [
  [375, 667, 2],   // iPhone SE (2nd, 3rd), 6s, 7, 8
  [414, 736, 3],   // 6s Plus, 7 Plus, 8 Plus
  [375, 812, 3],   // X, XS, 11 Pro, 12 mini, 13 mini
  [414, 896, 2],   // XR, 11
  [414, 896, 3],   // XS Max, 11 Pro Max
  [390, 844, 3],   // 12, 12 Pro, 13, 13 Pro, 14, 16e
  [428, 926, 3],   // 12 Pro Max, 13 Pro Max, 14 Plus
  [393, 852, 3],   // 14 Pro, 15, 15 Pro, 16
  [430, 932, 3],   // 14 Pro Max, 15 Plus, 15 Pro Max, 16 Plus
  [402, 874, 3],   // 16 Pro, 17, 17 Pro
  [420, 912, 3],   // Air
  [440, 956, 3],   // 16 Pro Max, 17 Pro Max
];
export const splashName = (w, h, r, landscape) => `iphone-${w}x${h}@${r}-${landscape ? 'landscape' : 'portrait'}.png`;

const browser = await chromium.launch(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {});
const splashOnly = process.argv[2] === 'splash';
const page = await browser.newPage({ deviceScaleFactor: 1 });
for (const job of splashOnly ? [] : jobs) {
  const svg = readFileSync(join(root, job.svg), 'utf8');
  await page.setViewportSize({ width: job.w, height: job.h });
  await page.setContent(`<!doctype html><html><head>
<link href="https://fonts.googleapis.com/css2?family=Sora:wght@400;700&display=swap" rel="stylesheet">
<style>html,body{margin:0;overflow:hidden}svg{display:block;width:100vw;height:100vh}</style></head><body>${svg}</body></html>`,
    { waitUntil: 'networkidle' });
  await page.evaluate(async () => {
    await document.fonts.load('700 30px Sora');
    await document.fonts.load('400 20px Sora');
    await document.fonts.ready;
  });
  const ok = await page.evaluate(() => document.fonts.check('700 30px Sora'));
  if (!ok) throw new Error('Sora font did not load');
  await page.screenshot({ path: join(root, job.out), type: job.type, quality: job.type === 'jpeg' ? 95 : undefined,
                          omitBackground: !!job.transparent, clip: { x: 0, y: 0, width: job.w, height: job.h } });
  console.log(`${job.svg} -> ${job.out}`);
}

// The splash screens: the simulator's dark ground (web/simulator.css' --bg),
// the icon and the wordmark in the middle, as PlayGuard's loading screen.
const sora = readFileSync(join(root, 'site/assets/fonts/sora.woff2')).toString('base64');
const icon = readFileSync(join(root, 'branding/icon.svg'), 'utf8');
for (const [w, h, ratio] of SPLASH) {
  const splash = await browser.newPage({ deviceScaleFactor: ratio });
  for (const landscape of [false, true]) {
    await splash.setViewportSize(landscape ? { width: h, height: w } : { width: w, height: h });
    await splash.setContent(`<!doctype html><html><head><style>
@font-face { font-family: Sora; src: url(data:font/woff2;base64,${sora}) format('woff2'); font-weight: 100 800; }
html, body { margin: 0; height: 100%; background: #0b1424; }
body { --s: ${Math.round(Math.min(w, h) * 0.26)}px; display: flex; flex-direction: column; align-items: center; justify-content: center; gap: calc(var(--s) * 0.16); }
.icon { width: var(--s); height: var(--s); border-radius: 22%; overflow: hidden; }
.icon svg { display: block; width: 100%; height: 100%; }
.word { font: 700 calc(var(--s) * 0.3)/1 Sora; letter-spacing: -0.02em; color: #E8EEF7; }
.word b { color: #2EC4A6; font-weight: 700; }
</style></head><body><div class="icon">${icon}</div><div class="word">Play<b>Guard</b></div></body></html>`);
    await splash.evaluate(() => document.fonts.ready);
    const out = join('images/splash', splashName(w, h, ratio, landscape));
    await splash.screenshot({ path: join(root, out), type: 'png' });
    // A flat ground and two marks: 96 colours look the same, at a third of
    // the size (ImageMagick, when it is there).
    try {
      execFileSync('convert', [join(root, out), '-dither', 'None', '-colors', '96', `PNG8:${join(root, out)}`]);
    } catch { /* no ImageMagick: kept as rendered */ }
    console.log(`splash -> ${out}`);
  }
  await splash.close();
}
await browser.close();

// The website's screenshots in each of the console's languages (site/README.md):
// the web simulator (web/README.md, `make web`) in a headless Chromium,
// written as <out-dir>/<language>/<screen>.webp for tools/build_site.py to use
// (SITE_SCREENSHOTS=<out-dir>). The same console as the desktop smoke test's
// (tools/desktop_smoke.py): Thursday 8 October 2026, 16:00 UTC, the focus
// without its glow, a release's version badge.
// Needs Node and Playwright (Chromium).
//   node tools/site_screenshots.mjs build-web/simulator <out-dir> [version] [language...]
// (every language when none is named; the version is the header's badge, 1.2.0).
import { chromium } from 'playwright';
import { createServer } from 'node:http';
import { mkdir, readFile, writeFile } from 'node:fs/promises';
import { extname, join, normalize } from 'node:path';

const [simulator, out, version = '', ...only] = process.argv.slice(2);
if (!simulator || !out) {
  console.error('usage: node tools/site_screenshots.mjs <simulator-dir> <out-dir> [version] [language...]');
  process.exit(2);
}
const LANGUAGES = ['en-US', 'fr', 'fr-CA', 'de', 'es', 'es-419', 'it', 'nl', 'pt', 'pt-BR', 'ru', 'ja', 'ko', 'zh-Hans', 'zh-Hant'];
// The screens of the home page's gallery (build_site.py's SCREENSHOTS), a few
// per start of PlayGuard (a start takes the longest): the tab it opens on,
// then each screen with the keys to it from the one before.
const RUNS = [
  // The tabs, down the sidebar.
  { tab: '', screens: [['dashboard'], ['play_timer', 'ArrowDown'], ['activity', 'ArrowDown'],
    ['restrictions', 'ArrowDown'], ['security', 'ArrowDown'], ['console', 'ArrowDown']] },
  // Not past History: on a busy machine it can take long enough to build
  // that a press seems missed, and pressed again goes one tab too far.
  { tab: 'preferences', screens: [['preferences']] },
  // The week chart on today, tomorrow, its editor (its picker open), the picker closed.
  { tab: 'play_timer', screens: [['per_day', 'ArrowRight', 'ArrowRight', 'Enter', 'Escape']] },
  // Console's first row, Network clock.
  { tab: 'console', screens: [['clock', 'ArrowRight', 'Enter']] },
];
const NOW = 1791475200;   // tools/desktop_smoke.py's PLAYGUARD_SIM_NOW

// The simulator needs threads, so a cross-origin isolated page (web/serve.py's headers).
const TYPES = { '.html': 'text/html', '.js': 'text/javascript', '.css': 'text/css', '.json': 'application/json',
  '.wasm': 'application/wasm', '.data': 'application/octet-stream', '.png': 'image/png', '.svg': 'image/svg+xml' };
const server = createServer(async (req, res) => {
  // normalize() of an absolute path never climbs above it: the files stay the simulator's.
  let path = normalize(decodeURIComponent(new URL(req.url, 'http://x').pathname));
  if (path.endsWith('/')) path += 'index.html';
  try {
    const body = await readFile(join(simulator, path));
    res.writeHead(200, { 'Content-Type': TYPES[extname(path)] ?? 'application/octet-stream',
      'Cross-Origin-Opener-Policy': 'same-origin', 'Cross-Origin-Embedder-Policy': 'require-corp' });
    res.end(body);
  } catch {
    res.writeHead(404).end();
  }
});
await new Promise((r) => server.listen(0, '127.0.0.1', r));
const base = `http://127.0.0.1:${server.address().port}/`;
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

// The screen above the footer, small and grey: enough to tell whether a
// press did anything. SwiftShader draws the focus' shadow a little
// differently from one frame to the next: a few pixels apart is the same.
const screen = (page) => page.evaluate(() => {
  const c = document.createElement('canvas');
  c.width = 160;
  c.height = 80;
  const g = c.getContext('2d');
  g.drawImage(document.getElementById('canvas'), 0, 0, 1280, 640, 0, 0, 160, 80);
  const rgba = g.getImageData(0, 0, 160, 80).data;
  return Array.from({ length: 160 * 80 }, (_, i) => (rgba[4 * i] + rgba[4 * i + 1] + rgba[4 * i + 2]) / 3);
});
const differ = (a, b) => a.filter((v, i) => Math.abs(v - b[i]) > 16).length > 8;

// The screen once it stopped moving: the same twice, 0.4 s apart (10 s at most).
async function still(page) {
  let last = await screen(page);
  for (let waited = 0; waited < 10000; waited += 400) {
    await sleep(400);
    const now = await screen(page);
    if (!differ(now, last)) return now;
    last = now;
  }
  return last;
}

// One start of PlayGuard in `lang` and its screens. A browser of its own:
// SwiftShader's WebGL can crash one (its GPU process), which then opens no
// page; it takes nothing else along.
async function run(lang, { tab, screens }) {
  const browser = await chromium.launch({
    executablePath: process.env.CHROMIUM_PATH || undefined,
    // WebGL 2 without a GPU; and no proxy for a page served from this machine.
    args: ['--no-proxy-server', '--ignore-gpu-blocklist', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'],
  });
  try {
    const context = await browser.newContext({ viewport: { width: 1600, height: 1000 }, timezoneId: 'UTC', colorScheme: 'light' });
    const page = await context.newPage();
    // The footer's clock follows the browser's: from the smoke test's 16:00 too.
    await page.clock.install({ time: new Date(NOW * 1000) });
    await page.clock.resume();
    const query = new URLSearchParams({ lang, theme: 'light', still_focus: '1', now: String(NOW) });
    if (tab) query.set('tab', tab);
    if (version) query.set('version', `${version}:`);
    await page.goto(`${base}?${query}`);
    await page.waitForFunction(() => document.getElementById('overlay')?.hidden, null, { timeout: 120000 });
    // PlayGuard's loading screen is dark, its light theme is not: wait for the
    // first screen (a corner of the header), then for its icons.
    await page.waitForFunction(() => {
      const c = document.createElement('canvas');
      c.width = c.height = 1;
      const g = c.getContext('2d');
      g.drawImage(document.getElementById('canvas'), 8, 8, 1, 1, 0, 0, 1, 1);
      return g.getImageData(0, 0, 1, 1).data[0] > 128;
    }, null, { timeout: 120000, polling: 250 });
    await sleep(1000);
    await page.focus('#canvas');
    for (const [name, ...keys] of screens) {
      // A busy machine can miss a press: each one must change the screen
      // (above the footer, whose clock always does), else it is pressed
      // again, after 8 s (pressed again too soon, the focus would go one
      // further); and each
      // waits for a still screen (an editor that opens a picker, borealis'
      // animations), or it could land in the middle.
      for (const key of keys) {
        const before = await still(page);
        let changed = false;
        for (let attempt = 0; attempt < 3 && !changed; attempt++) {
          await page.keyboard.press(key);
          for (let waited = 0; waited < 8000 && !changed; waited += 250) {
            await sleep(250);
            changed = differ(await screen(page), before);
          }
        }
        if (!changed) throw new Error(`${lang}/${name}: ${key} changed nothing`);
      }
      await still(page);
      const url = await page.evaluate(() => document.getElementById('canvas').toDataURL('image/webp', 0.9));
      if (!url.startsWith('data:image/webp')) throw new Error(`${lang}/${name}: no WebP from the canvas`);
      await mkdir(join(out, lang), { recursive: true });
      await writeFile(join(out, lang, `${name}.webp`), Buffer.from(url.split(',')[1], 'base64'));
    }
  } finally {
    await browser.close().catch(() => {});
  }
}

// Three tries: a start that never finished, or a browser that crashed, is taken again.
async function runAgain(lang, screens) {
  for (let attempt = 1; ; attempt++) {
    try {
      return await run(lang, screens);
    } catch (e) {
      if (attempt === 3) throw e;
      console.warn(`${lang}/${screens.screens[0][0]}: ${String(e.message).split('\n')[0]} (try ${attempt + 1})`);
    }
  }
}

// A few languages at once (SITE_SCREENSHOT_JOBS, 3 by default): each page
// runs PlayGuard on the CPU (no GPU, WebGL through SwiftShader).
const queue = [...(only.length ? only : LANGUAGES)];
const jobs = Math.max(1, Number(process.env.SITE_SCREENSHOT_JOBS) || 3);
let count = 0;
try {
  await Promise.all(Array.from({ length: jobs }, async () => {
    for (let lang; (lang = queue.shift());) {
      for (const screens of RUNS) {
        await runAgain(lang, screens);
        count += screens.screens.length;
      }
      console.log(`${lang}: done`);
    }
  }));
} finally {
  server.close();
}
console.log(`${count} screenshots in ${out}`);

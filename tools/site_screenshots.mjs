// The website's screenshots in each of the console's languages (site/README.md):
// the web simulator (web/README.md, `make web`) in a headless Chromium, one
// screen at a time, written as <out-dir>/<language>/<screen>.webp for
// tools/build_site.py to use (SITE_SCREENSHOTS=<out-dir>). The same console as
// the desktop smoke test's (tools/desktop_smoke.py): Thursday 8 October 2026,
// 16:00 UTC, the focus without its glow, a release's version badge.
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
// Each screen of the home page's gallery (build_site.py's SCREENSHOTS): the tab
// PlayGuard opens on, then the keys to the screen from there.
const SCREENS = {
  dashboard: { tab: '' },
  play_timer: { tab: 'play_timer' },
  // The week chart on today, tomorrow, its editor (its picker open), the picker closed.
  per_day: { tab: 'play_timer', keys: ['ArrowRight', 'ArrowRight', 'Enter', 'Escape'] },
  activity: { tab: 'activity' },
  restrictions: { tab: 'restrictions' },
  security: { tab: 'security' },
  console: { tab: 'console' },
  // Console's first row, Network clock.
  clock: { tab: 'console', keys: ['ArrowRight', 'Enter'] },
  preferences: { tab: 'preferences' },
};
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

// The browser, launched again if it went (a crashed GPU process can take it along).
let browser, context, launching;
async function ready() {
  while (launching) await launching;
  if (!browser?.isConnected()) {
    launching = (async () => {
      browser = await chromium.launch({
        executablePath: process.env.CHROMIUM_PATH || undefined,
        // WebGL 2 without a GPU; and no proxy for a page served from this machine.
        args: ['--no-proxy-server', '--ignore-gpu-blocklist', '--use-angle=swiftshader', '--enable-unsafe-swiftshader'],
      });
      context = await browser.newContext({ viewport: { width: 1600, height: 1000 }, timezoneId: 'UTC', colorScheme: 'light' });
    })().finally(() => { launching = null; });
    await launching;
  }
  return context;
}
const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

async function capture(lang, name, { tab, keys = [] }) {
  const page = await (await ready()).newPage();
  try {
    await shoot(page, lang, name, tab, keys);
  } finally {
    await page.close().catch(() => {});
  }
}

// Three tries: a page that never finished loading, or a browser that went, is taken again.
async function captureAgain(lang, name, screen) {
  for (let attempt = 1; ; attempt++) {
    try {
      return await capture(lang, name, screen);
    } catch (e) {
      if (attempt === 3) throw e;
      console.warn(`${lang}/${name}: ${String(e.message).split('\n')[0]} (try ${attempt + 1})`);
    }
  }
}

async function shoot(page, lang, name, tab, keys) {
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
  await sleep(1500);
  await page.focus('#canvas');
  // borealis' animations take up to 0.4 s; a press then waits for them.
  for (const key of keys) {
    await page.keyboard.press(key);
    await sleep(900);
  }
  await sleep(600);
  const url = await page.evaluate(() => document.getElementById('canvas').toDataURL('image/webp', 0.9));
  if (!url.startsWith('data:image/webp')) throw new Error(`${lang}/${name}: no WebP from the canvas`);
  await mkdir(join(out, lang), { recursive: true });
  await writeFile(join(out, lang, `${name}.webp`), Buffer.from(url.split(',')[1], 'base64'));
}

// A few languages at once (SITE_SCREENSHOT_JOBS, 3 by default): each page
// runs PlayGuard on the CPU (no GPU, WebGL through SwiftShader).
const queue = [...(only.length ? only : LANGUAGES)];
const jobs = Math.max(1, Number(process.env.SITE_SCREENSHOT_JOBS) || 3);
let count = 0;
try {
  await Promise.all(Array.from({ length: jobs }, async () => {
    for (let lang; (lang = queue.shift());) {
      for (const [name, screen] of Object.entries(SCREENS)) {
        await captureAgain(lang, name, screen);
        count++;
      }
      console.log(`${lang}: ${Object.keys(SCREENS).length} screens`);
    }
  }));
} finally {
  await browser?.close();
  server.close();
}
console.log(`${count} screenshots in ${out}`);

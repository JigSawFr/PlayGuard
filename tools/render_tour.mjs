// Renders branding/tour.html, frame by frame, to images/video/playguard-tour.mp4
// (1920x1080, 30 fps, with its music), images/video/playguard-teaser.mp4 (the
// same, tour.html?short: under half a minute) and images/video/playguard-tour.gif
// (640 px, 10 fps, silent: under the 10 MB GitHub shows inline): the README
// shows the GIF, which GitHub plays inline (its mobile app too), and links it
// to itself at full size: GitHub never plays an MP4 from the repository (its
// file view says it is too big, its raw link downloads it).
// The GIF is rendered with tour.html?still (no drifting glow, no zoom on the
// screenshots), so most of each frame repeats the last one and it stays small.
// The texts are the site's catalog (site/i18n/<language>.json, its tour_*
// keys), the music branding/tour-music.m4a (branding/tour-music.txt), the
// clips images/screenshots/clips/*.webm (tools/readme_screenshots.py clips).
// Needs Node + Playwright (Chromium) and ffmpeg; the Sora font is the sites' own (library/boreale-site/assets/fonts/).
//   node tools/render_tour.mjs [--gif]
// The website's tour in the console's other languages: their texts (the
// site's catalogs, site/i18n/) over their screenshots (tools/site_screenshots.mjs)
// and the gate's in that language (make gate-preview, when a C compiler is
// there), <shots-dir>/<language>/tour.mp4 (1280x720) for tools/build_site.py
// (SITE_SCREENSHOTS=<shots-dir>); a scene without its screenshot in that
// language is left out, a language without its screenshots gets none (the
// site shows the English tour). Fonts-noto-cjk for Japanese, Korean and Chinese.
//   node tools/render_tour.mjs <shots-dir> [language...]
import { chromium } from 'playwright';
import { spawn, spawnSync } from 'node:child_process';
import { existsSync, mkdtempSync, readFileSync, readdirSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const mp4 = join(root, 'images/video/playguard-tour.mp4');
const teaser = join(root, 'images/video/playguard-teaser.mp4');
const gif = join(root, 'images/video/playguard-tour.gif');
const music = join(root, 'branding/tour-music.m4a');
const tour = pathToFileURL(join(root, 'branding/tour.html')).href;

const [arg, ...only] = process.argv.slice(2);
const shots = arg === '--gif' ? undefined : arg;
const browser = await chromium.launch(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {});
// The site's tours at 1280x720: the page drawn at two thirds.
const newPage = () => browser.newPage({ viewport: { width: 1920, height: 1080 }, deviceScaleFactor: shots ? 2 / 3 : 1 });

// Pipes every frame of `url` in `page` at `fps` into ffmpeg, which writes `out` with `encode` (its output
// options), the music under it unless `silent`.
async function render(page, url, fps, encode, out, silent = false) {
  await page.goto(url, { waitUntil: 'networkidle' });
  await page.evaluate(() => document.fonts.ready);
  const duration = await page.evaluate(() => window.READY.then(() => window.DURATION));
  // The music faded in and out over the tour's length (the video's frames end it).
  const audio = silent ? [] : ['-i', music, '-map', '0:v', '-map', '1:a',
    '-af', `afade=t=in:d=0.6,afade=t=out:st=${(duration - 2.5).toFixed(2)}:d=2.5`, '-c:a', 'aac', '-b:a', '128k', '-shortest'];
  const ffmpeg = spawn('ffmpeg', ['-y', '-v', 'error', '-f', 'image2pipe', '-framerate', String(fps), '-i', '-', ...audio, ...encode, out],
    { stdio: ['pipe', 'inherit', 'inherit'] });
  // An ffmpeg that stops early (a bad path, a missing encoder) ends the loop instead of hanging it.
  const closed = new Promise(r => ffmpeg.on('close', r));
  ffmpeg.stdin.on('error', () => {});
  const frames = Math.round(duration * fps);
  for (let i = 0; i < frames && ffmpeg.exitCode === null; i++) {
    await page.evaluate(t => window.render(t), i / fps);   // its clips seeked to that frame
    const jpeg = await page.screenshot({ type: 'jpeg', quality: 95 });
    if (!ffmpeg.stdin.write(jpeg)) await Promise.race([new Promise(r => ffmpeg.stdin.once('drain', r)), closed]);
  }
  ffmpeg.stdin.end();
  const code = await closed;
  if (code !== 0) throw new Error(`ffmpeg exited with ${code} writing ${out}`);
  console.log(`${out}: ${frames} frames, ${duration.toFixed(1)} s`);
}

const H264 = ['-c:v', 'libx264', '-tune', 'animation', '-crf', '26', '-pix_fmt', 'yuv420p', '-movflags', '+faststart'];
const json = (...path) => JSON.parse(readFileSync(join(root, ...path), 'utf8'));
const flat = (d, prefix = '') => Object.entries(d).reduce((o, [k, v]) =>
  Object.assign(o, v && typeof v === 'object' ? flat(v, `${prefix}${k}/`) : { [`${prefix}${k}`]: v }), {});
const english = json('site', 'i18n', 'en-US.json');
const appEnglish = flat(json('resources', 'i18n', 'en-US', 'playguard.json'));

// A language's texts as build_site.py writes them: its own, else English's; {{app:<key>}} the app's own words.
function texts(lang) {
  const own = lang === 'en-US' ? english : json('site', 'i18n', `${lang}.json`);
  const app = flat(json('resources', 'i18n', lang, 'playguard.json'));
  const word = (k) => (app[k] ?? appEnglish[k]).replace(/[.…]+$/, '').trim();
  const t = (key) => (own[key] ?? english[key]).replace(/\{\{app:([^}]+)\}\}/g, (_, k) => word(k));
  return { t, tour: Object.fromEntries(Object.keys(english).filter((k) => k.startsWith('tour_')).map((k) => [k, t(k)])) };
}

if (shots) {
  const LANGUAGES = ['fr', 'fr-CA', 'de', 'es', 'es-419', 'it', 'nl', 'pt', 'pt-BR', 'ru', 'ja', 'ko', 'zh-Hans', 'zh-Hant'];
  // libnx's SetLanguage numbers, which the gate's preview takes (source/core/gate_text.c).
  const SET_LANGUAGE = { ja: 0, fr: 2, de: 3, it: 4, es: 5, ko: 7, nl: 8, pt: 9, ru: 10, 'fr-CA': 13, 'es-419': 14, 'zh-Hans': 15, 'zh-Hant': 16, 'pt-BR': 17 };
  // The website's screenshots the tour uses (tour.html's shot), in their dark theme, PlayGuard's default.
  const SHOTS = ['dashboard', 'play_timer', 'per_day', 'activity', 'restrictions', 'security', 'clock', 'console'];
  const preview = join(root, 'build', 'gate-preview', 'gate_preview');
  const gates = existsSync(preview) || spawnSync('make', ['-C', root, 'gate-preview'], { stdio: 'ignore' }).status === 0;
  const data = (file, type) => `data:image/${type};base64,${readFileSync(file).toString('base64')}`;
  // A few at once (TOUR_JOBS, 2 by default): a page draws on one core, ffmpeg on another.
  const queue = (only.length ? only : LANGUAGES).filter((lang) => existsSync(join(shots, lang, 'dashboard-dark.webp')));
  await Promise.all(Array.from({ length: Math.max(1, Number(process.env.TOUR_JOBS) || 2) }, async () => {
    for (let lang; (lang = queue.shift());) {
      const { t, tour: strings } = texts(lang);
      const src = {};
      for (const s of SHOTS) {
        const file = join(shots, lang, `${s}-dark.webp`);
        if (existsSync(file)) src[s] = data(file, 'webp');
      }
      // The gate's screens in that language: its own preview, drawn here; not in Japanese, Korean or
      // Chinese, which the preview draws in PlayGuard's Latin font (the console's own font has them).
      if (gates && !['ja', 'ko', 'zh-Hans', 'zh-Hant'].includes(lang)) {
        const dir = mkdtempSync(join(tmpdir(), 'gate-'));
        if (spawnSync(preview, [dir, String(SET_LANGUAGE[lang]), 'dark'], { stdio: 'ignore' }).status === 0) {
          for (const f of readdirSync(dir)) src[`gate_${f.replace(/\.png$/, '')}`] = data(join(dir, f), 'png');
        }
      }
      const TOUR = {
        lang,
        t: strings,
        src,
        tag: t('hero_title'),
        // "No phone app, no Nintendo account, no internet." a line each.
        hooks: t('hero_sub').split(/[,、，]\s*/).map((s) => s.trim().replace(/[.。]$/, '')).map((s) => s.charAt(0).toUpperCase() + s.slice(1)),
        stores: 'Homebrew App Store · sphaira',
        cfw: t('cfw_title'),
      };
      if (TOUR.hooks.length !== 3) TOUR.hooks = ['', t('hero_sub'), ''];
      const page = await newPage();
      await page.addInitScript((tour) => { window.TOUR = tour; }, TOUR);
      await render(page, tour, 30, ['-preset', 'slow', ...H264], join(shots, lang, 'tour.mp4'));
      await page.close();
    }
  }));
  await browser.close();
  process.exit(0);
}

const page = await newPage();
await page.addInitScript((tour) => { window.TOUR = tour; }, { lang: 'en-US', t: texts('en-US').tour });
// --gif: the GIF alone, the MP4s left as they are.
if (arg !== '--gif') {
  await render(page, tour, 30, ['-preset', 'veryslow', ...H264], mp4);
  await render(page, `${tour}?short`, 30, ['-preset', 'veryslow', ...H264], teaser);
}
// One palette for the whole GIF and no dithering: flat colours stay flat.
await render(page, `${tour}?still`, 10, ['-vf',
  'scale=640:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=128:stats_mode=full[p];[b][p]paletteuse=dither=none:diff_mode=rectangle'], gif, true);
await browser.close();

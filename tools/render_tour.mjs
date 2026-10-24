// Renders branding/tour.html, frame by frame, to images/video/playguard-tour.mp4
// (1920x1080, 30 fps) and images/video/playguard-tour.gif (640 px, 10 fps: under
// the 10 MB GitHub shows inline): the
// README shows the GIF, which GitHub plays inline (its mobile app too), and
// links it to itself at full size: GitHub never plays an MP4 from the
// repository (its file view says it is too big, its raw link downloads it).
// The GIF is rendered with tour.html?still (no drifting glow, no zoom on the
// screenshots), so most of each frame repeats the last one and it stays small.
// Needs Node + Playwright (Chromium) and ffmpeg; the Sora font is the site's own (site/assets/fonts/).
//   node tools/render_tour.mjs [--gif]
// The website's tour in the console's other languages: their texts (the
// site's catalogs, site/i18n/) over their screenshots (tools/site_screenshots.mjs),
// <shots-dir>/<language>/tour.mp4 (1280x720) for tools/build_site.py
// (SITE_SCREENSHOTS=<shots-dir>); a language without its screenshots gets none
// (the site shows the English tour). Fonts-noto-cjk for Japanese, Korean and Chinese.
//   node tools/render_tour.mjs <shots-dir> [language...]
import { chromium } from 'playwright';
import { spawn } from 'node:child_process';
import { existsSync, readFileSync } from 'node:fs';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const mp4 = join(root, 'images/video/playguard-tour.mp4');
const gif = join(root, 'images/video/playguard-tour.gif');
const tour = pathToFileURL(join(root, 'branding/tour.html')).href;

const [arg, ...only] = process.argv.slice(2);
const shots = arg === '--gif' ? undefined : arg;
const browser = await chromium.launch(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {});
// The site's tours at 1280x720: the page drawn at two thirds.
const newPage = () => browser.newPage({ viewport: { width: 1920, height: 1080 }, deviceScaleFactor: shots ? 2 / 3 : 1 });

// Pipes every frame of `url` in `page` at `fps` into ffmpeg, which writes `out` with `encode` (its output options).
async function render(page, url, fps, encode, out) {
  await page.goto(url, { waitUntil: 'networkidle' });
  await page.evaluate(() => document.fonts.ready);
  const duration = await page.evaluate(() => window.DURATION);
  const ffmpeg = spawn('ffmpeg', ['-y', '-v', 'error', '-f', 'image2pipe', '-framerate', String(fps), '-i', '-', ...encode, out],
    { stdio: ['pipe', 'inherit', 'inherit'] });
  // An ffmpeg that stops early (a bad path, a missing encoder) ends the loop instead of hanging it.
  const closed = new Promise(r => ffmpeg.on('close', r));
  ffmpeg.stdin.on('error', () => {});
  const frames = Math.round(duration * fps);
  for (let i = 0; i < frames && ffmpeg.exitCode === null; i++) {
    await page.evaluate(t => window.render(t), i / fps);
    const jpeg = await page.screenshot({ type: 'jpeg', quality: 95 });
    if (!ffmpeg.stdin.write(jpeg)) await Promise.race([new Promise(r => ffmpeg.stdin.once('drain', r)), closed]);
  }
  ffmpeg.stdin.end();
  const code = await closed;
  if (code !== 0) throw new Error(`ffmpeg exited with ${code} writing ${out}`);
  console.log(`${out}: ${frames} frames, ${duration.toFixed(1)} s`);
}

const H264 = ['-c:v', 'libx264', '-tune', 'animation', '-crf', '26', '-pix_fmt', 'yuv420p', '-movflags', '+faststart'];

if (shots) {
  const LANGUAGES = ['fr', 'fr-CA', 'de', 'es', 'es-419', 'it', 'nl', 'pt', 'pt-BR', 'ru', 'ja', 'ko', 'zh-Hans', 'zh-Hant'];
  const json = (...path) => JSON.parse(readFileSync(join(root, ...path), 'utf8'));
  const flat = (d, prefix = '') => Object.entries(d).reduce((o, [k, v]) =>
    Object.assign(o, v && typeof v === 'object' ? flat(v, `${prefix}${k}/`) : { [`${prefix}${k}`]: v }), {});
  const english = json('site', 'i18n', 'en-US.json');
  const appEnglish = flat(json('resources', 'i18n', 'en-US', 'playguard.json'));
  // The scenes of tour.html, each a screen (its dark theme, PlayGuard's default), its tab's name
  // (k, the app's own word) and the site's texts for it; the bullets stay English-only.
  const SCENES = [
    { img: 'dashboard', k: 'app:tabs/dashboard', h: 'shot_dashboard', p: 'shot_dashboard_alt', ox: '58%', oy: '30%', hl: [528, 226, 714, 126] },
    { img: 'play_timer', k: 'app:tabs/play_timer', h: 'f_timer_title', p: 'shot_play_timer_alt', ox: '55%', oy: '40%', hl: [256, 180, 980, 344] },
    { img: 'activity', k: 'app:tabs/activity', h: 'f_activity_title', p: 'f_activity_text', ox: '55%', oy: '25%', hl: [256, 150, 980, 158] },
    { img: 'restrictions', k: 'app:tabs/restrictions', h: 'f_restrictions_title', p: 'f_restrictions_text', ox: '55%', oy: '30%', hl: [256, 118, 980, 266] },
    { img: 'clock', k: 'app:tabs/console', h: 'shot_clock', p: 'f_console_text', ox: '50%', oy: '28%', hl: [66, 122, 1148, 160] },
  ];
  // The "And there's more" cards: an icon of tour.html's, a title and a line from the site's catalog
  // (app: the app's own word).
  const BONUS = [
    { i: 'bell', t: 'f_notices_title', p: 'f_notices_text' },
    { i: 'eye', t: 'app:cadran/today/child' },
    { i: 'undo', t: 'f_history_title', p: 'f_history_text' },
    { i: 'unlink', t: 'f_app_title', p: 'f_app_text' },
    { i: 'globe', t: 'f_languages_title', p: 'f_languages_text' },
    { i: 'play', t: 'simulator_title', p: 'try' },
  ];
  // A few at once (TOUR_JOBS, 2 by default): a page draws on one core, ffmpeg on another.
  const queue = (only.length ? only : LANGUAGES).filter((lang) => SCENES.every((s) => existsSync(join(shots, lang, `${s.img}-dark.webp`))));
  await Promise.all(Array.from({ length: Math.max(1, Number(process.env.TOUR_JOBS) || 2) }, async () => {
    for (let lang; (lang = queue.shift());) {
      const own = json('site', 'i18n', `${lang}.json`);
      const app = flat(json('resources', 'i18n', lang, 'playguard.json'));
      // A catalog's text as build_site.py writes it: {{app:<key>}} the app's own words.
      const word = (k) => (app[k] ?? appEnglish[k]).replace(/[.…]+$/, '').trim();
      const t = (key) => key.startsWith('app:') ? word(key.slice(4)) : (own[key] ?? english[key]).replace(/\{\{app:([^}]+)\}\}/g, (_, k) => word(k));
      // A screenshot's description without the screen's name before it ("Today: the time left…").
      const alt = (key) => { const s = t(key).replace(/^[^:：]*[:：]\s*/, ''); return s.charAt(0).toUpperCase() + s.slice(1); };
      const data = (file) => `data:image/webp;base64,${readFileSync(file).toString('base64')}`;
      const TOUR = {
        lang,
        tag: t('hero_title'),
        // "No phone app, no Nintendo account, no internet." a line each.
        hooks: t('hero_sub').split(/[,、，]\s*/).map((s) => s.trim().replace(/[.。]$/, '')).map((s) => s.charAt(0).toUpperCase() + s.slice(1)),
        hooksub: t('faq_closed_a').replace(/^[^.!?。！？:：]*[.!?。！？]\s*/, ''),
        scenes: SCENES.map((s) => ({ ...s, k: t(s.k), h: t(s.h), p: s.p.endsWith('_alt') ? alt(s.p) : t(s.p), src: data(join(shots, lang, `${s.img}-dark.webp`)) })),
        more: t('features_more'),
        bonus: BONUS.map((c) => ({ i: c.i, t: t(c.t), p: c.p ? t(c.p) : '' })),
        line1: t('footer_license'),
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
// --gif: the GIF alone, the MP4 left as it is.
if (arg !== '--gif') await render(page, tour, 30, ['-preset', 'veryslow', ...H264], mp4);
// One palette for the whole GIF and no dithering: flat colours stay flat.
await render(page, `${tour}?still`, 10, ['-vf',
  'scale=640:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=128:stats_mode=full[p];[b][p]paletteuse=dither=none:diff_mode=rectangle'], gif);
await browser.close();

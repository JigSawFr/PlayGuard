// Renders branding/tour.html, frame by frame, to images/video/playguard-tour.mp4
// (1920x1080, 30 fps) and images/video/playguard-tour.gif (720 px, 10 fps): the
// README shows the GIF, which GitHub plays inline (its mobile app too), and
// links it to itself at full size: GitHub never plays an MP4 from the
// repository (its file view says it is too big, its raw link downloads it).
// The GIF is rendered with tour.html?still (no drifting glow, no zoom on the
// screenshots), so most of each frame repeats the last one and it stays small.
// Needs Node + Playwright (Chromium) and ffmpeg; loads the Sora font from Google Fonts.
//   node tools/render_tour.mjs
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

const [shots, ...only] = process.argv.slice(2);
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
  // The scenes of tour.html, each a screen and the site's texts for it.
  const SCENES = [
    { img: 'dashboard', h: 'shot_dashboard', p: 'f_notices_text', ox: '60%', oy: '35%' },
    { img: 'play_timer', h: 'f_timer_title', p: 'f_timer_text', ox: '65%', oy: '35%' },
    { img: 'activity', h: 'f_activity_title', p: 'f_activity_text', ox: '65%', oy: '40%' },
    { img: 'restrictions', h: 'f_restrictions_title', p: 'f_restrictions_text', ox: '65%', oy: '30%' },
    { img: 'clock', h: 'shot_clock', p: 'f_console_text', ox: '65%', oy: '55%' },
    { img: 'dashboard-dark', h: 'f_languages_title', p: 'f_languages_text', ox: '50%', oy: '50%' },
  ];
  // A few at once (TOUR_JOBS, 2 by default): a page draws on one core, ffmpeg on another.
  const queue = (only.length ? only : LANGUAGES).filter((lang) => SCENES.every((s) => existsSync(join(shots, lang, `${s.img}.webp`))));
  await Promise.all(Array.from({ length: Math.max(1, Number(process.env.TOUR_JOBS) || 2) }, async () => {
    for (let lang; (lang = queue.shift());) {
      const own = json('site', 'i18n', `${lang}.json`);
      const app = flat(json('resources', 'i18n', lang, 'playguard.json'));
      // A catalog's text as build_site.py writes it: {{app:<key>}} the app's own words.
      const t = (key) => (own[key] ?? english[key]).replace(/\{\{app:([^}]+)\}\}/g, (_, k) => (app[k] ?? appEnglish[k]).replace(/[.…]+$/, '').trim());
      const data = (file) => `data:image/webp;base64,${readFileSync(file).toString('base64')}`;
      const TOUR = {
        lang,
        tag: t('hero_title'),
        // "No phone app, no Nintendo account, no internet." a line each.
        hooks: t('hero_sub').split(/[,、，]\s*/).map((s) => s.trim().replace(/[.。]$/, '')).map((s) => s.charAt(0).toUpperCase() + s.slice(1)),
        hooksub: t('faq_closed_a').replace(/^[^.!?。！？:：]*[.!?。！？]\s*/, ''),
        scenes: SCENES.map((s) => ({ ...s, h: t(s.h), p: t(s.p), src: data(join(shots, lang, `${s.img}.webp`)) })),
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
await render(page, tour, 30, ['-preset', 'veryslow', ...H264], mp4);
// One palette for the whole GIF and no dithering: flat colours stay flat.
await render(page, `${tour}?still`, 10, ['-vf',
  'scale=720:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=128:stats_mode=full[p];[b][p]paletteuse=dither=none:diff_mode=rectangle'], gif);
await browser.close();

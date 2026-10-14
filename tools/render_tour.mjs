// Renders branding/tour.html, frame by frame, to images/video/playguard-tour.mp4
// (1920x1080, 30 fps, the README's video tour). Its poster,
// images/video/playguard-tour.png, is a frame of it with a play button drawn on.
// Needs Node + Playwright (Chromium) and ffmpeg; loads the Sora font from Google Fonts.
//   node tools/render_tour.mjs
import { chromium } from 'playwright';
import { spawn } from 'node:child_process';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const out = join(root, 'images/video/playguard-tour.mp4');
const FPS = 30;

const browser = await chromium.launch(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {});
const page = await browser.newPage({ viewport: { width: 1920, height: 1080 }, deviceScaleFactor: 1 });
await page.goto(pathToFileURL(join(root, 'branding/tour.html')).href, { waitUntil: 'networkidle' });
await page.evaluate(() => document.fonts.ready);
const duration = await page.evaluate(() => window.DURATION);

const ffmpeg = spawn('ffmpeg', ['-y', '-v', 'error', '-f', 'image2pipe', '-framerate', String(FPS), '-i', '-',
  '-c:v', 'libx264', '-preset', 'slow', '-crf', '18', '-pix_fmt', 'yuv420p', '-movflags', '+faststart', out],
  { stdio: ['pipe', 'inherit', 'inherit'] });
// An ffmpeg that stops early (a bad path, a missing encoder) ends the loop instead of hanging it.
const closed = new Promise(r => ffmpeg.on('close', r));
ffmpeg.stdin.on('error', () => {});
const frames = Math.round(duration * FPS);
for (let i = 0; i < frames && ffmpeg.exitCode === null; i++) {
  await page.evaluate(t => window.render(t), i / FPS);
  const jpeg = await page.screenshot({ type: 'jpeg', quality: 95 });
  if (!ffmpeg.stdin.write(jpeg)) await Promise.race([new Promise(r => ffmpeg.stdin.once('drain', r)), closed]);
}
ffmpeg.stdin.end();
const code = await closed;
await browser.close();
if (code !== 0) throw new Error(`ffmpeg exited with ${code}`);
console.log(`${out}: ${frames} frames, ${duration.toFixed(1)} s`);

// Renders branding/tour.html, frame by frame, to images/video/playguard-tour.mp4
// (1920x1080, 30 fps) and images/video/playguard-tour.gif (720 px, 10 fps): the
// README shows the GIF, which GitHub plays inline (its mobile app too), and
// links it to itself at full size: GitHub never plays an MP4 from the
// repository (its file view says it is too big, its raw link downloads it).
// The GIF is rendered with tour.html?still (no drifting glow, no zoom on the
// screenshots), so most of each frame repeats the last one and it stays small.
// Needs Node + Playwright (Chromium) and ffmpeg; loads the Sora font from Google Fonts.
//   node tools/render_tour.mjs
import { chromium } from 'playwright';
import { spawn } from 'node:child_process';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';

const root = join(dirname(fileURLToPath(import.meta.url)), '..');
const mp4 = join(root, 'images/video/playguard-tour.mp4');
const gif = join(root, 'images/video/playguard-tour.gif');
const tour = pathToFileURL(join(root, 'branding/tour.html')).href;

const browser = await chromium.launch(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {});
const page = await browser.newPage({ viewport: { width: 1920, height: 1080 }, deviceScaleFactor: 1 });

// Pipes every frame of `url` at `fps` into ffmpeg, which writes `out` with `encode` (its output options).
async function render(url, fps, encode, out) {
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

await render(tour, 30, ['-c:v', 'libx264', '-preset', 'veryslow', '-tune', 'animation', '-crf', '26',
  '-pix_fmt', 'yuv420p', '-movflags', '+faststart'], mp4);
// One palette for the whole GIF and no dithering: flat colours stay flat.
await render(`${tour}?still`, 10, ['-vf',
  'scale=720:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=128:stats_mode=full[p];[b][p]paletteuse=dither=none:diff_mode=rectangle'], gif);
await browser.close();

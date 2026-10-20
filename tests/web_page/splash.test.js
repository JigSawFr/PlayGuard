// The simulator's splash screens (what an iPhone shows while it starts from
// the home screen): web/index.html links one per iPhone and orientation, with
// the media query iOS matches it by, and each is in images/splash/
// (tools/render_branding.mjs). A wrong ratio or size, and iOS shows white.
// `node --test tests/web_page/*.test.js` (CI's host tests).
'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const path = require('node:path');
const fs = require('node:fs');

const root = path.join(__dirname, '../..');
const page = fs.readFileSync(path.join(root, 'web/index.html'), 'utf8');
const links = [...page.matchAll(/<link rel="apple-touch-startup-image" href="splash\/([^"]+)" media="([^"]+)">/g)]
  .map(([, file, media]) => ({ file, media }));
const files = fs.readdirSync(path.join(root, 'images/splash')).filter((f) => f.endsWith('.png'));
const devices = [...fs.readFileSync(path.join(root, 'tools/render_branding.mjs'), 'utf8')
  .split('export const SPLASH = [')[1].split('];')[0].matchAll(/\[(\d+), (\d+), (\d+)\]/g)]
  .map((m) => m.slice(1).map(Number));

test('each iPhone of the list, in both orientations, is linked and rendered', () => {
  assert.ok(devices.length > 0);
  assert.equal(links.length, devices.length * 2);
  assert.deepEqual([...links.map((l) => l.file)].sort(), [...files].sort());
});

test('each link\'s media query is its image\'s screen', () => {
  for (const { file, media } of links) {
    const [, w, h, ratio, orientation] = file.match(/^iphone-(\d+)x(\d+)@(\d)-(portrait|landscape)\.png$/);
    assert.equal(media, `(device-width: ${w}px) and (device-height: ${h}px) and (-webkit-device-pixel-ratio: ${ratio}) and (orientation: ${orientation})`);
    // The image at the screen's pixels, turned with it.
    const png = fs.readFileSync(path.join(root, 'images/splash', file));
    const [width, height] = [png.readUInt32BE(16), png.readUInt32BE(20)];
    const [x, y] = orientation === 'portrait' ? [w * ratio, h * ratio] : [h * ratio, w * ratio];
    assert.deepEqual([width, height], [x, y], file);
  }
});

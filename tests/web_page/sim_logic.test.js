// Tests for web/sim_logic.js, the web simulator's page logic that touches
// neither the page nor PlayGuard: `node --test tests/web_page/*.test.js`
// (CI's host tests). The page itself runs in CI's headless Chromium
// (tools/web_smoke.py).
'use strict';

const test = require('node:test');
const assert = require('node:assert/strict');
const path = require('node:path');
const fs = require('node:fs');

const L = require('../../web/sim_logic.js');

const manifest = JSON.parse(fs.readFileSync(path.join(__dirname, '../../web/params.json'), 'utf8'));
const known = new Map(manifest.params.map((p) => [p.name, p]));

test('every language has its flag, as on the website', () => {
  assert.deepEqual(Object.keys(L.FLAGS).sort(), L.LANGUAGES.map(([code]) => code).sort());
  assert.equal(L.flag('fr'), '\u{1F1EB}\u{1F1F7}');
  assert.equal(L.flag('es-419'), '\u{1F1F2}\u{1F1FD}');
  assert.equal(L.flag('zh-Hant'), '\u{1F1F9}\u{1F1FC}');
  assert.equal(L.flag('xx'), '');
  // The website's (tools/build_site.py's FLAGS), the same.
  const site = fs.readFileSync(path.join(__dirname, '../../tools/build_site.py'), 'utf8');
  const pairs = /^FLAGS = \{([^}]*)\}/m.exec(site)[1].matchAll(/"([^"]+)": "([A-Z]{2})"/g);
  assert.deepEqual(Object.fromEntries([...pairs].map(([, c, f]) => [c, f])), L.FLAGS);
});

test('a browser language to one of the console\'s', () => {
  assert.equal(L.consoleLanguage('fr-CA'), 'fr-CA');
  assert.equal(L.consoleLanguage('fr-BE'), 'fr');
  assert.equal(L.consoleLanguage('en-GB'), 'en-US');
  assert.equal(L.consoleLanguage('es'), 'es');
  assert.equal(L.consoleLanguage('es-ES'), 'es');
  assert.equal(L.consoleLanguage('es-MX'), 'es-419');
  assert.equal(L.consoleLanguage('pt-BR'), 'pt-BR');
  assert.equal(L.consoleLanguage('pt-PT'), 'pt');
  assert.equal(L.consoleLanguage('zh-TW'), 'zh-Hant');
  assert.equal(L.consoleLanguage('zh-Hant-HK'), 'zh-Hant');
  assert.equal(L.consoleLanguage('zh-CN'), 'zh-Hans');
  assert.equal(L.consoleLanguage('ZH-HANS'), 'zh-Hans');
  assert.equal(L.consoleLanguage('ja-JP'), 'ja');
  assert.equal(L.consoleLanguage('pl'), null);
  assert.equal(L.consoleLanguage(undefined), null);
  assert.equal(L.firstConsoleLanguage(['pl', 'cs', 'de-AT', 'fr']), 'de');
  assert.equal(L.firstConsoleLanguage(['pl']), 'en-US');
  // Every language of the console is one the page offers.
  assert.deepEqual(L.LANGUAGES.map(([code]) => code).sort(), [...known.get('lang').options].sort());
});

test('a flag is off as core/sim_env.h says', () => {
  for (const off of [undefined, null, '', '0', 'false', 'OFF', 'No']) assert.equal(L.isOn(off), false, String(off));
  for (const on of ['1', 'yes', 'true', 'on', 'anything']) assert.equal(L.isOn(on), true, on);
});

test('the URL: only what is set, in the panel\'s order, flags as 1', () => {
  const values = new Map([['zzz', 'x'], ['fw', '20.5.0'], ['unlocked', 'yes'], ['restricted', '0'], ['skip', '']]);
  assert.equal(L.toQuery(values, known), '?unlocked=1&fw=20.5.0&zzz=x');
  assert.equal(L.toQuery(new Map(), known), '');
  // Read back: names in lower case, the others left out.
  const read = L.readQuery('?UNLOCKED=1&fw=20.5.0&bad-name=1&x%20y=2');
  assert.deepEqual([...read], [['unlocked', '1'], ['fw', '20.5.0']]);
  // What changed between two sets of settings, as their URLs say.
  const a = new Map([['unlocked', '1'], ['skip', '300']]);
  const b = new Map([['unlocked', 'true'], ['skip', '600'], ['idle', '1']]);
  assert.deepEqual(L.changedNames(a, b, known).sort(), ['idle', 'skip']);
});

test('the variables and config.json PlayGuard starts with', () => {
  const values = new Map([
    ['lang', 'de'], ['theme', 'dark'], ['tab', 'console'], ['rec', 'down'], ['unlocked', 'yes'],
    ['restricted', '0'], ['fw', '20.5.0'], ['sys_notes', 'sample'], ['unknown_one', 'v'],
  ]);
  const { env, config } = L.startEnvironment(values, known, 'fr', 'light');
  // The page's language and theme, not the URL's: the page already chose.
  assert.deepEqual(config, { language: 'fr', theme: 'light', start_tab: 'console' });
  assert.equal(env.PLAYGUARD_SIM_UNLOCKED, '1');
  assert.equal(env.PLAYGUARD_SIM_RESTRICTED, undefined);
  assert.equal(env.PLAYGUARD_SIM_FW, '20.5.0');
  assert.equal(env.PLAYGUARD_SIM_SYS_NOTES, known.get('sys_notes').values.sample);
  assert.equal(env.PLAYGUARD_SIM_UNKNOWN_ONE, 'v');   // a newer build's variable still reaches it
  assert.equal(env.PLAYGUARD_SIM_REC, undefined);      // the page's own
  assert.equal(L.envValue(known.get('unlocked'), 'off'), '');
  assert.equal(L.envName('dev_builds'), 'PLAYGUARD_SIM_DEV_BUILDS');
});

test('the time moved on, as the page shows it', () => {
  assert.equal(L.skipText(0), '');
  assert.equal(L.skipText('junk'), '');
  assert.equal(L.skipText(300), '+5 min');
  assert.equal(L.skipText(3600), '+1 h');
  assert.equal(L.skipText(3900), '+1 h 5 min');
});

test('the console\'s line under the screen', () => {
  assert.equal(L.durationText(0), '0 s');
  assert.equal(L.durationText(59.9), '59 s');
  assert.equal(L.durationText(2700), '45 min');
  assert.equal(L.durationText(7200), '2 h');
  assert.equal(L.durationText(4830), '1 h 20 min');
  const base = { game: 'Star Kart Racers', spent: 4800, limit: 120, left: 2420, restricted: false, alarm_only: false, unlocked: false };
  const line = (c) => L.consoleLine({ ...base, ...c });
  const keys = (c) => line(c).parts.map(([k]) => k);
  assert.deepEqual(line({}), {
    parts: [['console_game', { game: 'Star Kart Racers' }], ['console_spent', { t: '1 h 20 min' }], ['console_left', { t: '40 min' }]],
    tone: 'ok',
  });
  assert.deepEqual(keys({ game: '' }), ['console_no_game', 'console_spent', 'console_left']);
  // Past the limit: suspended, or a warning only and the time over it.
  assert.equal(line({ restricted: true, left: 0 }).tone, 'stop');
  assert.deepEqual(line({ alarm_only: true, left: -600 }).parts[2], ['console_warning', { t: '10 min' }]);
  assert.equal(line({ alarm_only: true, left: -600 }).tone, 'warn');
  // Unlocked first, whatever the timer says.
  assert.deepEqual(line({ unlocked: true, restricted: true, limit: -1 }).parts[2], ['console_unlocked', {}]);
  assert.equal(line({ unlocked: true }).tone, 'free');
  assert.deepEqual(keys({ limit: -1, left: 0 }), ['console_game', 'console_spent', 'console_no_limit']);
  // No game running: the timer has no time left to say.
  assert.deepEqual(keys({ game: '', left: 0 }), ['console_no_game', 'console_spent']);
});

test('steps: read before any is played', () => {
  assert.deepEqual(L.parseSteps('down*3.w15.t640,360.s:skip=3600.say:end.a').map((s) => {
    const { step, ...rest } = s;
    return rest;
  }), [
    { button: 'down', times: 3 }, { wait: 1500 }, { touch: [640, 360] }, { set: ['skip', '3600'] }, { say: 'end' },
    { button: 'a', times: 1 },
  ]);
  assert.deepEqual(L.parseSteps(''), []);
  assert.deepEqual(L.parseSteps('..a..'), [{ button: 'a', times: 1, step: 'a' }]);
  for (const bad of ['jump', 'w', 't1', 's:Bad=1', 'say:', 's:fw=%E0%A4%A']) assert.throws(() => L.parseSteps(`a.${bad}`), /no such step/, bad);
  // A live value with dots and spaces goes through a step and back.
  const step = `s:${L.stepValue('fw', '20.5.0 x')}`;
  assert.ok(!step.slice(2).includes('.'));
  assert.deepEqual(L.parseSteps(step)[0].set, ['fw', '20.5.0 x']);
  assert.deepEqual(L.parseSteps(`s:${L.stepValue('skip', '')}`)[0].set, ['skip', '']);
  // Noted steps written back: a button pressed in a row, once with *N.
  assert.equal(L.stepsText(['down', 'down', 'down', 'w5', 'a', 'a', 's:skip=60', 'say:end']), 'down*3.w5.a*2.s:skip=60.say:end');
  // Every button a step names is one the page presses.
  assert.equal(L.STEP_BUTTONS.length, 14);
});

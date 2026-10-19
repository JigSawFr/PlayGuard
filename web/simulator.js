// The web simulator's page (web/README.md): reads the settings from the URL,
// shows them in the side panel (built from params.json), and starts PlayGuard
// (playguard.js, Emscripten) with them: PLAYGUARD_SIM_* variables, and a
// config.json for the language, the theme and the first tab. Applying new
// settings reloads the page with them in the URL, so a link always opens the
// same state.
//
// The page speaks the console's languages (i18n/<language>.json): the
// browser's, unless the URL says `lang=`; its theme is the browser's (Auto),
// or the one chosen in its header or set by `theme=`. PlayGuard takes both.
//
// While PlayGuard runs, the `live` settings change without a restart
// (source/sim/web_page.cpp), the buttons under the screen move the console's
// time on (`skip`), and with `keep` its data folder stays in the browser
// (IndexedDB). <body data-playguard> says where it is: loading, running,
// ready (its first screen is up), error.
//
// Steps: what is pressed, touched and changed live is noted, with the time
// between; **Steps link** gives the settings PlayGuard started with and
// them (`rec=`), which a page opening it replays. The tour (`tour=1`) and
// the smoke test (tools/web_smoke.py, runSteps()) are steps too.
'use strict';

// The console's languages, as PlayGuard's config.json names them, in their own words.
const LANGUAGES = [
  ['en-US', 'English'], ['fr', 'Français'], ['fr-CA', 'Français (Canada)'], ['de', 'Deutsch'],
  ['es', 'Español'], ['es-419', 'Español (Latinoamérica)'], ['it', 'Italiano'], ['nl', 'Nederlands'],
  ['pt', 'Português'], ['pt-BR', 'Português (Brasil)'], ['ru', 'Русский'], ['ja', '日本語'],
  ['ko', '한국어'], ['zh-Hans', '简体中文'], ['zh-Hant', '繁體中文'],
];
const THEMES = ['auto', 'light', 'dark'];
// The languages PlayGuard draws with the CJK font (cjk.otf, next to the page):
// waited for before it starts; fetched once it runs in the others.
const CJK = new Set(['ja', 'ko', 'zh-Hans', 'zh-Hant']);
const CJK_FILE = '/app/resources/font/cjk.otf';
// The data folder (source/util/paths.cpp), kept in IndexedDB under that name with `keep`.
const DATA = '/playguard_data';
// What the time buttons add to `skip`, in minutes.
const SKIPS = [5, 15, 60];
// Where a report goes without build.json (a local build).
const REPO = 'JigSawFr/PlayGuard';

// A language tag (the URL's, or the browser's) to one of the console's; null when none fits.
function consoleLanguage(tag) {
  const t = String(tag || '').toLowerCase();
  const exact = LANGUAGES.find(([code]) => code.toLowerCase() === t);
  if (exact) return exact[0];
  const [base, region = ''] = t.split('-');
  switch (base) {
    case 'en': return 'en-US';
    case 'fr': return region === 'ca' ? 'fr-CA' : 'fr';
    case 'es': return !region || region === 'es' ? 'es' : 'es-419';
    case 'pt': return region === 'br' ? 'pt-BR' : 'pt';
    case 'zh': return ['tw', 'hk', 'mo', 'hant'].includes(region) || t.includes('hant') ? 'zh-Hant' : 'zh-Hans';
    case 'de': case 'it': case 'nl': case 'ru': case 'ja': case 'ko': return base;
    default: return null;
  }
}
function browserLanguage() {
  for (const tag of navigator.languages?.length ? navigator.languages : [navigator.language]) {
    const code = consoleLanguage(tag);
    if (code) return code;
  }
  return 'en-US';
}

// borealis' ControllerButton values (patches/borealis/0090-sdl-in-a-browser.patch).
const BUTTON = { ZL: 0, L: 1, UP: 3, RIGHT: 4, DOWN: 5, LEFT: 6, MINUS: 7, PLUS: 9, Y: 11, B: 12, A: 13, X: 14, R: 15, ZR: 16 };
// The keys the page presses buttons for, by what they say (any layout); the
// arrows, Enter and Escape reach SDL directly.
const KEYS = {
  x: BUTTON.X, X: BUTTON.X, y: BUTTON.Y, Y: BUTTON.Y,
  l: BUTTON.L, r: BUTTON.R, L: BUTTON.ZL, R: BUTTON.ZR,
  '+': BUTTON.PLUS, p: BUTTON.PLUS, '-': BUTTON.MINUS, m: BUTTON.MINUS,
  Backspace: BUTTON.B,
};
const NAME = /^[a-z0-9_]+$/;
const FALSY = new Set(['', '0', 'false', 'off', 'no']);

const $ = (id) => document.getElementById(id);
const canvas = $('canvas');
let manifest = { groups: [], params: [], presets: [] };
let known = new Map();          // name -> param
let current = new Map();        // what the URL says (applied)
let started = false;
let text = { ui: {}, groups: {}, params: {}, presets: {} };   // the page's language, English underneath
let language = 'en-US';         // the page's, and PlayGuard's
let consoleTheme = 'light';     // what PlayGuard was started with
let running = false;            // PlayGuard's runtime is up: ccall works
let version = '';               // the build's commit, for the files' URLs
let buildInfo = null;           // build.json, for a report
let startValues = new Map();    // the settings PlayGuard started with, for a steps link

// ---- Texts ------------------------------------------------------------

const t = (key) => text.ui[key] ?? key;
const paramText = (p, key) => text.params[p.name]?.[key] ?? '';
const optionText = (p, value) => text.params[p.name]?.options?.[value] ?? value;

async function loadTexts(code) {
  const get = async (c) => {
    try {
      const res = await fetch(`i18n/${c}.json`);
      return res.ok ? await res.json() : {};
    } catch { return {}; }
  };
  const [en, own] = await Promise.all([get('en-US'), code === 'en-US' ? {} : get(code)]);
  const merged = { ui: {}, groups: {}, params: {}, presets: {} };
  for (const part of Object.keys(merged)) {
    Object.assign(merged[part], en[part]);
    for (const [k, v] of Object.entries(own[part] ?? {})) {
      merged[part][k] = part === 'params' ? { ...merged.params[k], ...v, options: { ...merged.params[k]?.options, ...v.options } } : v;
    }
  }
  text = merged;
}

// ---- The theme ------------------------------------------------------------

const dark = matchMedia('(prefers-color-scheme: dark)');

function storedTheme() {
  try { return localStorage.getItem('pg-theme'); } catch { return null; }
}
// What the header chose: the URL's theme, else the one kept from an earlier
// visit, else Auto.
function chosenTheme() {
  const fromUrl = current.get('theme');
  if (fromUrl === 'light' || fromUrl === 'dark') return fromUrl;
  const kept = storedTheme();
  return THEMES.includes(kept) ? kept : 'auto';
}
const effectiveTheme = (choice) => (choice === 'auto' ? (dark.matches ? 'dark' : 'light') : choice);

function showTheme() {
  const effective = effectiveTheme(chosenTheme());
  document.documentElement.dataset.theme = effective;
  // Started with the other one (Auto following a change of the system's):
  // PlayGuard reads its theme at start-up, so offer to start it again.
  $('theme-notice').hidden = !started || effective === consoleTheme;
}

function chooseTheme(choice) {
  try { localStorage.setItem('pg-theme', choice); } catch { /* storage refused */ }
  const values = new Map(current);
  if (choice === 'auto') values.delete('theme');
  else values.set('theme', choice);
  if (effectiveTheme(choice) === consoleTheme) {
    // PlayGuard already has it: only the URL changes.
    current = values;
    history.replaceState(null, '', `${location.pathname}${toQuery(values)}`);
    showTheme();
    markChanges();
    return;
  }
  apply(values);
}

// ---- The URL ------------------------------------------------------------

function readUrl() {
  const out = new Map();
  for (const [k, v] of new URLSearchParams(location.search)) {
    const name = k.toLowerCase();
    if (NAME.test(name)) out.set(name, v);
  }
  return out;
}

// Only what differs from the defaults goes in the URL, in the panel's order.
function toQuery(values) {
  const q = new URLSearchParams();
  const names = [...known.keys(), ...[...values.keys()].filter((n) => !known.has(n)).sort()];
  for (const name of names) {
    if (!values.has(name)) continue;
    const p = known.get(name);
    let v = values.get(name);
    if (p?.type === 'bool') {
      if (FALSY.has(String(v).toLowerCase())) continue;
      v = '1';
    }
    if (v === '' || v == null) continue;
    q.set(name, v);
  }
  const s = q.toString();
  return s ? `?${s}` : '';
}

function linkFor(values) {
  return `${location.origin}${location.pathname}${toQuery(values)}`;
}

// ---- The panel ------------------------------------------------------------

function el(tag, attrs = {}, ...children) {
  const node = document.createElement(tag);
  for (const [k, v] of Object.entries(attrs)) {
    if (k === 'class') node.className = v;
    else if (k === 'text') node.textContent = v;
    else if (v !== false && v != null) node.setAttribute(k, v === true ? '' : v);
  }
  for (const c of children) if (c != null) node.append(c);
  return node;
}

function toLocalInput(seconds) {
  const d = new Date(Number(seconds) * 1000);
  if (Number.isNaN(d.getTime())) return '';
  const pad = (n) => String(n).padStart(2, '0');
  return `${d.getFullYear()}-${pad(d.getMonth() + 1)}-${pad(d.getDate())}T${pad(d.getHours())}:${pad(d.getMinutes())}`;
}

function fieldFor(p, value) {
  const id = `p-${p.name}`;
  const label = paramText(p, 'label') || p.name;
  const help = paramText(p, 'help') ? el('p', { class: 'help', text: paramText(p, 'help') }) : null;
  if (p.type === 'bool') {
    const input = el('input', { type: 'checkbox', id, 'data-name': p.name });
    input.checked = value != null && !FALSY.has(String(value).toLowerCase());
    return [el('label', { class: 'field check', for: id }, input, el('span', { text: label })), help];
  }
  if (p.type === 'multi') {
    const chosen = new Set(String(value ?? '').split(',').filter(Boolean));
    const box = el('div', { class: 'multi', 'data-multi': p.name });
    for (const o of p.options) {
      const input = el('input', { type: 'checkbox', value: o, 'data-of': p.name });
      input.checked = chosen.has(o);
      box.append(el('label', { class: 'field check' }, input, el('span', { text: optionText(p, o) })));
    }
    return [el('div', { class: 'field' }, el('span', { text: label })), box, help];
  }
  let input;
  if (p.type === 'select') {
    input = el('select', { id, 'data-name': p.name });
    for (const o of p.options) input.append(el('option', { value: o, text: optionText(p, o) }));
    if (value != null && !p.options.includes(value)) input.append(el('option', { value, text: value }));
    input.value = value ?? '';
  } else if (p.type === 'datetime') {
    input = el('input', { type: 'datetime-local', id, 'data-name': p.name, 'data-datetime': true });
    input.value = value ? toLocalInput(value) : '';
  } else {
    input = el('input', {
      type: p.type === 'number' ? 'number' : 'text', id, 'data-name': p.name,
      placeholder: p.placeholder ?? '', min: p.min, max: p.max, inputmode: p.type === 'number' ? 'numeric' : null,
      list: p.suggestions ? `${id}-list` : null,
    });
    input.value = value ?? '';
  }
  const list = p.suggestions ? el('datalist', { id: `${id}-list` }, ...p.suggestions.map((s) => el('option', { value: s }))) : null;
  return [el('label', { class: 'field', for: id }, el('span', { text: label }), input), list, help];
}

function buildForm(values) {
  const form = $('form');
  form.replaceChildren();
  for (const g of manifest.groups) {
    const params = manifest.params.filter((p) => p.group === g.id && p.panel !== false && !p.page);
    if (!params.length) continue;
    const set = params.filter((p) => values.has(p.name)).length;
    const details = el('details', { class: 'group', 'data-group': g.id },
      el('summary', {}, text.groups[g.id] ?? g.id, set ? el('span', { class: 'count', text: String(set) }) : null));
    if (g.id === 'app' || g.id === 'parental' || set) details.open = true;
    for (const p of params) for (const node of fieldFor(p, values.get(p.name))) if (node) details.append(node);
    form.append(details);
  }
  buildOther(values);
  markChanges();
}

function buildOther(values) {
  const box = $('other');
  box.replaceChildren();
  const others = [...values.entries()].filter(([n]) => !known.has(n));
  for (const [name, value] of others) {
    const input = el('input', { 'data-other': name, 'aria-label': name });
    input.value = value;
    const remove = el('button', { type: 'button', class: 'btn ghost', text: '×', title: t('remove'), 'aria-label': t('remove') });
    remove.addEventListener('click', () => { input.closest('.other-row').remove(); markChanges(); });
    box.append(el('div', { class: 'other-row' }, el('code', { text: name }), input, remove));
  }
  $('other-group').open = others.length > 0;
}

// What the panel says now, with the header's language and theme as the URL has them.
function readForm() {
  const out = new Map();
  for (const name of ['lang', 'theme']) if (current.has(name)) out.set(name, current.get(name));
  for (const input of $('form').querySelectorAll('[data-name]')) {
    const name = input.dataset.name;
    if (input.type === 'checkbox') { if (input.checked) out.set(name, '1'); continue; }
    let v = input.value.trim();
    if (input.dataset.datetime != null && v) {
      const ms = new Date(v).getTime();
      v = Number.isNaN(ms) ? '' : String(Math.round(ms / 1000));
    }
    if (v !== '') out.set(name, v);
  }
  for (const box of $('form').querySelectorAll('[data-multi]')) {
    const chosen = [...box.querySelectorAll('input:checked')].map((i) => i.value);
    if (chosen.length) out.set(box.dataset.multi, chosen.join(','));
  }
  for (const input of $('other').querySelectorAll('[data-other]')) out.set(input.dataset.other, input.value.trim());
  return out;
}

function markChanges() {
  const pending = toQuery(readForm()) !== toQuery(current);
  $('apply').classList.toggle('pending', pending);
  for (const input of $('form').querySelectorAll('[data-name]')) {
    const name = input.dataset.name;
    const field = input.closest('.field');
    if (field) field.classList.toggle('changed', current.has(name) || (input.type === 'checkbox' ? input.checked : input.value !== ''));
  }
}

// The settings whose value differs between `a` and `b`.
function changedNames(a, b) {
  const qa = new URLSearchParams(toQuery(a)), qb = new URLSearchParams(toQuery(b));
  return [...new Set([...qa.keys(), ...qb.keys()])].filter((n) => qa.get(n) !== qb.get(n));
}

// Only `live` settings changed, PlayGuard running: they reach it at once and
// the URL follows; otherwise the page reloads with them.
function apply(values) {
  const changed = changedNames(values, current).filter((n) => !known.get(n)?.page);
  if (running && changed.length && changed.every((n) => known.get(n)?.live)) {
    for (const name of changed) { setLive(name, values.get(name)); noteStep(`s:${stepValue(name, values.get(name))}`); }
    current = new Map(values);
    history.replaceState(null, '', linkFor(current));
    buildForm(current);
    showSkip();
    toast(t('live_applied'));
    return;
  }
  const query = toQuery(values);
  if (query === location.search || (query === '' && location.search === '')) { location.reload(); return; }
  location.search = query;   // reloads: PlayGuard starts again from scratch
}

// The page's language and theme stay, and the `sticky` settings; the rest is what `keep` says.
function keepingHeader(keep) {
  return new Map([...current].filter(([n]) => n === 'lang' || n === 'theme' || known.get(n)?.sticky || keep(n)));
}

// A preset's datetime `@21:30`: today at that time, in the browser's time zone.
function presetValue(name, value) {
  const m = known.get(name)?.type === 'datetime' && /^@(\d\d):(\d\d)$/.exec(value);
  if (!m) return value;
  const d = new Date();
  d.setHours(Number(m[1]), Number(m[2]), 0, 0);
  return String(Math.round(d.getTime() / 1000));
}

function buildPresets() {
  const select = $('preset');
  select.append(el('option', { value: '', text: t('preset_none') }));
  for (const p of manifest.presets) select.append(el('option', { value: p.id, text: text.presets[p.id] ?? p.id }));
  select.addEventListener('change', () => {
    const preset = manifest.presets.find((p) => p.id === select.value);
    if (!preset) return;
    // The first tab stays too.
    const values = keepingHeader((n) => known.get(n)?.config);
    for (const [n, v] of Object.entries(preset.params)) values.set(n, presetValue(n, v));
    apply(values);
  });
}

// ---- PlayGuard ------------------------------------------------------------

let logLines = [];
function log(line0, error) {
  // eslint-disable-next-line no-control-regex
  const line = String(line0).replace(/\x1b\[[0-9;]*m/g, '');
  logLines.push([line, error]);
  if (logLines.length > 1000) logLines = logLines.slice(-800);
  const pre = $('log');
  if (pre.closest('[hidden]')) return;
  appendLog(pre, line, error);
}
function appendLog(pre, line, error) {
  const span = document.createElement('span');
  if (error || /\[ERROR\]/.test(line)) span.className = 'err';
  span.textContent = `${line}\n`;
  pre.append(span);
  pre.parentElement.scrollTop = pre.parentElement.scrollHeight;
}
function renderLog() {
  const pre = $('log');
  pre.replaceChildren();
  for (const [line, error] of logLines) appendLog(pre, line, error);
}

function showStatus(message) {
  $('status').textContent = message;
}

function showProgress(done, total) {
  const bar = $('progress');
  if (!bar || !total) return;
  bar.hidden = false;
  bar.max = total;
  bar.value = done;
}

let toastTimer = 0;
function toast(message) {
  const box = $('toast');
  box.textContent = message;
  box.hidden = false;
  clearTimeout(toastTimer);
  toastTimer = setTimeout(() => { box.hidden = true; }, 2500);
}

function showError(message) {
  running = false;   // nothing to call into any more: Apply reloads
  document.body.dataset.playguard = 'error';
  const overlay = $('overlay');
  overlay.hidden = false;
  overlay.classList.add('error');
  const button = el('button', { type: 'button', class: 'btn primary', text: t('reload') });
  button.addEventListener('click', () => location.reload());
  overlay.replaceChildren(el('div', { class: 'spinner', 'aria-hidden': 'true' }), el('p', { text: message }), button);
}

// A setting as its PLAYGUARD_SIM_* variable has it: '' for none.
function envValue(name, value) {
  const p = known.get(name);
  if (value == null || value === '') return '';
  if (p?.type === 'bool') return FALSY.has(String(value).toLowerCase()) ? '' : '1';
  return p?.values?.[value] ?? value;
}
const envName = (name) => `PLAYGUARD_SIM_${name.toUpperCase()}`;

// The variables and the config.json PlayGuard starts with: the page's
// language and theme, whatever else the URL sets.
function startEnvironment(values) {
  const env = { SDL_EMSCRIPTEN_KEYBOARD_ELEMENT: '#canvas' };
  const config = { language, theme: consoleTheme };
  for (const [name, value] of values) {
    if (name === 'lang' || name === 'theme') continue;
    const p = known.get(name);
    if (p?.page) continue;
    if (p?.config) { if (value) config[p.config] = value; continue; }
    const v = envValue(name, value);
    if (v !== '') env[envName(name)] = v;
  }
  return { env, config };
}

// A variable changed while PlayGuard runs (source/sim/web_page.cpp).
function setLive(name, value) {
  window.Module.ccall('playguard_web_setenv', 'number', ['string', 'string'], [envName(name), envValue(name, value)]);
  log(`live: ${name}=${value ?? ''}`);
}

// ---- Keeping the state (keep=1) ------------------------------------------------

let syncing = false;
let forgetting = false;
// The data folder to IndexedDB, at most one write at a time.
function saveData() {
  const FS = window.Module?.FS;
  if (!FS || syncing || forgetting) return;
  syncing = true;
  FS.syncfs(false, (e) => { syncing = false; if (e) log(`keep: ${e}`, true); });
}

// Before PlayGuard starts: the data folder from IndexedDB, then config.json
// with the page's language, theme and first tab over what it kept.
function mountKept(m, config) {
  m.FS.mkdirTree(DATA);
  m.FS.mount(m.FS.filesystems.IDBFS, {}, DATA);
  m.addRunDependency('keep');
  m.FS.syncfs(true, (e) => {
    if (e) log(`keep: ${e}`, true);
    let kept = {};
    try { kept = JSON.parse(m.FS.readFile(`${DATA}/config.json`, { encoding: 'utf8' })); } catch { /* first run */ }
    m.FS.writeFile(`${DATA}/config.json`, JSON.stringify({ ...kept, ...config }));
    setInterval(saveData, 3000);
    addEventListener('pagehide', saveData);
    addEventListener('visibilitychange', () => { if (document.hidden) saveData(); });
    m.removeRunDependency('keep');
  });
}

function forget() {
  forgetting = true;
  const done = () => location.reload();
  const req = indexedDB.deleteDatabase(DATA);
  req.onsuccess = req.onerror = req.onblocked = done;
}

// ---- The CJK font ------------------------------------------------------------

async function fetchCjk() {
  const res = await fetch(`cjk.otf${version ? `?v=${version}` : ''}`);
  if (!res.ok) throw new Error(`cjk.otf: ${res.status}`);
  return new Uint8Array(await res.arrayBuffer());
}
function writeCjk(m, bytes) {
  m.FS.mkdirTree(CJK_FILE.slice(0, CJK_FILE.lastIndexOf('/')));
  m.FS.writeFile(CJK_FILE, bytes);
}
// In Japanese, Korean or Chinese, PlayGuard starts once the font is there.
function preloadCjk(m) {
  m.addRunDependency('cjk');
  fetchCjk().then((bytes) => writeCjk(m, bytes), (e) => log(String(e), true)).finally(() => m.removeRunDependency('cjk'));
}
// In the others, it comes behind the Latin font once PlayGuard runs.
async function laterCjk() {
  try {
    const bytes = await fetchCjk();
    writeCjk(window.Module, bytes);
    window.Module.ccall('playguard_web_cjk_font', 'number', ['string'], [CJK_FILE]);
  } catch (e) {
    log(String(e), true);
  }
}

function start() {
  if (!self.crossOriginIsolated) {
    // sw.js reloads the page once its service worker is installed, which a
    // first visit on a slow connection can take a while to do. Without service workers (a private window in some
    // browsers) it never will; otherwise give it time before saying so.
    if (!('serviceWorker' in navigator) || !self.isSecureContext) { showError(t('no_isolation')); return; }
    setTimeout(() => showError(t('no_isolation')), 30000);
    return;
  }
  if (!document.createElement('canvas').getContext('webgl2')) { showError(t('no_webgl')); return; }

  // Screenshots read the canvas after the frame: keep its drawing buffer.
  const getContext = HTMLCanvasElement.prototype.getContext;
  HTMLCanvasElement.prototype.getContext = function (type, attrs) {
    if (this === canvas && /^webgl/.test(type)) attrs = { ...(attrs || {}), preserveDrawingBuffer: true };
    return getContext.call(this, type, attrs);
  };

  consoleTheme = effectiveTheme(chosenTheme());
  const { env, config } = startEnvironment(current);
  log(`settings: ${decodeURIComponent(toQuery(current)) || '(defaults)'}; language ${language}, theme ${consoleTheme}`);
  showStatus(t('downloading'));
  const keep = current.has('keep') && !FALSY.has(String(current.get('keep')).toLowerCase());
  window.Module = {
    canvas,
    // GitHub Pages lets browsers keep files 10 minutes: after a push, the
    // commit in their URL keeps an old .wasm from meeting a new .js.
    locateFile: (path, prefix) => `${prefix}${path}${version ? `?v=${version}` : ''}`,
    print: (line) => {
      log(line);
      if (line.includes('start-up: first screen ready') && document.body.dataset.playguard !== 'ready') {
        document.body.dataset.playguard = 'ready';
        ready();
      }
    },
    printErr: (line) => log(line, true),
    setStatus: (status) => {
      if (!status) return;
      // "Downloading data... (1234/5678)": how far the download is.
      const m = /\((\d+)\/(\d+)\)/.exec(status);
      if (m) showProgress(Number(m[1]), Number(m[2]));
      showStatus(status.startsWith('Downloading data') ? t('downloading') : status);
    },
    preRun: [(m) => {
      Object.assign(m.ENV, env);
      if (keep) {
        mountKept(m, config);
      } else {
        m.FS.mkdirTree(DATA);
        m.FS.writeFile(`${DATA}/config.json`, JSON.stringify(config));
      }
      if (CJK.has(language)) preloadCjk(m);
    }],
    onRuntimeInitialized: () => {
      running = true;
      document.body.dataset.playguard = 'running';
      showStatus(t('starting'));
      showProgress(1, 1);
      requestAnimationFrame(() => requestAnimationFrame(() => { $('overlay').hidden = true; canvas.focus(); }));
      if (!CJK.has(language)) laterCjk();
    },
    onAbort: (what) => showError(`${t('error')} ${what ?? ''}`),
  };
  window.addEventListener('error', (e) => {
    if (started) showError(`${t('error')} ${e.message ?? e.error ?? ''}`);
  });
  started = true;
  document.body.append(el('script', { src: `playguard.js${version ? `?v=${version}` : ''}` }));
}

// ---- Input ------------------------------------------------------------

function press(button, down) {
  window.Module?._brls_web_button?.(button, down ? 1 : 0);
}

// The keys SDL reads itself, as the buttons they press, for the steps.
const SDL_KEYS = { ArrowUp: BUTTON.UP, ArrowDown: BUTTON.DOWN, ArrowLeft: BUTTON.LEFT, ArrowRight: BUTTON.RIGHT, Enter: BUTTON.A, Escape: BUTTON.B };

function setUpKeys() {
  const held = new Map();
  canvas.addEventListener('keydown', (e) => {
    if (e.ctrlKey || e.metaKey || e.altKey) return;
    if (!e.repeat) noteButton(SDL_KEYS[e.key] ?? KEYS[e.key]);
    const button = KEYS[e.key];
    if (button == null) return;
    e.preventDefault();
    if (e.repeat) return;
    held.set(e.code, button);
    press(button, true);
  });
  canvas.addEventListener('keyup', (e) => {
    const button = held.get(e.code);
    if (button == null) return;
    held.delete(e.code);
    press(button, false);
  });
  canvas.addEventListener('blur', () => { for (const b of held.values()) press(b, false); held.clear(); });
  canvas.addEventListener('pointerdown', (e) => {
    canvas.focus();
    // A touch, in the console's 1280x720.
    const r = canvas.getBoundingClientRect();
    if (!e.isTrusted || !r.width) return;
    noteStep(`t${Math.round((e.clientX - r.left) / r.width * 1280)},${Math.round((e.clientY - r.top) / r.height * 720)}`);
  });
}

function setUpPad() {
  for (const button of $('pad').querySelectorAll('[data-button]')) {
    const code = Number(button.dataset.button);
    const up = (e) => { button.classList.remove('active'); press(code, false); e.preventDefault(); };
    button.addEventListener('pointerdown', (e) => {
      button.setPointerCapture(e.pointerId);
      button.classList.add('active');
      noteButton(code);
      press(code, true);
      e.preventDefault();
    });
    button.addEventListener('pointerup', up);
    button.addEventListener('pointercancel', up);
    button.addEventListener('contextmenu', (e) => e.preventDefault());
  }
  const toggle = $('pad-toggle');
  const show = (on) => {
    $('pad').hidden = !on;
    toggle.setAttribute('aria-pressed', String(on));
    try { localStorage.setItem('pg-pad', on ? '1' : '0'); } catch { /* storage refused */ }
  };
  let saved = null;
  try { saved = localStorage.getItem('pg-pad'); } catch { /* storage refused */ }
  show(saved ? saved === '1' : matchMedia('(pointer: coarse)').matches);
  toggle.addEventListener('click', () => show($('pad').hidden));
}

// A file name that says the state: playguard_unlocked_1.png.
const fileName = (ext) => `playguard${decodeURIComponent(toQuery(current)).replace(/[^a-z0-9.]+/gi, '_')}.${ext}`;

function download(blob, name) {
  const a = el('a', { href: URL.createObjectURL(blob), download: name });
  a.click();
  setTimeout(() => URL.revokeObjectURL(a.href), 1000);
}

function screenshot() {
  canvas.toBlob((blob) => { if (blob) download(blob, fileName('png')); }, 'image/png');
}

// A video of the screen (WebM, 30 frames a second) until Stop.
let recorder = null;
function record() {
  const button = $('record');
  if (recorder) { recorder.stop(); return; }
  const type = ['video/webm;codecs=vp9', 'video/webm'].find((m) => MediaRecorder.isTypeSupported(m));
  const chunks = [];
  recorder = new MediaRecorder(canvas.captureStream(30), type ? { mimeType: type } : {});
  recorder.ondataavailable = (e) => { if (e.data.size) chunks.push(e.data); };
  recorder.onstop = () => {
    recorder = null;
    button.textContent = t('record');
    button.setAttribute('aria-pressed', 'false');
    download(new Blob(chunks, { type: 'video/webm' }), fileName('webm'));
  };
  recorder.start(1000);
  button.textContent = t('stop_recording');
  button.setAttribute('aria-pressed', 'true');
  canvas.focus();
}

// ---- The time ------------------------------------------------------------

const skipNow = () => Math.max(0, Number(current.get('skip')) || 0);

function showSkip() {
  const s = skipNow();
  const h = Math.floor(s / 3600), min = Math.floor((s % 3600) / 60);
  $('skip-shown').textContent = s ? `+${h ? `${h} h ` : ''}${min || !h ? `${min} min` : ''}`.trim() : '';
  $('skip-reset').disabled = !s;
}

// The console's time moved on by `seconds` (0: back to now), a game played
// all along: live, so PlayGuard keeps running.
function moveTime(seconds) {
  const values = new Map(current);
  const s = seconds ? skipNow() + seconds : 0;
  if (s) values.set('skip', String(s)); else values.delete('skip');
  apply(values);
  showSkip();
  canvas.focus();
}

function setUpTime() {
  const box = $('time-buttons');
  for (const min of SKIPS) {
    const label = min % 60 ? t('plus_min').replace('{n}', min) : t('plus_hour').replace('{n}', min / 60);
    const button = el('button', { type: 'button', class: 'btn ghost', text: label });
    button.addEventListener('click', () => moveTime(min * 60));
    box.append(button);
  }
  $('skip-reset').addEventListener('click', () => moveTime(0));
  showSkip();
}

// ---- Steps ------------------------------------------------------------
//
// One step per dot: a button (up, down, left, right, a, b, x, y, l, r, zl,
// zr, plus, minus), `*N` to press it N times; `wN` waits N tenths of a
// second; `tX,Y` touches the screen (1280x720); `s:name=value` changes a live
// setting; `say:key` shows the page's text `tour_<key>` (the tour).

const STEP_BUTTONS = {
  up: BUTTON.UP, down: BUTTON.DOWN, left: BUTTON.LEFT, right: BUTTON.RIGHT, a: BUTTON.A, b: BUTTON.B,
  x: BUTTON.X, y: BUTTON.Y, l: BUTTON.L, r: BUTTON.R, zl: BUTTON.ZL, zr: BUTTON.ZR, plus: BUTTON.PLUS, minus: BUTTON.MINUS,
};
const BUTTON_STEP = Object.fromEntries(Object.entries(STEP_BUTTONS).map(([name, code]) => [code, name]));
// A touch's press, and the pause after a step: borealis' animations last
// 300 ms.
const PRESS_MS = 60;
const AFTER_MS = 400;
// The tour (tour=1): the tabs, then an hour of play.
const TOUR = 'w10.say:overview.w45.down.say:play_timer.w45.down.say:activity.w45.down*3.say:console.w45' +
             '.up*5.say:time.w15.s:skip=3600.w45.say:end.w60';

const sleep = (ms) => new Promise((done) => setTimeout(done, ms));
// The next display refresh, which runs one of PlayGuard's frames
// (emscripten_set_main_loop, source/main.cpp).
const frame = () => new Promise((done) => requestAnimationFrame(() => done()));
// A live value in a step: its dots are escaped too.
const stepValue = (name, value) => `${name}=${encodeURIComponent(value ?? '').replace(/\./g, '%2E')}`;

let steps = [];                 // what was done since the first screen
let lastStep = 0;
let replaying = null;           // the steps being replayed: { stop }

function noteStep(step) {
  if (!lastStep || replaying) return;
  const now = performance.now();
  const wait = Math.round((now - lastStep) / 100);
  if (wait >= 2) steps.push(`w${Math.min(wait, 600)}`);
  steps.push(step);
  lastStep = now;
}
function noteButton(code) {
  if (code != null && BUTTON_STEP[code]) noteStep(BUTTON_STEP[code]);
}

// The steps as a string: the same button pressed in a row, once with *N.
function stepsText(list) {
  const out = [];
  for (const step of list) {
    const last = out[out.length - 1];
    const m = last && /^([a-z]+)(?:\*(\d+))?$/.exec(last);
    if (m && m[1] === step && STEP_BUTTONS[step] != null) out[out.length - 1] = `${step}*${Number(m[2] || 1) + 1}`;
    else out.push(step);
  }
  return out.join('.');
}

function touch(x, y) {
  const r = canvas.getBoundingClientRect();
  const at = { clientX: r.left + x / 1280 * r.width, clientY: r.top + y / 720 * r.height, bubbles: true, button: 0 };
  canvas.dispatchEvent(new MouseEvent('mousedown', at));
  return sleep(PRESS_MS).then(() => canvas.dispatchEvent(new MouseEvent('mouseup', at)));
}

// Plays `text` (the steps above); `onStep(n, total, step)` before each.
// Resolves when done or stopped; throws on a step it does not know.
async function runSteps(text, onStep) {
  const list = String(text ?? '').split('.').filter(Boolean);
  const run = { stopped: false };
  replaying = run;
  try {
    for (let i = 0; i < list.length && !run.stopped; i++) {
      const step = list[i];
      onStep?.(i + 1, list.length, step);
      let m;
      if ((m = /^([a-z]+)(?:\*(\d+))?$/.exec(step)) && STEP_BUTTONS[m[1]] != null) {
        // A button is tapped: down and up at once, which brls_web_button()
        // keeps for the next frame as one press. Held for a while instead,
        // its release could come late on a busy page (SwiftShader's frames,
        // a timer run late) and borealis repeat the press: the smoke test's
        // tabs one off. Two refreshes then, so that a frame has read it
        // before the next tap.
        for (let n = Number(m[2] || 1); n > 0 && !run.stopped; n--) {
          press(STEP_BUTTONS[m[1]], true);
          press(STEP_BUTTONS[m[1]], false);
          await frame();
          await frame();
          await sleep(AFTER_MS);
        }
      } else if ((m = /^w(\d+)$/.exec(step))) {
        await sleep(Number(m[1]) * 100);
      } else if ((m = /^t(\d+),(\d+)$/.exec(step))) {
        await touch(Number(m[1]), Number(m[2]));
        await sleep(AFTER_MS);
      } else if ((m = /^s:([a-z0-9_]+)=(.*)$/.exec(step))) {
        const values = new Map(current);
        const value = decodeURIComponent(m[2]);
        if (value) values.set(m[1], value); else values.delete(m[1]);
        apply(values);
        showSkip();
      } else if ((m = /^say:([a-z_]+)$/.exec(step))) {
        $('steps-text').textContent = t(`tour_${m[1]}`);
      } else {
        throw new Error(`steps: no such step "${step}"`);
      }
    }
  } finally {
    if (replaying === run) replaying = null;
  }
}

// Replays steps with the notice above the screen, and Stop.
async function replay(text, tour) {
  const notice = $('steps-notice');
  notice.hidden = false;
  $('steps-stop').onclick = () => { if (replaying) replaying.stopped = true; };
  log(`steps: ${tour ? 'tour' : 'replaying'} ${text}`);
  try {
    await runSteps(text, (n, total, step) => {
      if (!tour && !step.startsWith('w')) $('steps-text').textContent = t('replaying').replace('{n}', n).replace('{total}', total);
    });
    log('steps: done');
  } catch (e) {
    log(String(e), true);
  } finally {
    notice.hidden = true;
    canvas.focus();
  }
}

// Once the first screen is up: steps are noted from now on, and those of
// the URL (rec=, tour=1) played. They leave the URL: a reload starts plain.
function ready() {
  lastStep = performance.now();
  const rec = current.get('rec');
  const tour = current.has('tour') && !FALSY.has(String(current.get('tour')).toLowerCase());
  if (rec || tour) {
    current.delete('rec');
    current.delete('tour');
    history.replaceState(null, '', linkFor(current));
    setTimeout(() => replay(tour ? TOUR : rec, tour), 1000);
  }
}

// The link: the settings PlayGuard started with, and the steps since.
function stepsLink() {
  const values = new Map(startValues);
  const text = stepsText(steps);
  if (text) values.set('rec', text);
  return linkFor(values);
}

async function copySteps() {
  const link = stepsLink();
  try { await navigator.clipboard.writeText(link); } catch { prompt(t('steps_copy'), link); return; }
  toast(t('steps_copied'));
}

// A bug report on GitHub, its form filled in: the steps link, the build, the
// browser and the end of the log. GitHub takes a URL of 8 KB or so.
function report() {
  const repo = buildInfo?.repo || REPO;
  const build = buildInfo ? `commit ${buildInfo.sha.slice(0, 7)}${buildInfo.pr ? `, pull request #${buildInfo.pr}` : ''}` : t('unknown_build');
  const url = (lines) => {
    const tail = logLines.slice(-lines).map(([line]) => line).join('\n');
    const body = `In the web simulator (${build}), these steps:\n${stepsLink()}\n\n` +
                  `Browser: ${navigator.userAgent}\n\nThe end of the log:\n\`\`\`\n${tail}\n\`\`\``;
    const q = new URLSearchParams({ template: '1-bug.yml', title: '[simulator] ', steps: body, version: `simulator, ${build}`, launcher: 'web simulator' });
    return `https://github.com/${repo}/issues/new?${q}`;
  };
  let lines = 40;
  while (lines > 0 && url(lines).length > 8000) lines -= 5;
  window.open(url(lines), '_blank', 'noopener');
}

// ---- The page ------------------------------------------------------------

function translatePage() {
  document.documentElement.lang = language;
  document.title = `PlayGuard · ${t('title')}`;
  for (const node of document.querySelectorAll('[data-i18n]')) node.textContent = t(node.dataset.i18n);
  for (const node of document.querySelectorAll('[data-i18n-html]')) node.innerHTML = t(node.dataset.i18nHtml);
  for (const node of document.querySelectorAll('[data-i18n-title]')) node.title = t(node.dataset.i18nTitle);
  for (const node of document.querySelectorAll('[data-i18n-label]')) node.setAttribute('aria-label', t(node.dataset.i18nLabel));
}

// The website's home page in the page's language (tools/build_site.py's folders).
function homeLink() {
  const folder = language === 'en-US' ? '' : `${language.toLowerCase()}/`;
  $('home-link').href = `../${folder}`;
}

function setUpHeader() {
  const lang = $('language-select');
  for (const [code, name] of LANGUAGES) lang.append(el('option', { value: code, text: name, lang: code }));
  lang.value = language;
  lang.addEventListener('change', () => {
    const values = new Map(current);
    values.set('lang', lang.value);
    apply(values);
  });
  const theme = $('theme-select');
  for (const choice of THEMES) theme.append(el('option', { value: choice, text: t(`theme_${choice}`) }));
  theme.value = chosenTheme();
  theme.addEventListener('change', () => chooseTheme(theme.value));
  dark.addEventListener('change', showTheme);
  $('theme-restart').addEventListener('click', () => location.reload());
}

// The build's commit (build.json, written by CI), or '' for a local build.
async function showBuild() {
  const box = $('build');
  try {
    const res = await fetch('build.json', { cache: 'no-cache' });
    if (!res.ok) throw new Error(res.status);
    const b = await res.json();
    const repo = `https://github.com/${b.repo}`;
    const where = b.pr ? el('a', { href: `${repo}/pull/${b.pr}`, text: `PR #${b.pr}` }) : el('span', { text: b.ref });
    const sha = el('a', { href: `${repo}/commit/${b.sha}` }, el('code', { text: b.sha.slice(0, 7) }));
    box.replaceChildren(where, ' · ', sha, b.title ? ` · ${b.title}` : '');
    box.title = b.title ?? '';
    buildInfo = b;
    return b.sha;
  } catch {
    box.textContent = t('unknown_build');
    return '';
  }
}

function setUpPanel() {
  const layout = document.querySelector('.layout');
  const toggle = $('panel-toggle');
  toggle.addEventListener('click', () => {
    // Narrow, the panel is under the screen: the button goes there.
    if (matchMedia('(max-width: 900px)').matches) { $('panel').scrollIntoView({ behavior: 'smooth' }); return; }
    const open = layout.classList.toggle('no-panel') === false;
    toggle.setAttribute('aria-expanded', String(open));
    if (open) return;
    canvas.focus();
  });
  for (const tab of document.querySelectorAll('.tab')) {
    tab.addEventListener('click', () => {
      for (const other of document.querySelectorAll('.tab')) other.setAttribute('aria-selected', String(other === tab));
      for (const body of document.querySelectorAll('.tab-body')) body.hidden = body.dataset.body !== tab.dataset.tab;
      if (tab.dataset.tab === 'log') renderLog();
    });
  }
  $('form').addEventListener('input', markChanges);
  $('form').addEventListener('change', markChanges);
  $('other').addEventListener('input', markChanges);
  $('form').addEventListener('submit', (e) => { e.preventDefault(); apply(readForm()); });
  $('apply').addEventListener('click', () => apply(readForm()));
  $('reset').addEventListener('click', () => apply(keepingHeader(() => false)));
  const keeping = current.has('keep') && !FALSY.has(String(current.get('keep')).toLowerCase());
  $('forget').hidden = !keeping;
  $('forget').addEventListener('click', forget);
  $('copy').addEventListener('click', async () => {
    const link = linkFor(readForm());
    try { await navigator.clipboard.writeText(link); } catch { prompt(t('copy'), link); return; }
    const button = $('copy');
    button.textContent = t('copied');
    setTimeout(() => { button.textContent = t('copy'); }, 1500);
  });
  $('other-add').addEventListener('click', () => {
    const name = $('other-name').value.trim().toLowerCase().replace(/^playguard_sim_/, '');
    if (!NAME.test(name)) { $('other-name').focus(); return; }
    const values = readForm();
    values.set(name, $('other-value').value.trim() || '1');
    $('other-name').value = $('other-value').value = '';
    buildOther(new Map([...values].filter(([n]) => !known.has(n))));
    markChanges();
  });
  $('shot').addEventListener('click', screenshot);
  $('record').hidden = typeof MediaRecorder === 'undefined' || !canvas.captureStream;
  $('record').addEventListener('click', record);
  $('steps-copy').addEventListener('click', copySteps);
  $('tour').addEventListener('click', () => { if (running && !replaying) replay(TOUR, true); });
  $('report').addEventListener('click', report);
}

async function main() {
  current = readUrl();
  startValues = new Map([...current].filter(([n]) => n !== 'rec' && n !== 'tour'));
  language = consoleLanguage(current.get('lang')) ?? browserLanguage();
  showTheme();
  try {
    const res = await fetch('params.json', { cache: 'no-cache' });
    manifest = await res.json();
  } catch (e) {
    log(`params.json: ${e}`, true);
  }
  known = new Map(manifest.params.map((p) => [p.name, p]));
  await loadTexts(language);
  const build = showBuild();
  translatePage();
  homeLink();
  setUpHeader();
  setUpPanel();
  setUpKeys();
  setUpPad();
  buildPresets();
  buildForm(current);
  setUpTime();
  version = await build;
  start();
  showTheme();
}

document.body.dataset.playguard = 'loading';
main();

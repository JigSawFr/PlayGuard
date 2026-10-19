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
'use strict';

// The console's languages, as PlayGuard's config.json names them, in their own words.
const LANGUAGES = [
  ['en-US', 'English'], ['fr', 'Français'], ['fr-CA', 'Français (Canada)'], ['de', 'Deutsch'],
  ['es', 'Español'], ['es-419', 'Español (Latinoamérica)'], ['it', 'Italiano'], ['nl', 'Nederlands'],
  ['pt', 'Português'], ['pt-BR', 'Português (Brasil)'], ['ru', 'Русский'], ['ja', '日本語'],
  ['ko', '한국어'], ['zh-Hans', '简体中文'], ['zh-Hant', '繁體中文'],
];
const THEMES = ['auto', 'light', 'dark'];

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
      placeholder: p.placeholder ?? '', min: p.min, inputmode: p.type === 'number' ? 'numeric' : null,
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
    const params = manifest.params.filter((p) => p.group === g.id && p.panel !== false);
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

function apply(values) {
  const query = toQuery(values);
  if (query === location.search || (query === '' && location.search === '')) { location.reload(); return; }
  location.search = query;   // reloads: PlayGuard starts again from scratch
}

// The page's language and theme stay; the rest is what `keep` says.
function keepingHeader(keep) {
  return new Map([...current].filter(([n]) => n === 'lang' || n === 'theme' || keep(n)));
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
    for (const [n, v] of Object.entries(preset.params)) values.set(n, v);
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

function showError(message) {
  const overlay = $('overlay');
  overlay.hidden = false;
  overlay.classList.add('error');
  const button = el('button', { type: 'button', class: 'btn primary', text: t('reload') });
  button.addEventListener('click', () => location.reload());
  overlay.replaceChildren(el('div', { class: 'spinner', 'aria-hidden': 'true' }), el('p', { text: message }), button);
}

// The variables and the config.json PlayGuard starts with: the page's
// language and theme, whatever else the URL sets.
function startEnvironment(values) {
  const env = { SDL_EMSCRIPTEN_KEYBOARD_ELEMENT: '#canvas' };
  const config = { language, theme: consoleTheme };
  for (const [name, value] of values) {
    if (name === 'lang' || name === 'theme') continue;
    const p = known.get(name);
    if (p?.config) { if (value) config[p.config] = value; continue; }
    if (p?.type === 'bool' && FALSY.has(String(value).toLowerCase())) continue;
    env[`PLAYGUARD_SIM_${name.toUpperCase()}`] = p?.type === 'bool' ? '1' : value;
  }
  return { env, config };
}

function start(version) {
  if (!self.crossOriginIsolated) {
    // coi-serviceworker reloads the page once it is installed (twice at
    // most); still here after that, this browser will not isolate it.
    setTimeout(() => showError(t('no_isolation')), 6000);
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
  window.Module = {
    canvas,
    // GitHub Pages lets browsers keep files 10 minutes: after a push, the
    // commit in their URL keeps an old .wasm from meeting a new .js.
    locateFile: (path, prefix) => `${prefix}${path}${version ? `?v=${version}` : ''}`,
    print: (line) => log(line),
    printErr: (line) => log(line, true),
    setStatus: (status) => { if (status) showStatus(status.replace(/^Downloading data\.\.\./, t('downloading'))); },
    preRun: [(m) => {
      Object.assign(m.ENV, env);
      m.FS.mkdirTree('/playguard_data');
      m.FS.writeFile('/playguard_data/config.json', JSON.stringify(config));
    }],
    onRuntimeInitialized: () => {
      showStatus(t('starting'));
      requestAnimationFrame(() => requestAnimationFrame(() => { $('overlay').hidden = true; canvas.focus(); }));
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

function setUpKeys() {
  const held = new Map();
  canvas.addEventListener('keydown', (e) => {
    if (e.ctrlKey || e.metaKey || e.altKey) return;
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
  canvas.addEventListener('pointerdown', () => canvas.focus());
}

function setUpPad() {
  for (const button of $('pad').querySelectorAll('[data-button]')) {
    const code = Number(button.dataset.button);
    const up = (e) => { button.classList.remove('active'); press(code, false); e.preventDefault(); };
    button.addEventListener('pointerdown', (e) => {
      button.setPointerCapture(e.pointerId);
      button.classList.add('active');
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

function screenshot() {
  canvas.toBlob((blob) => {
    if (!blob) return;
    const a = el('a', { href: URL.createObjectURL(blob), download: `playguard${decodeURIComponent(toQuery(current)).replace(/[^a-z0-9.]+/gi, '_')}.png` });
    a.click();
    setTimeout(() => URL.revokeObjectURL(a.href), 1000);
  }, 'image/png');
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
}

async function main() {
  current = readUrl();
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
  const version = showBuild();
  translatePage();
  setUpHeader();
  setUpPanel();
  setUpKeys();
  setUpPad();
  buildPresets();
  buildForm(current);
  start(await version);
  showTheme();
}

main();

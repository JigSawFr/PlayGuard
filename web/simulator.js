// The web simulator's page (web/README.md): reads the settings from the URL,
// shows them in the side panel (built from params.json), and starts PlayGuard
// (playguard.js, Emscripten) with them: PLAYGUARD_SIM_* variables, and a
// config.json for the language, the theme and the first tab. Applying new
// settings reloads the page with them in the URL, so a link always opens the
// same state.
'use strict';

const LANG = (navigator.language || 'en').toLowerCase().startsWith('fr') ? 'fr' : 'en';

const TEXT = {
  en: {
    title: 'Simulator',
    loading: 'Loading…',
    downloading: 'Downloading PlayGuard…',
    starting: 'Starting…',
    settings: 'Settings',
    log: 'Log',
    preset: 'Preset',
    preset_none: 'Choose a preset…',
    other: 'Other parameters',
    other_help: 'Any other PLAYGUARD_SIM_* variable, by its name in lower case without the prefix: a variable this page does not know yet (from a newer build) still reaches PlayGuard.',
    add: 'Add',
    apply: 'Apply',
    copy: 'Copy link',
    copied: 'Link copied',
    reset: 'Reset',
    pad: 'Pad',
    screenshot: 'Screenshot',
    note: 'Apply restarts PlayGuard from a fresh state with these settings; what you change inside it is not kept.',
    keys: '<kbd>↑↓←→</kbd> move · <kbd>Enter</kbd> A · <kbd>Esc</kbd> B · <kbd>X</kbd> <kbd>Y</kbd> · <kbd>L</kbd> <kbd>R</kbd> · <kbd>⇧L</kbd> ZL · <kbd>⇧R</kbd> ZR · <kbd>+</kbd> <kbd>−</kbd> · a click is a touch',
    error: 'PlayGuard stopped:',
    reload: 'Reload',
    no_isolation: 'This browser does not let the page run threads (no cross-origin isolation). Try a recent Chrome, Edge or Firefox, outside a private window.',
    no_webgl: 'This browser has no WebGL 2, which PlayGuard draws with.',
    unknown_build: 'local build',
    remove: 'Remove',
  },
  fr: {
    title: 'Simulateur',
    loading: 'Chargement…',
    downloading: 'Téléchargement de PlayGuard…',
    starting: 'Démarrage…',
    settings: 'Réglages',
    log: 'Journal',
    preset: 'Préréglage',
    preset_none: 'Choisir un préréglage…',
    other: 'Autres paramètres',
    other_help: 'Toute autre variable PLAYGUARD_SIM_*, par son nom en minuscules sans le préfixe : une variable que cette page ne connaît pas encore (d’une build plus récente) arrive quand même à PlayGuard.',
    add: 'Ajouter',
    apply: 'Appliquer',
    copy: 'Copier le lien',
    copied: 'Lien copié',
    reset: 'Réinitialiser',
    pad: 'Manette',
    screenshot: 'Capture',
    note: 'Appliquer redémarre PlayGuard d’un état neuf avec ces réglages ; ce que vous changez dans l’appli n’est pas conservé.',
    keys: '<kbd>↑↓←→</kbd> déplacer · <kbd>Entrée</kbd> A · <kbd>Échap</kbd> B · <kbd>X</kbd> <kbd>Y</kbd> · <kbd>L</kbd> <kbd>R</kbd> · <kbd>⇧L</kbd> ZL · <kbd>⇧R</kbd> ZR · <kbd>+</kbd> <kbd>−</kbd> · un clic est un toucher',
    error: 'PlayGuard s’est arrêté :',
    reload: 'Recharger',
    no_isolation: 'Ce navigateur ne permet pas à la page d’utiliser des threads (pas d’isolation cross-origin). Essayez un Chrome, Edge ou Firefox récent, hors navigation privée.',
    no_webgl: 'Ce navigateur n’a pas WebGL 2, avec lequel PlayGuard dessine.',
    unknown_build: 'build locale',
    remove: 'Retirer',
  },
};
const t = (key) => TEXT[LANG][key] ?? TEXT.en[key] ?? key;
const tr = (value) => (value && typeof value === 'object' ? value[LANG] ?? value.en : value ?? '');

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
  const label = tr(p.label);
  const help = p.help ? el('p', { class: 'help', text: tr(p.help) }) : null;
  if (p.type === 'bool') {
    const input = el('input', { type: 'checkbox', id, 'data-name': p.name });
    input.checked = value != null && !FALSY.has(String(value).toLowerCase());
    return [el('label', { class: 'field check', for: id }, input, el('span', { text: label })), help];
  }
  if (p.type === 'multi') {
    const chosen = new Set(String(value ?? '').split(',').filter(Boolean));
    const box = el('div', { class: 'multi', 'data-multi': p.name });
    for (const o of p.options) {
      const input = el('input', { type: 'checkbox', value: o.value, 'data-of': p.name });
      input.checked = chosen.has(o.value);
      box.append(el('label', { class: 'field check' }, input, el('span', { text: tr(o.label) })));
    }
    return [el('div', { class: 'field' }, el('span', { text: label })), box, help];
  }
  let input;
  if (p.type === 'select') {
    input = el('select', { id, 'data-name': p.name });
    for (const o of p.options) input.append(el('option', { value: o.value, text: tr(o.label) }));
    if (value != null && !p.options.some((o) => o.value === value)) input.append(el('option', { value, text: value }));
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
    const params = manifest.params.filter((p) => p.group === g.id);
    if (!params.length) continue;
    const set = params.filter((p) => values.has(p.name)).length;
    const details = el('details', { class: 'group', 'data-group': g.id },
      el('summary', {}, tr(g.label), set ? el('span', { class: 'count', text: String(set) }) : null));
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

// What the panel says now.
function readForm() {
  const out = new Map();
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

function buildPresets() {
  const select = $('preset');
  select.append(el('option', { value: '', text: t('preset_none') }));
  manifest.presets.forEach((p, i) => select.append(el('option', { value: String(i), text: tr(p.label) })));
  select.addEventListener('change', () => {
    const preset = manifest.presets[Number(select.value)];
    if (!preset) return;
    // The language, theme and first tab stay; the rest is the preset's.
    const values = new Map([...readForm()].filter(([n]) => known.get(n)?.config));
    for (const [n, v] of Object.entries(preset.params)) values.set(n, v);
    apply(values);
  });
}

// ---- PlayGuard ------------------------------------------------------------

let logLines = [];
function log(text, error) {
  // eslint-disable-next-line no-control-regex
  const line = String(text).replace(/\x1b\[[0-9;]*m/g, '');
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

function showStatus(text) {
  $('status').textContent = text;
}

function showError(message) {
  const overlay = $('overlay');
  overlay.hidden = false;
  overlay.classList.add('error');
  const button = el('button', { type: 'button', class: 'btn primary', text: t('reload') });
  button.addEventListener('click', () => location.reload());
  overlay.replaceChildren(el('div', { class: 'spinner', 'aria-hidden': 'true' }), el('p', { text: message }), button);
}

// The variables and the config.json PlayGuard starts with.
function startEnvironment(values) {
  const env = { SDL_EMSCRIPTEN_KEYBOARD_ELEMENT: '#canvas' };
  const config = {};
  for (const [name, value] of values) {
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

  const { env, config } = startEnvironment(current);
  log(`settings: ${decodeURIComponent(toQuery(current)) || "(defaults)"}`);
  showStatus(t('downloading'));
  window.Module = {
    canvas,
    // GitHub Pages lets browsers keep files 10 minutes: after a push, the
    // commit in their URL keeps an old .wasm from meeting a new .js.
    locateFile: (path, prefix) => `${prefix}${path}${version ? `?v=${version}` : ''}`,
    print: (text) => log(text),
    printErr: (text) => log(text, true),
    setStatus: (text) => { if (text) showStatus(text.replace(/^Downloading data\.\.\./, t('downloading'))); },
    preRun: [(m) => {
      Object.assign(m.ENV, env);
      if (Object.keys(config).length) {
        m.FS.mkdirTree('/playguard_data');
        m.FS.writeFile('/playguard_data/config.json', JSON.stringify(config));
      }
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
  document.documentElement.lang = LANG;
  document.title = `PlayGuard · ${t('title')}`;
  for (const node of document.querySelectorAll('[data-i18n]')) node.textContent = t(node.dataset.i18n);
  for (const node of document.querySelectorAll('[data-i18n-html]')) node.innerHTML = t(node.dataset.i18nHtml);
  for (const node of document.querySelectorAll('[data-i18n-title]')) node.title = t(node.dataset.i18nTitle);
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
  $('reset').addEventListener('click', () => apply(new Map()));
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
  translatePage();
  setUpPanel();
  setUpKeys();
  setUpPad();
  const version = showBuild();
  try {
    const res = await fetch('params.json', { cache: 'no-cache' });
    manifest = await res.json();
  } catch (e) {
    log(`params.json: ${e}`, true);
  }
  known = new Map(manifest.params.map((p) => [p.name, p]));
  current = readUrl();
  buildPresets();
  buildForm(current);
  start(await version);
}

main();

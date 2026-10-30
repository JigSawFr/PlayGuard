# File and message formats

Part of the [contributor's guide](../../CONTRIBUTING.md).

Every format one version of PlayGuard (or the agent, the gate, the recovery
sysmodule, the Home Assistant integration) writes and another reads carries a
version: a rollback, a development build, the simulator or an agent left
behind all meet files from another version. The rule:

1. **A `schema` integer**, one named constant per format (`schema` in a JSON
   document, `schema=N` in a `key=value` file, magic + version + size in a
   binary, the protocol number of the IPC with the size of each structure
   checked at build time). Never a repeated literal.
2. **Adding a field keeps the schema; a field that changes meaning bumps it.**
3. **A reader skips what it does not know**: an unknown field is ignored, a
   newer `schema` is read as absent (defaults, the previous copy), never as an
   error. A missing `schema` counts as 1 for the files that existed before it.
   JSON readers use `json_util::newer_schema`.
4. **A file of a newer `schema` is never written over**: the app works in
   memory, or leaves the change unsaved.
5. **A rewrite keeps the keys it does not know**, so a rollback loses nothing.
6. **A document served to installed versions keeps its old schema
   published** next to the new one (`compat.json` and `compat-<N>.json`).
7. **Each reader is tested** with `schema` missing, newer (ignored, the file
   left as it is) and an unknown field (kept on rewrite).

## The formats

| Format | Written by → read by | Version | Newer | Unknown fields | Tests |
|---|---|---|---|---|---|
| `config.json` | `util/config.cpp` → itself; `core/gate.c` (language and theme only), the web simulator | `config::SCHEMA` 1 | defaults in memory, never written | kept on save | `tests/config/` |
| `nand/<system>/state.json` | `util/config.cpp` | `config::SCHEMA` 1 | defaults in memory, never written | kept on save | `tests/config/` |
| `nand/<system>/history.json` | `util/history.cpp` | `history::SCHEMA` 1 | read as empty, not put aside, never written (changes not recorded) | an entry's skipped | `tests/history/` |
| `nand/<system>/watch.json` | `action/outside_change_logic.cpp` | `outside_change_logic::SCHEMA` 1 | a fresh record, never saved | skipped | `tests/outside_change_logic/` |
| `nand/<system>/forwarders.json` | `util/forwarders.cpp` | `forwarders::SCHEMA` 1 (none before: 1) | read as none, never written | kept on save (the file's and each entry's) | `tests/forwarders/` |
| `github_token` | `util/github_auth.cpp` | the `v1:` prefix of `util/secret_box` | another console's box, or one this version cannot open: signed out, the file left as it is | a plain token (an older version's) sealed where it lies | `tests/secret_box/` |
| `migrations.json` | `util/migrations.cpp` | `migrations::SCHEMA` 1 + `level` | a higher `level` brought back, nothing runs | skipped | `tests/migrations/` |
| `nand/<system>/backups/*.json` | `util/backup.cpp` | `FORMAT` 1 (`format`), strict | refused | skipped | `tests/backup/` |
| `compat.json`, `compat-<N>.json` (release assets) | `tools/gen_compat.py` → `util/update.cpp` | `update::COMPAT_SCHEMA` 1; `compat.json` stays 1 forever | `compat-<COMPAT_SCHEMA>.json` fetched first, `compat.json` when missing or unreadable | skipped | `tests/update/` |
| `sync.conf` | `sync/sync_conf.c` (PlayGuard and the agent) | `SYNC_CONF_SCHEMA` 1 | read for the keys known, never written (by either) | kept (up to `SYNC_EXTRA_MAX`; beyond, the file is not written) | `tests/sync_core/`, `tests/sync_files/` |
| `nand/<system>/sync/agent_events.log` | agent `main.c` → `util/sync_files.cpp` | the number of fields (10 or 11) | a line with more fields read for the known ones | an unreadable line kept in `agent_events.log.rejected` | `tests/sync_files/` |
| `nand/<system>/sync/titles.txt` | `util/titles.cpp` → `sync/sync_titles.c` (the agent) | none (one word per title) | an unknown kind read as `homebrew`: never published by its ID | — | `tests/sync_core/`, `tests/titles/` |
| `nand/<system>/sync/integration.json` | the agent or PlayGuard, as the integration sent it → `util/sync_files.cpp` | `sync_files::INTEGRATION_SCHEMA` 1 | kept, not read | skipped | `tests/sync_files/` |
| `nand/<system>/sync/nro_state.txt`, `agent_state.txt` | `sync/sync_records.c` | none | a missing `read_only` writes nothing | skipped | `tests/sync_files/`, `tests/agent/` |
| IPC `pg:agent` | `sync/agent_ipc.h` (PlayGuard ↔ agent) | `AGENT_PROTOCOL` 3; each structure's size checked at build time (`AGENT_SIZE_*`) | `AgentHello` / `AgentHelloReply` never change: the reply says the protocol, no session | — | `tests/agent/` |
| MQTT documents (`state`, `system`, `games`, `activity`, `names`, `week`, `event`) | `source/sync/` → the Home Assistant integration | `SYNC_SCHEMA` 1 | the integration's repair issue | skipped | `tests/sync_core/`, `tools/check_sync_json.py` |
| MQTT orders (`<entity>/set`) | the integration → `sync/sync_engine.c` | `state`'s `schema` (the console announces what it accepts) | an unknown entity refused (`unknown_entity`), the order cleared | — | `tests/sync_engine/` |
| `gate_log.txt` | the gate (`core/gate_nx.c`) → `action/forwarder_rules_logic.cpp` | none | an unknown result kept as `other` (history: *Other change*) | — | `tests/gate/`, `tests/forwarder_rules_logic/` |
| Game and play-time caches (`cache/*.bin`) | `util/game_cache.cpp`, `util/play_cache.cpp` | magic + `VERSION` + structure size | made again | — | `tests/game_cache/`, `tests/play_cache/` |
| `play_timer/log.csv` | `util/pt_log.cpp` | its header | rotated | — | `tests/pt_log/` |
| Recovery reports and proof | `sysmodule/rescue` → `core/rescue.c` | label `playguard-rescue/2` | an older proof read as none | — | `tests/rescue/` |
| boreale.dev's `versions.json` (the site's menu of Boréale's sites) | `JigSawFr/boreale` → boreale-site's `read_versions()` (`library/boreale-site`) | `VERSIONS_SCHEMA` (1) | another schema or shape: no versions in the menu | — | boreale-site's `tests/` |

A format that changes meaning: bump its constant, keep reading the previous
schema where installed versions still write it, and update this table and the
format's own page ([`config.md`](../config.md),
[`sync-protocol.md`](../sync-protocol.md)).

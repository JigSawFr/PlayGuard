<!-- translated from docs/install.md, sha256 900d9a183a17 -->
# 설치

PlayGuard를 설치, 업데이트, 삭제하는 방법과 PlayGuard가 SD 카드에 저장하는 파일을 설명합니다. 설치한 뒤에는 [시작하기](getting-started.md)를 보세요. 처음으로 돌아가기: [README](../../README.md).

## 받는 곳

다음 중 하나를 고르세요.

- **Homebrew App Store** / **sphaira의 App Store**(같은 카탈로그) — *PlayGuard*를 검색해 설치하세요. 새 버전은 검토를 거친 뒤 자동으로 반영되며, 보통 GitHub보다 며칠 늦습니다. PlayGuard는 그곳에서 설치되었다는 것을 알고 스토어를 통해 업데이트하도록 권장합니다.
- **수동 설치** — [최신 버전](https://github.com/JigSawFr/PlayGuard/releases/latest)에서 `playguard.zip`을 내려받아 SD 카드의 **루트**에 압축을 푸세요. 앱은 `sd:/switch/playguard/`에, 앱의 파일은 `sd:/config/playguard/`에 들어갑니다.
- **sphaira › GitHub** — 배포 zip에 항목(`/config/sphaira/github/playguard.json`)이 이미 들어 있습니다. 처음 설치한 뒤에는 sphaira의 *GitHub*에서 업데이트하세요.

선택 사항인 **복구 sysmodule**은 PlayGuard에서 설치하거나(*본체 › 추가 모듈* 또는 *보안 › 본체가 잠겼나요?*), 별도로 내려받은 파일(`playguard-rescue.zip`)로 직접 설치합니다. 직접 설치할 때는 이것도 SD 카드의 루트에 압축을 푼 뒤 본체를 다시 시작하세요. `atmosphere/contents/4200000000505247/`에 들어가며, `RESCUE` 파일이 요청할 때만 작동합니다. [본체가 잠겼나요?](troubleshooting.md#본체가-잠겼나요)를 참고하세요.

그다음 보호자의 사용자로 PlayGuard를 **게임 위에서** 여세요([이유](compatibility.md#런처)).

## 업데이트

- **PlayGuard에서:** *환경 설정 › 업데이트 방법*에서 방법을 고릅니다. PlayGuard 자체에서(버전을 내려받아 확인하고 자신을 교체한 뒤 다시 시작), sphaira, Homebrew App Store, 수동 중 하나입니다. *매일 업데이트 확인*을 켜 두면 새 버전이 있을 때 PlayGuard가 그 버전 화면으로 열립니다.
- **복구 sysmodule**은 PlayGuard 버전에 맞춰야 합니다. 업데이트할 때마다 그 버전의 `playguard-rescue.zip`을 다시 압축 해제하고(또는 벨의 *복구 sysmodule이 최신이 아닙니다*에서 *업데이트*, 또는 *본체 › 추가 모듈*에서), 본체를 다시 시작하세요.
- **원격 연결 에이전트**는 설치되어 있다면, PlayGuard에 더 새로운 에이전트가 들어 있을 때 다음 시작 시 제안됩니다([Home Assistant](../home-assistant.md#keep-the-link-up-when-playguard-is-closed), 영어).
- 설정, 변경 기록, 프로필, 백업은 그대로 남습니다. 앱 옆이 아니라 `sd:/config/playguard/`에 있기 때문입니다.

## 삭제

`sd:/switch/playguard/`(앱)를 삭제하고, 앱의 파일까지 지우려면 `sd:/config/playguard/`도 삭제하세요. 선택 사항인 sysmodule을 설치했다면 `sd:/atmosphere/contents/4200000000505247/`(복구)와 `sd:/atmosphere/contents/4200000000504741/`(에이전트)도 삭제하거나, 먼저 *본체 › 추가 모듈*에서 제거하세요. 플레이 시간 제한, PIN, 제한 설정은 본체 자체의 것이므로 **본체에 그대로 남습니다**. 이것들도 없애려면 먼저 *보안 › 자녀 보호 기능 모두 삭제*를 하세요.

## SD 카드의 파일

모두 `sd:/config/playguard/`에 있습니다(앱 자체는 `sd:/switch/playguard/`에 있으므로 앱을 삭제하거나 다시 설치해도 이 파일들은 남습니다). 파일을 앱 옆에 두던 이전 버전에서는 처음 실행할 때 이곳으로 옮겼습니다.

- **`config.json`** — 환경 설정(언어, 테마, NTP 서버, *PIN으로 PlayGuard 보호* 등). 모든 키는 [config.md](../config.md)(영어)에 있습니다. 읽을 수 없는 파일은 `config.json.bad`로(그 이전 파일은 `.bad.1`로) 보관되고 설정은 기본값으로 돌아갑니다. PlayGuard가 시작할 때 한 번 알려 줍니다
- **`profiles/`** — 저장한 플레이 시간 제한 프로필
- **`exports/`** — 플레이 기록 내보내기 파일
- **`nand/<system>/`** — 시스템마다 따로 저장되는 파일입니다. 시스템마다 자녀 보호 기능이 따로 있습니다. sysMMC는 `sysmmc/`, emuMMC는 `emummc/`입니다. 안에는 다음이 있습니다. `history.json`은 변경 기록입니다(최근 500개, 본체가 거부한 변경도 포함. 읽을 수 없는 파일은 덮어쓰지 않고 `history.json.bad`로, 그 이전 파일은 `.bad.1`로 보관하며 PlayGuard가 한 번 알려 줍니다). `state.json`은 *모든 소프트웨어 차단*, 추가 시간, 대기 중인 재잠금, 펌웨어 선택을 위해 기억해 두는 내용입니다([config.md](../config.md#state-kept-between-launches), 영어). `forwarders.json`은 PlayGuard가 만든 포워더입니다. `watch.json`은 PlayGuard가 마지막으로 본 본체 상태로, PlayGuard 밖에서 바뀐 것을 알아내는 데 씁니다([config.md](../config.md#watchjson), 영어. 삭제해도 문제없습니다). `backups/`는 설정 백업입니다(PIN은 절대 들어가지 않습니다). `cache/`는 마지막으로 읽은 플레이 기록(모든 사용자, 그리고 열어 본 각 사용자)으로, 다음에 시작할 때 기록을 다시 읽는 동안 바로 표시됩니다. 이 폴더들이 생기기 전 버전의 파일은 PlayGuard가 처음 시작하는 시스템으로 옮겨집니다
- **`cache/`** — 게임 아이콘(`icons/`, 최대 16 MB, 본체 언어가 바뀌면 비워짐)으로, 모든 시스템이 함께 씁니다. 시작할 때마다 게임마다 이름과 아이콘을 다시 읽지 않기 위한 것입니다. Atmosphère, hekate, sys-patch, 펌웨어의 릴리스 노트(`sys_notes.json`, *본체*용)와 PlayGuard 자체의 릴리스 노트(*정보*용)도 있습니다. 개발자 모드에서는 *다른 빌드 설치*의 목록(`dev_builds.json`)도 있습니다
- **`github_token`** — *보고서 보내기…*와, 개발자 모드에서는 *다른 빌드 설치*에 쓰는 GitHub 로그인 정보입니다. 로그아웃하면 삭제됩니다(*정보 › 도움말 및 진단 › GitHub 계정*, 로그인한 뒤에 표시)
- **`gate.nro`**, **`gate-<hash>.nro`**, **`gate_*.txt`** — 보호된 포워더의 게이트(*본체 › 포워더*)입니다. 그 프로그램, PIN 없는 시간, 열리지 않는 시간, 거부 후 일시 정지, PlayGuard가 읽을 때까지의 시도 기록, 시작 화면이 표시되는 시간(`gate_wait.txt`, 시작할 때마다 읽음)
- **`previous_build.json`**, **`boot_check.json`** — 빌드를 그 자리에 설치한 뒤에 쓰입니다. 교체된 빌드의 이름(`.nro` 옆에 `playguard.nro.previous`로 보관)과 다음 시작 확인용 파일(첫 화면이 뜨면 삭제)입니다
- **`rescue_report.txt`** — 복구 sysmodule이 작동한 뒤 남기는 파일로, PlayGuard가 시작할 때 표시할 때까지 남아 있습니다(읽을 수 없는 파일은 삭제되고 PlayGuard가 알려 줍니다)
- **`rescue_nonce`** — 복구 sysmodule이 보고서 안에서 답하는 무작위 확인 값입니다. 이것으로 PlayGuard는 sysmodule이 쓴 보고서와 직접 손으로 쓴 보고서를 구별합니다([sysmodule/rescue/README.md](../../sysmodule/rescue/README.md), 영어). 시작할 때마다 새로 만들어집니다
- **`sync.conf`** — *원격 접속*을 설정한 뒤에만 생깁니다. 연결 설정이며 브로커 비밀번호도 들어 있습니다(보고서나 업로드에는 절대 들어가지 않습니다)
- **`sync/`** — *원격 접속*을 쓸 때만 생깁니다. 선택 사항인 에이전트를 위한 저장된 프로필(`profiles.txt`)이 있으며, 에이전트가 한 시스템에 대해 읽고 쓰는 것은 그 시스템의 `nand/<system>/sync/`에 있습니다(기록, 게임 이름, 바꾼 내용, 이벤트). sysMMC와 emuMMC는 `sync.conf`를 함께 쓰지만 Home Assistant에서는 서로 다른 두 기기입니다
- **`logs/`** — `reports/`는 저장한 진단 보고서(PIN이나 시리얼 번호는 절대 들어가지 않습니다), `crashes/`는 충돌할 때마다 남는 파일(PlayGuard가 멈춘 원인), `uploads/`는 온라인으로 보낸 보고서마다 하나씩 만드는 인터넷 바로 가기(`.url`), `play_timer/`는 개발자 도구의 플레이 타이머 파일입니다. 보고서, 충돌 파일, 링크를 몇 개 보관할지는 *정보 › 도움말 및 진단*에서 정합니다(개수, 기간, 크기 기준. 가장 최근 것은 항상 남습니다)

자세한 내용은 [packaging/README.md](../../packaging/README.md)(영어)를 참고하세요.

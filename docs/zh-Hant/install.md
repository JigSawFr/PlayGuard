<!-- translated from docs/install.md, sha256 7d2be6a20dff -->
# 安裝

如何安裝、更新和移除 PlayGuard，以及它會在 SD 卡上寫入哪些檔案。安裝之後：[開始使用](getting-started.md)。返回 [README](../../README.md)。

## 取得方式

> [!WARNING]
> PlayGuard **還沒有上架 Homebrew App Store（也還沒有上架 sphaira 的商店）**：它的商店頁面仍在等待審核。目前請手動安裝。

任選一種：

- **手動**（目前建議使用）——從[最新版本](https://github.com/JigSawFr/PlayGuard/releases/latest)下載 `playguard.zip`，解壓縮到 SD 卡的**根目錄**。應用程式會放在 `sd:/switch/playguard/`，它的檔案則在 `sd:/config/playguard/`。
- **sphaira › GitHub**——版本的 zip 檔已包含對應的項目（`/config/sphaira/github/playguard.json`）：第一次安裝之後，在 sphaira 中從 *GitHub* 更新即可。
- **Homebrew App Store** / **sphaira 的 App Store**（同一個目錄）——**尚未上架。** 商店頁面通過審核後，搜尋 *PlayGuard* 即可。之後新版本會在審核後自動提供：通常比 GitHub 晚幾天。PlayGuard 知道自己是從那裡安裝的，會建議透過商店更新。

選用的**復原 sysmodule**（`playguard-rescue.zip`）需要另外下載：同樣解壓縮到 SD 卡的根目錄，然後重新啟動主機。它會放在 `atmosphere/contents/4200000000505247/`，只有在 `RESCUE` 檔案要求時才會動作——請見[被鎖在外面？](troubleshooting.md#被鎖在外面)。

接著請**在遊戲之上**、用家長的使用者開啟 PlayGuard（[原因](compatibility.md#啟動器)）。

## 更新

- **在 PlayGuard 中：** *偏好設定 › 更新方式*可以選擇更新的方式——在 PlayGuard 內（它會下載版本、檢查、取代自己並重新啟動）、sphaira、Homebrew App Store 或手動。開啟*每天檢查更新*時，有新版本時 PlayGuard 會開在新版本的畫面上。
- **復原 sysmodule** 要跟上 PlayGuard 的版本：每次更新後，都要重新解壓縮該版本的 `playguard-rescue.zip`（或在通知鈴鐺的*復原 sysmodule 不是最新版本*上選*更新*），然後重新啟動主機。
- 設定、紀錄、設定檔和備份都會保留：它們在 `sd:/config/playguard/`，不在應用程式旁邊。

## 移除

刪除 `sd:/switch/playguard/`（應用程式），如果也要移除它的檔案，再刪除 `sd:/config/playguard/`；裝了復原 sysmodule 的話，也要刪除 `sd:/atmosphere/contents/4200000000505247/`。上限、PIN碼和限制都是主機本身的：它們**會留在主機上**。如果也要移除這些，請先使用*安全 › 刪除所有家長監護設定*。

## SD 卡上的檔案

全部都在 `sd:/config/playguard/`（應用程式本身留在 `sd:/switch/playguard/`：刪除或重新安裝應用程式都不會影響這些檔案）。舊版本把這些檔案放在應用程式旁邊，第一次啟動時已將它們移到這裡。

- **`config.json`**——偏好設定（語言、主題、NTP 伺服器、*用PIN碼保護PlayGuard*……）；每個鍵的說明見 [config.md](../config.md)（英文）。無法讀取時，它會被保留為 `config.json.bad`（前一份保留為 `.bad.1`），設定會恢復為預設值；PlayGuard 會在啟動時提示一次
- **`profiles/`**——已儲存的遊玩時間上限設定檔
- **`exports/`**——匯出的活動紀錄
- **`nand/<system>/`**——屬於某一個系統的資料，每個系統都有自己的家長監護：`sysmmc/` 是 sysMMC 的，`emummc/` 是 emuMMC 的。裡面有：`history.json`，變更紀錄（最新的 500 筆，包括遭主機拒絕的變更；無法讀取時會保留為 `history.json.bad`、前一份保留為 `.bad.1`，而不是直接覆蓋，PlayGuard 會提示一次）；`state.json`，*封鎖所有遊戲*、延長時間、待重新啟用的鎖定以及韌體選擇需要記住的內容（[config.md](../config.md#state-kept-between-launches)，英文）；`watch.json`，PlayGuard 上次看到的主機狀態，用來判斷在它之外有哪些變更（[config.md](../config.md#watchjson)，英文；刪除它沒有影響）；`backups/`，設定的備份（絕不包含PIN碼）；`cache/`，最近一次讀取的遊玩活動（所有帳號，以及每個看過的帳號），下次啟動時會立即顯示，同時重新讀取記錄。這些資料夾出現之前的版本所留下的檔案，會歸到 PlayGuard 第一次啟動的那個系統
- **`cache/`**——遊戲圖示（`icons/`，最多 16 MB，主機語言變更時會清空），所有系統共用，這樣每次啟動時就不必重新讀取每款遊戲的名稱和圖示；Atmosphère、hekate、sys-patch 和韌體的版本說明（`sys_notes.json`，供*主機*使用）以及 PlayGuard 自己的版本說明（供*關於*使用）；在開發者模式下，還有*安裝其他建置*的清單（`dev_builds.json`）
- **`github_token`**——*線上傳送報告*，以及開發者模式下*安裝其他建置*所使用的 GitHub 登入資訊；登出時會刪除（*關於 › 說明與診斷 › GitHub 帳戶*，登入後才會顯示）
- **`previous_build.json`**、**`boot_check.json`**——直接在原處安裝建置之後：被取代的建置名稱（以 `playguard.nro.previous` 保留在 `.nro` 旁邊），以及下次啟動時的檢查（第一個畫面出現後即刪除）
- **`rescue_report.txt`**——復原 sysmodule 執行動作後留下的報告，直到 PlayGuard 在啟動時顯示它為止（無法讀取的報告會被刪除，PlayGuard 也會提示）
- **`rescue_nonce`**——一個隨機的驗證值，復原 sysmodule 會在報告中回應它，讓 PlayGuard 能分辨報告是 sysmodule 寫的還是手動寫的（[sysmodule/README.md](../../sysmodule/README.md)，英文）；每次啟動都會產生新的
- **`logs/`**——`reports/`，已儲存的診斷報告（絕不包含PIN碼或序號）；`crashes/`，每次當機一個檔案（讓 PlayGuard 停止的原因）；`uploads/`，每份線上傳送的報告各一個網際網路捷徑（`.url`）；`play_timer/`，開發者工具的遊玩時間記錄檔。要保留多少報告、當機檔案和連結：*關於 › 說明與診斷*（依數量、天數或大小；最新的一份一定會保留）

更多細節請見 [packaging/README.md](../../packaging/README.md)（英文）。

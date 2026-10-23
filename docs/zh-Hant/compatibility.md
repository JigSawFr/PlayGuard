<!-- translated from docs/compatibility.md, sha256 2e539d2ecda1 -->
# 相容性

支援的韌體、Atmosphère 和啟動器，emuMMC 和 sysMMC，以及 PlayGuard 與替代型 sysmodule 的比較。返回 [README](../../README.md)。

## 支援的版本

**已在主機上測試**——PlayGuard 曾在以下環境執行：

- **韌體 23.0.1**——Atmosphère 1.12.1
- **韌體 22.0.0、22.1.0、22.5.0**——Atmosphère 1.11.x
- hbmenu、sphaira 和 Homebrew App Store

**應該可用，尚未測試**——PlayGuard 沒有任何地方排除這些版本，但還沒有人回報過（[歡迎回報](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)）。每個韌體都需要能啟動它的 Atmosphère：列出的版本或任何更新的版本。

- **韌體 23.0.0**——Atmosphère 1.12.0。在 [switchbrew](https://switchbrew.org/wiki/Parental_Control_services) 的表格中，指令與 23.0.1 相同。
- **韌體 21.0.0 → 21.2.0**——Atmosphère 1.10.x。所有功能皆可使用。

**21.0.0 之前：除了遊玩時間限制以外都可用**——限制、PIN碼、暫時解除、活動紀錄、網路時鐘、手機應用程式和紀錄都能使用。遊玩時間限制的設定區塊（0x44 位元組）從 21.0.0 起才是這個格式，所以 PlayGuard 會關閉依賴它的功能：每日上限、就寢鬧鐘、延長時間和*封鎖所有遊戲*。

- **韌體 20.0.0 → 20.5.0**——Atmosphère 1.9.x
- **韌體 19.0.x**——Atmosphère 1.8.0
- **韌體 18.0.0 → 18.1.0**——Atmosphère 1.7.x
- **韌體 17.0.x**——Atmosphère 1.6.x
- **韌體 16.0.0 → 16.1.0**——Atmosphère 1.5.x
- **韌體 15.0.x**——Atmosphère 1.4.x
- **韌體 14.x**——Atmosphère 1.3.x
- **韌體 13.x**——Atmosphère 1.1.0 → 1.2.6
- **韌體 12.x 及更早版本**——Atmosphère 1.0.0 及更早版本

PlayGuard 只檢查 Atmosphère 是否在執行，不檢查它的版本：重要的是它啟動的韌體。*主機*會顯示兩者。

## 須知

### 啟動器

建議在遊戲之上啟動（title override）。應用程式會顯示它是以應用程式還是 applet（相簿）的方式執行。在遊戲之上執行時，主機會把在 PlayGuard 裡的時間算成那款遊戲的時間。在活動紀錄中，這段時間會算在啟動時選擇的使用者身上：請用家長的使用者開啟，不要用孩子的。遊玩時間限制是主機的，所有使用者共用同一個：不管是誰開啟 PlayGuard，這段時間都會被計算（除非計時器已關閉）。

### Nintendo Switch 2

不支援：Switch 2 沒有 Atmosphère。

### 韌體比較新

如果韌體比在主機上測試過的最新韌體還新，PlayGuard 會以**唯讀**方式開啟，並檢查是否有更新的版本支援它。如果有，它會建議更新（在 PlayGuard 內、透過 sphaira 或 Homebrew App Store）。如果沒有，由你選擇：唯讀、唯讀並開啟開發者工具（用於調查這個韌體），或使用所有功能、風險自負。這個選擇可以針對該韌體和該應用程式版本記住；*主機 › 相容性*可以再次叫出這個畫面。

### emuMMC 和 sysMMC

兩者各有各的家長監護，PlayGuard 只能看到它所執行的那個系統的設定。在 emuMMC 上，*主機*和*初始設定*會提醒這一點；在 sysMMC 上，如果 SD 卡上設有 emuMMC，也會提醒。請在兩邊都設定家長監護，或隱藏 hekate 的開機選單，讓孩子無法選擇另一個系統。PlayGuard 會把每個系統的紀錄、備份和狀態分開保存（sysMMC 和 emuMMC，在 `nand/` 中）；偏好設定和設定檔則是共用的。

### 主機會被封鎖嗎？

PlayGuard 從不連線任天堂：連網時，它只會存取公用時間伺服器和 GitHub（更新，以及你傳送的報告）。導致主機被封鎖的，是自訂韌體或 emuMMC 連線到任天堂（[switchbrew FAQ](https://switchbrewdocs.readthedocs.io/en/latest/faq.html)，英文）。目前沒有因修改家長監護或時鐘而被封鎖的記錄；但這不是保證。

### 沒有網際網路的主機

這正是 PlayGuard 的用途，而遊玩時間限制仍然需要一個設定過的網路時鐘。沒有時間伺服器可以詢問時，*主機 › 網路時鐘 › 測量時間差*會改為提議把網路時鐘設為主機時鐘：請先確認主機時鐘，同時讓*透過網際網路同步時鐘*保持關閉，之後再開啟這個設定（請見[網路時鐘](features.md#沒有伺服器回應)）。

### 手機通知

從 22.0.0 起，仍與手機應用程式連動的主機，在主機上輸入PIN碼時可能會通知家長（[switchbrew](https://switchbrew.org/wiki/22.0.0)，英文）。目前不知道 PlayGuard 自己的解除是否也會觸發通知。

## 為什麼要做 PlayGuard

孩子喜歡電玩，而電玩就是螢幕：設定上限是照顧他們的一部分。在未改機的 Switch 上，這件事由任天堂的手機應用程式負責。主機一旦執行自訂韌體，就打開了一個全新的世界——家長監護卻也跟著消失了，因為手機應用程式再也連不到主機。想知道孩子玩了多久、設定上限、讓遊戲停下來而不必爭吵、不必再聽一次「再五分鐘就好」，都變得很困難。

現有的少數工具都不夠好：活動報告幾乎沒有維護、內容也不詳細；家長監護功能粗糙、尚未完成。我也不想要自製的替代品，或一個一直在背景執行的 sysmodule。想法是**盡可能重用主機本身的家長監護**，連同它已經能做的一切——時間上限、PIN碼、提醒、暫停——再把它的設定帶回主機上。這就是 PlayGuard。

操作主機本身的家長監護，也為更多可能打開了大門：把遊玩時間回報給家中的伺服器、接受它的指令、Home Assistant、自動化。可能的發展方向請見[路線圖的遠景](../../ROADMAP.md#horizon)（英文）。

## PlayGuard 還是替代型 sysmodule？

PlayGuard 操作的是主機內建的家長監護（`pctl` 服務），完全離線。另一種做法是用 sysmodule 取代它，例如 [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl)（一個 sysmodule 加上一個 Tesla / Ultrahand overlay）。

| | PlayGuard | 替代型 sysmodule |
|---|---|---|
| **遊玩時間上限** | 整台主機一個（主機本身的計時器） | 每個使用者帳號一個 |
| **PIN碼、提醒、暫停** | 主機本身的 | 它自己的 |
| **在背景執行** | 沒有（選用的復原 sysmodule 只在開機時、有 `RESCUE` 檔案要求時才會動作） | 一個 sysmodule，從開機起就在執行 |
| **韌體更新之後** | 主機的家長監護照常運作；PlayGuard 會以唯讀方式開啟，直到有版本支援該韌體 | 家長監護取決於 sysmodule 是否仍能運作 |
| **從 SD 卡刪除之後** | 上限仍留在主機上 | 家長監護也跟著消失 |

**每個孩子需要不同的上限？** 請選擇替代型 sysmodule——主機的計時器整台主機只有一個上限。否則，PlayGuard 會保留任天堂的家長監護，只是把它的設定帶到主機上。

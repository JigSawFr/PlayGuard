<!-- translated from docs/install.md, sha256 7d2be6a20dff -->
# 安装

如何安装、更新和删除 PlayGuard，以及它会在 SD 卡上写入哪些文件。安装之后：[快速上手](getting-started.md)。返回 [README](../../README.md)。

## 获取途径

> [!WARNING]
> PlayGuard **还没有上架 Homebrew App Store（也没有上架 sphaira 的商店）**：它的上架申请仍在等待审核。目前请手动安装。

任选其一：

- **手动**（目前推荐）— 从[最新版本](https://github.com/JigSawFr/PlayGuard/releases/latest)下载 `playguard.zip`，解压到 SD 卡的**根目录**。应用位于 `sd:/switch/playguard/`，它的文件位于 `sd:/config/playguard/`。
- **sphaira › GitHub** — 发布的 zip 已包含这个条目（`/config/sphaira/github/playguard.json`）：首次安装后，在 sphaira 的 *GitHub* 中更新。
- **Homebrew App Store** / **sphaira 的 App Store**（同一个目录）— **尚未上架。** 上架通过后，搜索 *PlayGuard*。之后新版本会自动获取，但要经过审核：通常比 GitHub 晚几天。PlayGuard 知道自己是从那里安装的，会推荐通过商店更新。

可选的**恢复 sysmodule**（`playguard-rescue.zip`）需要单独下载：同样把它解压到 SD 卡的根目录，然后重启主机。它位于 `atmosphere/contents/4200000000505247/`，只有在 `RESCUE` 文件要求时才会起作用——见[被锁在外面？](troubleshooting.md#被锁在外面)。

然后**在游戏之上**、用家长的用户打开 PlayGuard（[原因](compatibility.md#启动器)）。

## 更新

- **在 PlayGuard 中：** *偏好设置 › 更新方式* 选择更新的方式——在 PlayGuard 内（它会下载发布版本、校验、替换自身并重新启动）、sphaira、Homebrew App Store 或手动。开启 *每天检查更新* 后，有新版本时 PlayGuard 打开时会显示它。
- **恢复 sysmodule** 与 PlayGuard 的版本保持一致：每次更新后，重新解压该版本的 `playguard-rescue.zip`（或者在铃铛的 *恢复 sysmodule 不是最新版本* 上选择 *更新*），然后重启主机。
- 设置、历史记录、配置和备份都会保留：它们位于 `sd:/config/playguard/`，而不是应用旁边。

## 删除

删除 `sd:/switch/playguard/`（应用本身）；如果要连它的文件一起删除，再删除 `sd:/config/playguard/`；装了恢复 sysmodule 的话，还要删除 `sd:/atmosphere/contents/4200000000505247/`。时间限制、PIN 和各项限制都是主机自身的：它们**会留在主机上**。如果也要删除它们，请先使用 *安全 › 删除所有家长控制*。

## SD 卡上的文件

全部位于 `sd:/config/playguard/`（应用本身留在 `sd:/switch/playguard/`：删除或重新安装应用不会影响这些文件）。早期版本把它们放在应用旁边，这些版本首次启动时已把它们移到这里。

- **`config.json`** — 偏好设置（语言、主题、NTP 服务器、*用PIN保护PlayGuard* 等）；所有键见 [config.md](../config.md)（英文）。如果无法读取，它会被保留为 `config.json.bad`（之前的一份保留为 `.bad.1`），设置恢复为默认值；PlayGuard 会在启动时提示一次
- **`profiles/`** — 已保存的游戏时间限制配置
- **`exports/`** — 游玩记录的导出文件
- **`nand/<system>/`** — 属于某一个系统的文件，每个系统都有自己的家长控制：`sysmmc/` 对应 sysMMC，`emummc/` 对应 emuMMC。其中：`history.json` 是更改历史（最近 500 条，包括主机拒绝的更改；无法读取时会保留为 `history.json.bad`，之前的一份保留为 `.bad.1`，而不是被覆盖，PlayGuard 会提示一次）；`state.json` 记录 *阻止所有游戏*、加时、待重新启用的锁定和系统版本选择需要记住的内容（[config.md](../config.md#state-kept-between-launches)，英文）；`watch.json` 记录 PlayGuard 上次看到的主机状态，用来发现在它之外发生的更改（[config.md](../config.md#watchjson)，英文；删除它没有影响）；`backups/` 是设置备份（从不包含 PIN）；`cache/` 是最近读取的游玩记录（所有用户，以及查看过的每个用户），下次启动时立即显示，同时重新读取记录。这些文件夹出现之前的版本留下的文件，会归入 PlayGuard 首次启动的那个系统
- **`cache/`** — 游戏图标（`icons/`，最多 16 MB，主机语言更改时清空），所有系统共用，这样每次启动时不必重新读取每个游戏的名称和图标；Atmosphère、hekate、sys-patch 和系统版本的版本说明（`sys_notes.json`，用于 *主机*），以及 PlayGuard 自己的版本说明（用于 *关于*）；在开发者模式下，还有 *安装其他构建* 的列表（`dev_builds.json`）
- **`github_token`** — *在线发送报告* 以及开发者模式下 *安装其他构建* 所用的 GitHub 登录；退出登录即删除（*关于 › 帮助与诊断 › GitHub 账户*，登录后显示）
- **`previous_build.json`**、**`boot_check.json`** — 原地安装构建之后：被替换的构建名称（它本身作为 `playguard.nro.previous` 保留在 `.nro` 旁边），以及下次启动时的检查（第一个画面出现后删除）
- **`rescue_report.txt`** — 恢复 sysmodule 执行操作后留下，直到 PlayGuard 在启动时显示它（无法读取的会被删除，PlayGuard 会提示）
- **`rescue_nonce`** — 一个随机挑战值，恢复 sysmodule 会在报告中回应它，这样 PlayGuard 能区分它写的报告和手写的报告（[sysmodule/README.md](../../sysmodule/README.md)，英文）；每次启动都会生成新的
- **`logs/`** — `reports/` 是已保存的诊断报告（从不包含 PIN 或序列号）；`crashes/` 每次崩溃一个文件（是什么让 PlayGuard 停止）；`uploads/` 每份在线发送的报告一个网络快捷方式（`.url`）；`play_timer/` 是开发者工具的游戏时间记录文件。保留多少份报告、崩溃文件和链接：*关于 › 帮助与诊断*（按数量、时间或大小；最新的一份始终保留）

更多内容见 [packaging/README.md](../../packaging/README.md)（英文）。

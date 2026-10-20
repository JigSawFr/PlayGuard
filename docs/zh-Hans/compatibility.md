<!-- translated from docs/compatibility.md, sha256 33251c660faf -->
# 兼容性

支持的系统版本、Atmosphère 和启动器，emuMMC 和 sysMMC，以及 PlayGuard 与替代型 sysmodule 的比较。返回 [README](../../README.md)。

## 支持的版本

**已在主机上测试**——PlayGuard 在以下环境中运行过：

- **系统版本 23.0.1**——Atmosphère 1.12.1
- **系统版本 22.0.0、22.1.0、22.5.0**——Atmosphère 1.11.x
- hbmenu、sphaira 和 Homebrew App Store

**应该可用，未测试**——PlayGuard 中没有任何东西排除这些版本，但还没有人反馈过（[欢迎反馈](https://github.com/JigSawFr/PlayGuard/discussions/categories/console-reports)）。每个系统版本都需要一个能启动它的 Atmosphère：列出的那个，或任何更新的版本。

- **系统版本 23.0.0**——Atmosphère 1.12.0。在 [switchbrew](https://switchbrew.org/wiki/Parental_Control_services) 的表格中，命令与 23.0.1 相同。
- **系统版本 21.0.0 → 21.2.0**——Atmosphère 1.10.x。所有功能可用。

**21.0.0 之前，除游戏时间计时器外都可用**——限制、PIN、暂时解除、游玩记录、网络时钟、手机应用和历史记录都可用。游戏时间计时器的设置数据块（0x44 字节）从 21.0.0 起才是这种格式，所以 PlayGuard 会关闭存放在其中的功能：每日限制、就寝提醒、加时和阻止所有游戏。

- **系统版本 20.0.0 → 20.5.0**——Atmosphère 1.9.x
- **系统版本 19.0.x**——Atmosphère 1.8.0
- **系统版本 18.0.0 → 18.1.0**——Atmosphère 1.7.x
- **系统版本 17.0.x**——Atmosphère 1.6.x
- **系统版本 16.0.0 → 16.1.0**——Atmosphère 1.5.x
- **系统版本 15.0.x**——Atmosphère 1.4.x
- **系统版本 14.x**——Atmosphère 1.3.x
- **系统版本 13.x**——Atmosphère 1.1.0 → 1.2.6
- **系统版本 12.x 及更早**——Atmosphère 1.0.0 及更早

PlayGuard 检查的是 Atmosphère 是否在运行，而不是它的版本：重要的是它启动的系统版本。*主机* 会同时显示两者。

## 需要了解的事项

### 启动器

建议在游戏之上启动（title override）。应用会显示它是作为应用还是作为小程序（相册）运行。在游戏之上启动时，主机会把 PlayGuard 的时间算作那个游戏的时间。在游玩记录中，这些时间记在启动时选择的用户名下：请用家长的用户打开它，而不是孩子的。游戏时间计时器是主机的，对所有用户都一样：无论谁打开 PlayGuard，这段时间都会计入（除非计时器已关闭）。

### Nintendo Switch 2

不支持：它没有 Atmosphère。

### 更新的系统版本

系统版本比在主机上测试过的最新版本还新时，PlayGuard 会以**只读**方式打开，并检查是否有更新的发布版本支持它。如果有，它会提示更新（在 PlayGuard 内，或通过 sphaira 或 Homebrew App Store）。否则由你选择：只读、只读并带开发者工具（用于研究这个系统版本），或使用所有功能，风险自负。可以针对该系统版本和应用版本记住这个选择；*主机 › 兼容性* 会重新显示这个画面。

### emuMMC 和 sysMMC

两者各有各的家长控制，PlayGuard 只能看到它所运行的系统的设置。在 emuMMC 上，*主机* 和 *入门* 会说明这一点；在 sysMMC 上，如果 SD 卡上设置了 emuMMC，也会说明。请在两边都设置家长控制，或者隐藏 hekate 的启动菜单，让孩子无法选择另一个系统。PlayGuard 把每个系统的历史记录、备份和状态分开保存（sysMMC 和 emuMMC，在 `nand/` 中）；偏好设置和配置是共用的。

### 主机会被封禁吗？

PlayGuard 从不连接任天堂：联网时，它只访问公共时间服务器和 GitHub（更新，以及你发送的报告）。导致主机被封的是带着自制固件或 emuMMC 连接任天堂（[switchbrew FAQ](https://switchbrewdocs.readthedocs.io/en/latest/faq.html)）。目前没有因修改家长控制或时钟而被封的记录；但这不是保证。

### 不联网的主机

PlayGuard 正是为此而做的，而游戏时间计时器仍然需要一个设置过的网络时钟。没有时间服务器可以询问时，*主机 › 网络时钟 › 测量时间差* 会提示改为把它设为主机时钟：请先检查主机时钟，同时让 *通过互联网同步时钟* 保持关闭，之后再开启这个设置（见[网络时钟](features.md#没有服务器回应)）。

### 手机通知

从 22.0.0 起，仍然关联着手机应用的主机在输入 PIN 时，可能会通知家长（[switchbrew](https://switchbrew.org/wiki/22.0.0)）。PlayGuard 自己的解除是否会触发通知，目前还不清楚。

## 为什么做 PlayGuard

孩子们喜欢电子游戏，而电子游戏意味着屏幕：设定限制是照顾他们的一部分。在没有破解的 Switch 上，任天堂的手机应用负责这件事。主机运行自制固件之后，一个全新的世界打开了——家长控制却没了着落，因为手机应用再也连接不到主机。想知道孩子玩了多久、设定限制、让游戏停下来而不用争吵，也不再有一次又一次的“再玩五分钟”，都变得很难。

已有的少数工具都做不好：游玩报告几乎没人维护，也不够详细；家长控制功能粗糙，没有完成。我也不想要一个自己写的替代品，或一个一直在后台运行的 sysmodule。思路是**尽可能沿用主机自身的家长控制**，以及它已经做到的一切——时间限制、PIN、提醒、中断——再把它们的设置带回主机上。这就是 PlayGuard。

操作主机自身的家长控制还打开了更多可能：把游戏时间报告给家庭服务器、接受它的指令、Home Assistant、自动化。这些可能的发展方向写在[路线图的远景](../../ROADMAP.md#horizon)（英文）中。

## PlayGuard 还是替代型 sysmodule？

PlayGuard 离线操作主机内置的家长控制（`pctl` 服务）。另一种做法是用 sysmodule 取代它们，例如 [NS Parental Control](https://github.com/TristanIsrael/NSParentalControl)（一个 sysmodule 加一个 Tesla / Ultrahand 覆盖层）。

| | PlayGuard | 替代型 sysmodule |
|---|---|---|
| **游戏时间限制** | 整台主机一个（主机自身的计时器） | 每个用户一个 |
| **PIN、提醒、中断** | 主机的 | 它自己的 |
| **在后台** | 没有任何东西（可选的恢复 sysmodule 只在启动时、有 `RESCUE` 文件要求时才起作用） | 一个 sysmodule，从开机起运行 |
| **系统版本更新之后** | 主机的家长控制继续有效；PlayGuard 以只读方式打开，直到有版本支持该系统版本 | 家长控制取决于 sysmodule 是否仍然能用 |
| **从 SD 卡删除后** | 限制仍然留在主机上 | 家长控制随之消失 |

**每个孩子需要不同的限制**？请选择替代型 sysmodule——主机的计时器整台主机只有一个限制。否则，PlayGuard 保留任天堂的家长控制，只是把它们的设置带到主机上。

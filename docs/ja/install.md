<!-- translated from docs/install.md, sha256 7d2be6a20dff -->
# インストール

PlayGuard のインストール、アップデート、削除の方法と、PlayGuard が SD カードに書き込むファイルについて説明します。インストールしたら、[はじめに](getting-started.md)へ。トップページは [README](../../README.md)。

## 入手方法

> [!WARNING]
> PlayGuard は**まだ Homebrew App Store（sphaira の App Store も）にはありません**。掲載はまだ承認待ちです。今は手動でインストールしてください。

次のどれかを選んでください。

- **手動**（今はこれがおすすめ） — [最新リリース](https://github.com/JigSawFr/PlayGuard/releases/latest)から `playguard.zip` をダウンロードし、SD カードの**ルート**に展開します。アプリは `sd:/switch/playguard/` に、アプリのファイルは `sd:/config/playguard/` に入ります。
- **sphaira › GitHub** — リリースの zip にはエントリー（`/config/sphaira/github/playguard.json`）が入っています。一度インストールしたあとは、sphaira の *GitHub* からアップデートできます。
- **Homebrew App Store** / **sphaira の App Store**（同じカタログ） — **まだ掲載されていません。** 掲載が承認されたら、*PlayGuard* を検索してください。そのあと新しいリリースは審査のあとに自動で取り込まれます（多くは GitHub の数日後です）。PlayGuard はそこからインストールされたことを認識し、ストアでのアップデートをおすすめします。

オプションの**リカバリー sysmodule**（`playguard-rescue.zip`）は別にダウンロードします。これも SD カードのルートに展開し、本体を再起動してください。`atmosphere/contents/4200000000505247/` に入り、`RESCUE` ファイルで求められたときにだけ動きます。[ロックされて入れない？](troubleshooting.md#ロックされて入れない)を参照してください。

そのあと、PlayGuard を**ゲームの上から**、保護者のユーザーで開きます（[理由](compatibility.md#ランチャー)）。

## アップデート

- **PlayGuard で：** *オプション › アップデート方法* で方法を選びます。PlayGuard 内で（リリースをダウンロードして確認し、自分を置き換えて再起動します）、sphaira、Homebrew App Store、手動のどれかです。*毎日アップデートを確認* がオンなら、新しいリリースがあるとき、PlayGuard はそのリリースの画面で開きます。
- **リカバリー sysmodule** は PlayGuard のバージョンに合わせます。アップデートのたびに、そのリリースの `playguard-rescue.zip` を展開し直し（またはベルの *リカバリー sysmodule が最新ではありません* で *更新*）、本体を再起動してください。
- 設定、履歴、プリセット、バックアップはそのまま残ります。アプリの隣ではなく、`sd:/config/playguard/` にあるからです。

## 削除

`sd:/switch/playguard/`（アプリ）を削除します。アプリのファイルも消すなら `sd:/config/playguard/` も、リカバリー sysmodule を入れていれば `sd:/atmosphere/contents/4200000000505247/` も削除します。上限、暗証番号、制限は本体自身のものなので、**本体に残ります**。それらも消すには、先に *セキュリティ › みまもり設定をすべて削除* を使ってください。

## SD カード上のファイル

すべて `sd:/config/playguard/` にあります（アプリ自体は `sd:/switch/playguard/` にあるので、アプリを削除・再インストールしてもこれらのファイルは残ります）。これらのファイルをアプリと同じ場所に置いていた以前のバージョンでは、最初の起動時にここへ移動しました。

- **`config.json`** — オプション（言語、テーマ、NTP サーバー、*PlayGuardを暗証番号で保護* …）。すべてのキーは [config.md](../config.md)（英語）にあります。読み込めないファイルは `config.json.bad` として残され（その前のものは `.bad.1`）、設定は初期値に戻ります。PlayGuard は起動時に一度そのことを知らせます
- **`profiles/`** — 保存したプレイ時間のプリセット
- **`exports/`** — プレイ記録のエクスポート
- **`nand/<system>/`** — システムごとのデータです。システムはそれぞれ独自のみまもり設定を持っています。sysMMC は `sysmmc/`、emuMMC は `emummc/` です。中身は次のとおりです。`history.json`：変更履歴（新しいものから 500 件。本体が拒否した変更も含みます。読み込めないファイルは上書きせずに `history.json.bad`、その前のものは `.bad.1` として残し、PlayGuard が一度知らせます）。`state.json`：*すべてのソフトをブロック*、延長、保留中の再ロック、ファームウェアの選択のために覚えておく内容（[config.md](../config.md#state-kept-between-launches)、英語）。`watch.json`：PlayGuard が最後に見た本体の状態で、PlayGuard の外で変わったことを知らせるために使います（[config.md](../config.md#watchjson)、英語。削除しても問題ありません）。`backups/`：設定のバックアップ（暗証番号は含みません）。`cache/`：最後に読み込んだプレイ記録（すべてのユーザーと、表示した各ユーザー）。次の起動時、記録を読み直している間にすぐ表示されます。これらのフォルダーができる前のバージョンのファイルは、PlayGuard が最初に起動したシステムに移されます
- **`cache/`** — ゲームのアイコン（`icons/`、最大 16 MB、本体の言語が変わると空になります）。すべてのシステムで共有し、起動のたびに各ゲームから名前とアイコンを読み直さずに済むようにします。Atmosphère、hekate、sys-patch、ファームウェアのリリースノート（`sys_notes.json`、*本体* 用）と、PlayGuard 自身のリリースノート（*情報* 用）。開発者モードでは、*別のビルドをインストール* の一覧（`dev_builds.json`）
- **`github_token`** — *レポートをオンラインで送信* と、開発者モードでは *別のビルドをインストール* で使う GitHub のログイン情報です。ログアウトすると削除されます（*情報 › ヘルプと診断 › GitHub アカウント*、ログイン中に表示されます）
- **`previous_build.json`**、**`boot_check.json`** — ビルドをその場でインストールしたあとに使います。置き換えたビルドの名前（そのビルドは `.nro` の隣に `playguard.nro.previous` として残ります）と、次回の起動のチェック（最初の画面が表示されると削除されます）
- **`rescue_report.txt`** — リカバリー sysmodule が動作したあとに残すファイルです。PlayGuard が起動時に表示するまで残ります（PlayGuard が読み込めない場合は削除され、そのことを知らせます）
- **`rescue_nonce`** — リカバリー sysmodule がレポートの中で答えるランダムなチャレンジです。これで PlayGuard は、sysmodule が書いたレポートと手で書かれたレポートを見分けられます（[sysmodule/README.md](../../sysmodule/README.md)、英語）。起動のたびに新しくなります
- **`logs/`** — `reports/`：保存した診断レポート（暗証番号やシリアル番号は含みません）。`crashes/`：クラッシュごとに 1 ファイル（PlayGuard が止まった原因）。`uploads/`：オンラインで送信したレポートごとのインターネットショートカット（`.url`）。`play_timer/`：開発者ツールのプレイ時間記録ファイル。レポート、クラッシュファイル、リンクをいくつ残すかは *情報 › ヘルプと診断* で設定します（件数、日数、容量で指定。最新のものは必ず残ります）

詳しくは [packaging/README.md](../../packaging/README.md)（英語）をご覧ください。

Chronosの開発の手引き
=====================

 * 開発には Visual Studio 2022が必要です。
   * `Desktop Development with C++ > C++ CMake tools for Windows` を選択
   * `Desktop Development with C++ > C++ MFC for latest ...` を選択
 * Windows SDK 10.0.26100.0の事前インストールが必要です。
   * Visual Studio Installerからインストール可能です。

## ビルド手順

 1. このレポジトリをローカルにクローンする。
 2. スタートメニューから開発者コマンドプロンプト（Developer Command Prompt for VS 2022）を開き`setup-cef.bat`を実行する。
    * この段階で以下のディレクトリが作成される。
      - `D32`: デバッグ版の実行ファイル一式（CEFから抽出＋Chronos固有のビルド済みバイナリが置かれる）
      - `R32`: リリース版の実行ファイル一式（CEFから抽出＋Chronos固有のビルド済みバイナリが置かれる）
      - `lib`: デバッグ版のコンパイルに必要なライブラリ（CEFから抽出）
      - `rlib`: リリース版のコンパイルに必要なライブラリ（CEFから抽出）
      - `include`: CEFのヘッダファイル（CEFから抽出）
 3. Visual Studio 2022でプロジェクト（Sazabi.sln）を開く。
 4. 構成を「R64_CSG」（リリース版）または「D64_CSG」（デバッグ版）から選択する。
 5. メニューから「Local Windows Debugger」を実行する。

## デバッグ実行について

Chronosのビルド成果物は`ChronosN.dll`であり、実行ファイルはCEFに同梱されている
bootstrap実行ファイル（`setup-cef.bat`が`ChronosN.exe`という名前で`D32`/`R32`に配置する）である。
詳しい経緯は[SANDBOX.md](doc/SANDBOX.md)を参照。

このため、プロジェクトのデバッグ設定では起動対象として`$(OutDir)ChronosN.exe`を指定している。
「Local Windows Debugger」を実行すればbootstrap経由で`ChronosN.dll`が読み込まれ、
ブラウザープロセスにはそのままデバッガーがアタッチされる。

なお、`Sazabi.vcxproj.user`は`.gitignore`の対象であり、Visual Studio上でデバッグ設定を
変更すると同ファイルに書き出されてプロジェクトの設定を上書きする。
「Local Windows Debugger」が起動しなくなった場合は、同ファイルを削除して確認するとよい。

### サブプロセス（レンダラーなど）のデバッグ

「Local Windows Debugger」でアタッチされるのはブラウザープロセスのみである。
レンダラーやGPUプロセスを追う場合は、次のいずれかを使う。

* コマンド引数に`--renderer-startup-dialog`を指定する。
  レンダラープロセスが起動時にダイアログを表示して停止するので、
  「デバッグ > プロセスにアタッチ」でそのプロセスにアタッチしてからダイアログを閉じる。
  GPUプロセス用の`--gpu-startup-dialog`、ユーティリティプロセス用の
  `--utility-startup-dialog`も同様。
* Visual Studio Marketplaceの「Child Process Debugging Power Tool」を導入すると、
  子プロセスにも自動的にアタッチされる。
* 事象の切り分けだけであれば、`--single-process`で全処理を1プロセスに寄せる方法もある
  （本来の構成とは異なるため、再現しない不具合もある点に注意）。

## Chromiumのバージョンの更新手順

 1. `setup-cef.bat`に記載されている`CEFVER`を更新する。
    * https://cef-builds.spotifycdn.com/index.html#windows32 に掲載されている「Windows 32」且つ「minimal」のものを使用する。
 2. スタートメニューから開発者コマンドプロンプト（Developer Command Prompt for VS 2022）を開き`setup-cef.bat`を再実行する。
 3. Chronosのバージョンを上げる。
    * CEF（Chromium）のメジャーバージョンをChronosのバージョンの3桁目に反映する。
      たとえば、Chromium 96ベースのCEFであれば、Chronosのバージョンは x.x.96.x となる。
    * Visual Studio 2022でプロジェクト（Sazabi.sln）を開き、リソースビューの「Sazabi」から
      「Sazabi.rc」→「Version」→「VS_VERSION_INFO」と「VS_VERSION_INFO[英語]」のそれぞれについて
      「FILEVERSION」と「PRODUCTVERSION」の箇所を変更して保存する。
      （それ以外の箇所は自動的に追従するため、特に変更の必要はない。）
 4. Visual Studio上でChronosをビルドする。

## Chromiumのバージョンを確認する方法

 1. Chronosを起動し、ツールバーからヘルプ > システム情報を選択する。
 2. ダイアログの「CEF Version Information」を選択する。
 3. CEFの欄のバージョンを確認する。

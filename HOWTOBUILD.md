Chronosの開発の手引き
=====================

 * 開発には Visual Studio 2019が必要です。
   * `Desktop Development with C++ > C++ CMake tools for Windows` を選択
   * `Desktop Development with C++ > C++ MFC for latest ...` を選択

## ビルド手順

 1. このレポジトリをローカルにクローンする。
 2. スタートメニューから開発者コマンドプロンプト（Developer Command Prompt for VS 2019）を開き`setup-cef.bat`を実行する。
    * この段階で以下のディレクトリが作成される。
      - `D32`: デバッグ版の実行ファイル一式（CEFから抽出＋Chronos固有のビルド済みバイナリが置かれる）
      - `R32`: リリース版の実行ファイル一式（CEFから抽出＋Chronos固有のビルド済みバイナリが置かれる）
      - `lib`: デバッグ版のコンパイルに必要なライブラリ（CEFから抽出）
      - `rlib`: リリース版のコンパイルに必要なライブラリ（CEFから抽出）
      - `include`: CEFのヘッダファイル（CEFから抽出）
 3. Visual Studio 2019でプロジェクト（Sazabi.sln）を開く。
 4. 構成を「R64_CSG」（リリース版）または「D64_CSG」（デバッグ版）から選択する。
 5. メニューから「Local Windows Debugger」を実行する。

## Chromiumのバージョンの更新手順

 1. `setup-cef.bat`に記載されている`CEFVER`を更新する。
    * https://cef-builds.spotifycdn.com/index.html#windows32 に掲載されている「Windows 32」且つ「minimal」のものを使用する。
 2. スタートメニューから開発者コマンドプロンプト（Developer Command Prompt for VS 2019）を開き`setup-cef.bat`を再実行する。
 3. Chronosのバージョンを上げる。
    * CEF（Chromium）のメジャーバージョンをChronosのバージョンの3桁目に反映する。
      たとえば、Chromium 96ベースのCEFであれば、Chronosのバージョンは x.x.96.x となる。
 4. sbcommon.hのユーザーエージェント文字列を更新する。
    * `sgSZB_UA_END` に含まれるバージョンを、CEFのバージョンに合わせる。
 5. Visual Studio上でChronosをビルドする。

## Chromiumのバージョンを確認する方法

 1. Chronosを起動し、ツールバーからヘルプ > バージョン情報を選択する。
 2. ダイアログの「CEF Version Information」を選択する。
 3. CEFの欄のバージョンを確認する。

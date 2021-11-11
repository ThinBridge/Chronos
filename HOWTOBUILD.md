Chronosの開発の手引き
=====================

 * 開発には Visual Studio 2019が必要です。
   * `Desktop Development with C++ > C++ CMake tools for Windows` を選択
   * `Desktop Development with C++ > C++ MFC for latest ...` を選択

## ビルド手順

 1. このレポジトリをローカルにクローンする。
 2. スタートメニューから開発者コマンドプロンプトを開き`setup-cef.bat`を実行する。
 3. Visual Studio 2019でプロジェクトを開く。
 4. メニューから「Local Windows Debugger」を実行する。

## Chromiumのバージョンの更新手順

 1. `setup-cef.bat`に記載されている`CEFVER`を更新する。
 2. `setup-cef.bat`を再実行する。
 3. Visual Studio上でChronosをビルドする。

## Chromiumのバージョンを確認する方法

 1. Chronosを起動し、ツールバーからヘルプ > バージョン情報を選択する。
 2. ダイアログの「CEF Version Information」を選択する。
 3. CEFの欄のバージョンを確認する。

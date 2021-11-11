Chronosの開発の手引き
=====================

 * 開発には Visual Studio 2019 (ATL/MFC含) が必要です。
 * バイナリファイルの管理に[Git LFS](https://git-lfs.github.com/)を利用しています。
   * Windows環境の場合は"Git for Windows"に標準で含まれています。
   * Ubuntu環境の場合は`sudo apt install git-lfs`でインストールください。

## ビルド手順

 1. このレポジトリをローカルにクローンする。
 2. 同梱されている`setup-cef.bat`を実行し、ビルド環境をセットアップする。
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

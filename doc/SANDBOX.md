# レンダラープロセスのサンドボックス化に関する調査

## 目的

現在のChronosはCEFのサンドボックスを無効 (`CefSettings.no_sandbox = true`) にして動作している。
Web由来の攻撃(レンダラープロセスの侵害)に対する多層防御として、レンダラーを含む
サブプロセスのサンドボックス化を検討する。本書はその実現方式・影響範囲・リスク
(特にMinHookとの干渉)を整理したものである。

## 現状の構成(2026-07時点 / CEF 146)

* 単一EXE構成: `ChronosN.exe` がブラウザプロセスとすべてのサブプロセス
  (renderer / gpu / utility / network など)を兼ねる。
  * サブプロセスは `AfxWinMain`(Sazabi.cpp)冒頭の `--type=` 判定で
    `CefExecuteProcess()` を呼び、MFCの初期化に入る前に終了する。
  * `sandbox_info` は常に `NULL`、`CefSettings.no_sandbox = true`(Sazabi.cpp)。
* ビルド: Win32 (x86) / MFC / スタティックCRT (`/MT`) / サブシステム Windows。
  構成は `D64_CSG` / `R64_CSG`。
* CEFバイナリ: `cef_binary_146.0.10+...+chromium-146.0.7680.179_windows32_minimal`
  を `setup-cef.bat`(`-DUSE_SANDBOX=Off`)で取得・ビルド。
* 配布: ThinApp化された `Chronos.exe` から `ChronosN.exe` を起動する形態がある
  (`CSazabi::InitFunc_ExecOnVOS`)。ネイティブ起動も可。
* MinHookによるAPIフック(`APIHook.cpp`)をブラウザプロセスで使用(後述)。

## CEF 146 でサンドボックスを有効化するための要件

CEF M138以降、Windowsのサンドボックス有効化方式が大きく変わった。
従来の `cef_sandbox.lib` の静的リンクは廃止され(Chromium同梱のClang/LLVM/libc++
ツールチェーンへの依存が避けられなくなったため)、**bootstrap実行ファイル方式**が
唯一の手段になっている。

参考: https://chromiumembedded.github.io/cef/sandbox_setup
(`include/cef_sandbox_win.h` のコメントからも参照されている)

要件の要点:

1. **クライアントをDLLとしてビルドする。** EXEはCEF配布物に含まれる
   `bootstrap.exe`(GUI用)/ `bootstrapc.exe`(コンソール用)を使う。
   * CEF 146 の windows32 minimal 配布物に `bootstrap.exe` / `bootstrapc.exe` が
     同梱されていることを確認済み(`cef-cache/.../Release/`)。
2. **`bootstrap.exe` はリネームして利用できる。** 例えば `ChronosN.exe` に
   リネームすると、同じディレクトリの `ChronosN.dll` を自動でロードする
   (`--module=<name>` での明示指定も可)。
   * よって「EXE名が `ChronosN.exe` であること」に依存した既存ロジック
     (`InitFunc_ExecOnVOS` のEXE名判定など)は原理的には維持できる。
3. クライアントDLLは **`RunWinMain` をエクスポート**する
   (`include/cef_sandbox_win.h` 参照)。シグネチャ:
   ```cpp
   int RunWinMain(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow,
                  void* sandbox_info, cef_version_info_t* version_info);
   ```
   bootstrapがサンドボックス情報を生成して `sandbox_info` として渡してくるので、
   これを従来どおり `CefExecuteProcess()` と `CefInitialize()` に渡す。
   `version_info` によりbootstrapとlibcef/クライアントのバージョン整合が
   検証されるため、**bootstrap.exe はCEFバージョン更新のたびに差し替える**必要がある。
4. スタティックCRT (`/MT`) 縛りは撤廃された(旧 `cef_sandbox.lib` 時代の制約)。
   現行の `/MT` のままでも問題はない。
5. 配布時はリネーム・リソース差し替え後の**全バイナリにコード署名**を行うこと
   (公式ドキュメントの指示)。bootstrapのアイコン・バージョンリソースは
   Resource Hacker等で差し替え可能。

つまり「レンダラーのサンドボックス化」は設定変更ではなく、
**ChronosN.exe を「bootstrap.exe(リネーム)+ ChronosN.dll」に分割する
アーキテクチャ変更**を伴う。

## MinHookとの干渉分析(本調査の主眼)

### 現在のフック内容と導入プロセス

`APIHookC::DoHookComDlgAPI()`(APIHook.cpp)で以下をフックしている:

| モジュール | API | 目的 |
|---|---|---|
| comdlg32.dll | `GetSaveFileNameW` / `GetOpenFileNameW` | ダウンロード/アップロード先の制限(SGモードのB:ドライブ制限等) |
| shell32.dll | `SHBrowseForFolderW` | フォルダー参照の禁止 |
| ole32.dll | `CoCreateInstance` | `IFileOpenDialog` / `IFileSaveDialog` をラップして保存/選択先を制限 |
| gdi32.dll | `CreateDCW` / `DeleteDC` / `StartDocW` | 「Microsoft Print to PDF」の使用ブロック(ThinApp環境でのクラッシュ回避) |

導入箇所は `CSazabi::InitInstance()`(Sazabi.cpp)であり、サブプロセスは
`AfxWinMain` 冒頭の `--type=` 分岐で `InitInstance` に到達する前に終了する。
したがって **MinHookのフックは現状ブラウザプロセスにのみ存在し、
レンダラー等のサブプロセスには一切入っていない**。

### 結論: 直接の干渉はない(ただし検証必須の間接影響あり)

サンドボックスが制限するのはサブプロセス(renderer / gpu / utility / network)で
あり、ブラウザプロセス自体はサンドボックス化されない。フックがブラウザプロセス
のみに存在する現構成では、**サンドボックス有効化とMinHookは原理上直接干渉しない**。
bootstrap方式に移行してもフック導入はDLL内の `InitInstance`(= `CefInitialize` 前)
のままで、タイミングも変わらない。

ただし、以下の間接的な影響・検証項目がある。

#### (1) フック対象APIが「ブラウザプロセスで呼ばれる」前提の維持 【重要】

上記フックはすべて「ファイルダイアログや印刷がブラウザプロセス内で実行される」
ことを前提にしている。Chromiumは近年、こうした処理をutilityプロセスへ分離する
方向にある(例: OOP Print Backend / `EnableOopPrintDrivers`、Chrome本体の
ファイルダイアログのutilityプロセス化)。

* サンドボックスを有効化すると、これらのutilityプロセスもサンドボックスポリシー
  の適用対象になり、挙動が変わる可能性がある。
* フックの入っていないプロセスでダイアログ・印刷が実行されるようになると、
  **B:ドライブ制限、アップロード/ダウンロード制限、Print to PDFブロックが
  静かに機能しなくなる**(フックが呼ばれないだけでエラーにはならない)。
* また、仮に将来これらのプロセスにフックを入れたくなっても、フック関数は
  `theApp`(MFC)やUI(`MessageBoxW`)に依存しているため、そのままでは動かない。

→ サンドボックス有効ビルドで、SGモードの保存先制限・アップロード制限・
Print to PDFブロックの全シナリオを回帰確認することが必須。

#### (2) サンドボックス化プロセス内ではMinHookは使用不能(将来の制約)

サンドボックス化されたレンダラー等では ACG(Arbitrary Code Guard /
動的コード生成禁止)等のプロセス緩和策が有効になるため、MinHookのトランポリン
生成(実行可能メモリの確保・書き換え)は失敗する。現状レンダラーにフックは
ないので問題にならないが、**「レンダラー側にもフックを入れる」という将来の
選択肢は失われる**点は設計上意識しておく。

#### (3) bootstrap / Chromium自身のフック機構との共存

* Chromium自身も `chrome_elf.dll` 等でAPIフックを行うが、これは現行構成でも
  共存できており、bootstrap化で新たに悪化する要素はない。
* bootstrap.exe(Chromiumツールチェーンでビルド)はCFG/CETなどの緩和策付きで
  ビルドされている可能性がある。MinHookはret改変を行わず直接jmpによる
  トランポリンなので、CET シャドウスタック/CFGとは原理上互換だが、
  **bootstrap配下でのフック動作確認**はPhase 1の検証項目に含めること。
* ブラウザプロセスにはCIG(コード整合性ガード=非MS署名DLLのロード禁止)は
  適用されない(bootstrap自体が非MS署名のクライアントDLLをロードする方式の
  ため)。MinHook・MFC DLLのロードに支障はない。

## ThinApp(VOS)との関係 【最大のリスク】

* 配布形態として ThinApp 化された `Chronos.exe` → `ChronosN.exe` の起動チェーンが
  ある(`InitFunc_ExecOnVOS`)。ThinApp自体がAPIフックとファイル/レジストリ
  仮想化で成立している技術であり、Chromiumサンドボックス(lockdownトークン、
  ジョブオブジェクト、win32kロックダウン等)配下の子プロセスでThinAppランタイム
  が正常に初期化できるかは不明。
* コード中にも ThinApp × Chromium の相性問題の実績がある:
  * `Hook_StartDocW` のコメント: ThinApp化されたChronosでは
    Microsoft Print to PDF実行時にクラッシュするため、フックでブロックしている。
  * `InitFunc_ExecOnVOS` のコメント: サンドボックス使用に起因するThinApp配下での
    アプリケーションエラーへの言及。
* **サンドボックス有効化はネイティブ配布では成立しても、ThinApp配布では
  成立しない可能性がそれなりに高い。** ThinApp環境での検証を早期に行い、
  不成立の場合は「ネイティブ配布のみサンドボックス有効」などの分岐が必要か
  判断する。

## 必要な変更一覧(実装ステップ)

Phase 1(DLL化・bootstrap起動、サンドボックスは無効のまま):

1. `Sazabi.vcxproj`
   * `ConfigurationType`: `Application` → `DynamicLibrary`(`ChronosN.dll` を出力)
   * MFCはスタティックリンクのまま(regular MFC DLLとして動作させる)
2. エントリポイントの追加(新規ファイル例: `BootstrapMain.cpp`)
   ```cpp
   #include "include/cef_sandbox_win.h"

   extern int MY_WinMain(HINSTANCE, HINSTANCE, LPTSTR, int); // 既存(Sazabi.cpp)

   int RunWinMain(HINSTANCE hInstance, LPTSTR lpCmdLine, int nCmdShow,
                  void* sandbox_info, cef_version_info_t* version_info)
   {
       // 既存の AfxWinMain 相当の処理へ委譲する。
       // - --type= サブプロセス分岐(CefExecuteProcess に sandbox_info を渡す)
       // - SetDllDirectoryW / SetSearchPathMode / setlocale
       // - MY_WinMain(AfxWinInit → InitInstance → Run)
       // sandbox_info はグローバル(theApp等)に保持して
       // InitializeCef() の CefInitialize にも渡す。
   }
   ```
   * 注意: `hInstance` はbootstrap(EXE)のインスタンスハンドル。
     MFCのリソース(Sazabi.rc)はDLL側に入るため、`AfxWinInit` 後に
     `AfxSetResourceHandle(DLLのHMODULE)` が必要になる見込み。
     `theApp.m_hInstance` / `CefMainArgs(hInstance)` にどちらを渡すかの整理が
     Phase 1の主な作業。
3. `Sazabi.cpp`
   * `AfxWinMain` のオーバーライドを `RunWinMain` からの委譲に置き換え
   * `settings.no_sandbox = false`、`CefInitialize` / `CefExecuteProcess` に
     `sandbox_info` を渡す(Phase 1では `no_sandbox = true` のまま起動確認)
4. `setup-cef.bat` / パッケージング
   * `bootstrap.exe` を `ChronosN.exe` にリネームして配置する手順を追加
     (D32/R32にはRelease丸ごとコピーで既に入っている)
   * CEFバージョン更新時にbootstrapも必ず差し替わることを担保
   * bootstrapのバージョンリソース・アイコンの差し替え、全バイナリの再署名
5. EXE名・パス前提コードの点検
   * `CheckChronosVersionMismatch`、`InitFunc_ExecOnVOS`、
     クラッシュリカバリ、`CopyDBLEXEToTempInit` 等、
     「自分自身のEXE」を前提にしたコードの洗い出し

Phase 2(サンドボックス有効化・ネイティブ環境):

* `no_sandbox = false` + `sandbox_info` 有効化
* 後述チェックリストの回帰確認

Phase 3(ThinApp環境):

* ThinApp化パッケージでの起動・全機能検証
* 不成立の場合の方針判断(ThinApp配布のみ `no_sandbox` 維持等)

## 検証チェックリスト(Phase 2以降)

* [ ] SGモード: アップロードダイアログのB:/Upload制限(`IFileOpenDialog` 経由・
      `GetOpenFileNameW` 経由の両方)
* [ ] SGモード: 保存ダイアログのB:ドライブ制限、Uploadフォルダーへの保存禁止
* [ ] ネイティブモード: ダウンロード/アップロードフォルダー制限、拡張子変更制限
* [ ] アップロード/ダウンロード禁止設定(`IsEnableUploadRestriction` 等)
* [ ] `SHBrowseForFolderW` ブロック
* [ ] 印刷: 「PDFに保存」動作、「Microsoft Print to PDF」のブロックが機能すること
      (OOP印刷への移行有無を `chrome://print` 系の挙動やプロセス構成で確認)
* [ ] アップロードログ(`SendLoggingMsg(LOG_UPLOAD, ...)`)が記録されること
* [ ] GPU有効/無効(`disable-gpu` 設定)双方での描画
* [ ] プロキシ設定、認証ダイアログ、ポップアップ、DevTools、ダウンロード継続
* [ ] クラッシュ時のリカバリ動線、二重起動チェック
* [ ] タスクマネージャーで各サブプロセスのサンドボックス状態確認
      (`chrome://sandbox` 相当の情報が取れないため、Process Explorerで
      Integrity Level / Job / トークンを確認)

## まとめ

| 論点 | 評価 |
|---|---|
| MinHookとの直接干渉 | なし(フックはブラウザプロセスのみ、サンドボックスはサブプロセスのみに適用) |
| MinHookへの間接影響 | ダイアログ・印刷処理のプロセス配置変化により制限機能が無効化されるリスク → 要回帰検証 |
| 実装コスト | 大。設定変更ではなくEXE→DLL+bootstrap方式への構造変更(MFCリソース処理含む) |
| 最大のリスク | ThinApp(VOS)配布形態との相性。早期の実機検証が必要 |
| CRT/ビルド制約 | 旧方式の `/MT` 縛りは撤廃済みで現行設定のまま移行可能 |

推奨: Phase 1(bootstrap化のみ・サンドボックス無効)を独立したPRとして先行させ、
挙動が変わらないことを確認したうえで、Phase 2でサンドボックスを有効化するのが
安全である。

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

Phase 1(DLL化・bootstrap起動、サンドボックスは無効のまま): **実装済み**

このブランチで実装・ビルド・起動確認まで完了している。詳細は後述の
「Phase 1 実装メモ」を参照。

> **現在のビルド設定について**
>
> `Sazabi.vcxproj` は `CHRONOS_ENABLE_SANDBOX` を定義済みであり、
> **サンドボックスは有効になっている**(Phase 2 の状態)。
> ただし後述のとおりレンダラープロセスが起動しない問題は未解決のため、
> このままではページが表示されない。
> Phase 1 の状態(サンドボックス無効)で動かしたい場合は、
> `Sazabi.vcxproj` の `PreprocessorDefinitions` から
> `CHRONOS_ENABLE_SANDBOX` を外してビルドする。

1. `Sazabi.vcxproj`
   * `ConfigurationType`: `Application` → `DynamicLibrary`(`ChronosN.dll` を出力)
   * MFCはスタティックリンクのまま
   * `CHRONOS_USE_BOOTSTRAP` を定義
   * `TerminalServerAware` を削除(DLLでは無視され LNK4075 になるため。
     CIは `/warnaserror` でビルドしている)
2. エントリポイントの追加(`BootstrapMain.cpp`)
   * `RunWinMain` をエクスポートし、`ChronosRunMain()` へ委譲
   * `DllMain` を自前で定義(MFCの `dllmodul.obj` をリンクさせないため)
3. `Sazabi.cpp`
   * `AfxWinMain` の本体を `ChronosRunMain(lpCmdLine, nCmdShow)` に切り出し
   * `CSazabi::GetSandboxInfo()` を追加。`CHRONOS_ENABLE_SANDBOX` が未定義の場合は
     bootstrapから渡された `sandbox_info` を無視して `NULL` を返す
     (サンドボックス無効。Phase 1 の切り分け用に残してある)
   * `settings.no_sandbox = (sandbox_info == NULL)`
4. `setup-cef.bat`
   * `bootstrap.exe` を `D32\ChronosN.exe` / `R32\ChronosN.exe` として配置
   * CEFバージョン更新時にbootstrapも必ず差し替わる(setup-cef.bat経由のため)
5. 未対応(Phase 1の残作業)
   * bootstrapのバージョンリソース・アイコンの差し替え、全バイナリの再署名
   * CI(`.github/workflows/build.yaml`)の成果物の扱い。
     現在は `ChronosN.exe` を `Chronos.exe` にコピーしているが、
     bootstrapとDLLの両方を扱うよう見直しが必要
   * EXE名・パス前提コードの点検(`CheckChronosVersionMismatch`、
     `InitFunc_ExecOnVOS`、クラッシュリカバリ、`CopyDBLEXEToTempInit` など)

Phase 2(サンドボックス有効化・ネイティブ環境): **着手中**

* `CHRONOS_ENABLE_SANDBOX` は定義済み(サンドボックス有効)
* **未解決**: storage サービスのプロセスがクラッシュを繰り返し、
  ページが表示されない(「Phase 2 の予備調査」を参照)
* 上記を解決したうえで、後述チェックリストの回帰確認

Phase 3(ThinApp環境):

* ThinApp化パッケージでの起動・全機能検証
* 不成立の場合の方針判断(ThinApp配布のみ `no_sandbox` 維持等)

## Phase 1 実装メモ

### ビルド・起動確認の結果

`msbuild /p:Platform=Win32 /p:Configuration=R64_CSG`(CIと同じ
`/p:RunCodeAnalysis=true` 付き)でエラー・警告なくビルドでき、
`R32\ChronosN.exe`(bootstrap)から起動してブラウザーウィンドウの表示、
ページの描画までを確認した。

エクスポートも期待どおり(`dumpbin /exports ChronosN.dll`):

```
ordinal hint RVA      name
      1    0 000091A0 RunWinMain = _RunWinMain
```

起動時の引数についても、引数なし・Chronosが解釈しないスイッチ・URL・
`--renderer-startup-dialog` のいずれでも正常に動作することを確認した
(URLを渡した場合はそのページに遷移する)。

プロセス構成もサブプロセスが正しく起動している。レンダラーには
`--no-sandbox` が付与されており、Phase 1の意図どおりサンドボックスは無効:

```
20772   browser
52644   gpu-process
64684   utility
56228   utility
21604   renderer     --type=renderer --no-sandbox
58172   renderer
```

### つまずいた点: MFCが「DLLコンテキスト」として動作してしまう 【重要】

素直にDLL化しただけでは、起動直後に `CSazabi::InitInstance()` 内で
アクセス違反(0xC0000005)によりクラッシュした。原因は次のとおり。

* MFCは `AFX_MODULE_STATE::m_bDLL`(マクロ `afxContextIsDLL`)で
  「自分がEXEなのかDLLなのか」を判断し、20箇所以上で挙動を分岐している。
* このフラグを `FALSE` にしているのはMFCの `appmodul.cpp` にある
  `_afxInitAppState` という静的初期化子(`AfxInitialize(FALSE, _MFC_VER)` を
  呼ぶ)だが、`appmodul.obj` は `wWinMain` が参照されたときにしかリンクされない。
  DLLビルドではリンクされないため、`m_bDLL` は `TRUE` のままになる。
* その結果、
  * `AfxWinInit()` が `AfxInitThread()` を呼ばない(appinit.cpp)
  * `AfxOleInit()` が `COleMessageFilter` を生成せずに `TRUE` を返す(oleinit.cpp)
  * `CWinApp::ExitInstance()` が `SaveStdProfileSettings()` を呼ばない
  といった、アプリケーションとして動かすには不都合な分岐に入る。
* クラッシュの直接原因は `CSazabi::InitFunc_Base()` の
  `AfxOleGetMessageFilter()->EnableNotRespondingDialog(FALSE);` で、
  生成されなかった `COleMessageFilter`(NULL)を参照したためだった。

対策として、`RunWinMain` の先頭で `AfxWinInit()` より前に
MFCの公開API `AfxInitialize(FALSE, _MFC_VER)` を呼び、EXEコンテキストである
ことを明示している。これは `appmodul.cpp` がEXEビルドで行っている処理と同じ。

副作用として、EXEビルドで併せて登録される `_AfxTermAppState()` の
`atexit` 登録(MFCのスレッドローカルデータ解放)は行われない。プロセス終了時に
OSが回収するため実害はないが、デバッグビルドのリーク検出には出る可能性がある。

### つまずいた点: `__argc` / `__wargv` がDLLでは初期化されない 【重要】

引数を1つでも付けて起動すると `CSazabi::InitParseCommandLine()` で
アクセス違反(0xC0000005)によりクラッシュした。

`__argc` / `__wargv` はCRTのスタートアップコードが設定するが、これを行うのは
**EXEのスタートアップだけ**である。DLLの `_DllMainCRTStartup` はコマンドライン
引数の解析を行わないため、bootstrap構成ではこれらが未初期化のまま参照されていた。

引数なしで起動したときは `__argc` の値によってループが1度も回らず、
たまたま問題が表面化しなかった。そのため発見が遅れやすい。

対策として、`::CommandLineToArgvW(::GetCommandLineW(), &argCount)` で
プロセスのコマンドラインを直接解析するように変更した。EXEビルドでも
まったく同じ結果になるため、構成による分岐は不要である。

これは「既定のブラウザーとして他アプリからURL付きで起動される」という
実運用の経路に直撃する不具合だったため、同種の
**「EXEのCRTスタートアップに依存しているコード」**が他にないかは
今後も注意が必要である(現時点でリポジトリ内の `__argc` / `__wargv` /
`_pgmptr` の使用箇所はこの1箇所のみで、他にはない)。

### HINSTANCE の扱い

bootstrap構成では「モジュール」と「プロセス」のHINSTANCEが別物になるため、
用途ごとに使い分けている(`Sazabi.h` の `g_hChronos*Instance`)。

| 用途 | 使うハンドル |
|---|---|
| `AfxWinInit()`、ウィンドウクラス登録、リソース読み込み | `ChronosN.dll` のHMODULE |
| `CefMainArgs` | `ChronosN.exe`(bootstrap)のHINSTANCE |

`Sazabi.rc` はDLL側に入るため、リソースハンドルはDLLでなければならない。
`CSazabi::SetThisAppVersionString()` が `AfxGetInstanceHandle()` から
バージョン情報を読むため、バージョンリソースもDLL側のものが使われる点に注意
(bootstrap.exe側のバージョンはCEFのものになる)。

### Phase 2 の予備調査(サンドボックス有効での試行)

`CHRONOS_ENABLE_SANDBOX` を定義したビルドでも、ブラウザープロセスの起動と
ウィンドウの表示まではできた。サンドボックス自体も有効になっており、
`--no-sandbox` が付与されなくなっていることを確認した。

しかし**ページの内容がまったく描画されない**(白画面)。
画面には次のエラーが表示される。

```
このウェブページの表示中に問題が発生しました。
エラーコード: STATUS_BREAKPOINT
```

#### 原因の切り分け結果

プロセスを高頻度でサンプリングして種別を追ったところ、
**`storage.mojom.StorageService` の utility プロセスが約2秒周期で
クラッシュと再起動を繰り返している**ことが判明した。

```
61888   sub=network.mojom.NetworkService   sandbox=none
57636   sub=storage.mojom.StorageService   sandbox=service
12728   sub=storage.mojom.StorageService   sandbox=service
18524   sub=storage.mojom.StorageService   sandbox=service
   ... 以降ずっと繰り返し ...
```

例外コードは `0xE06D7363`(未処理のC++例外)で、発生モジュールは
`KERNELBASE.dll`(=`RaiseException` の位置なので、投げた場所は分からない)。
画面の `STATUS_BREAKPOINT` はこの状態に対するChromium側の表示である。

storage サービスが起動できないためページの読み込みが成立せず、白画面になる。
レンダラープロセス自体は起動している(この点は当初「レンダラーが起動しない」と
記録していたが誤りだった。サンドボックス化されたプロセスは外部から
コマンドラインを読めないため、プロセス一覧だけでは種別を判別できない)。

CEFのログ(`ChronosDefault.conf` / `Chronos.conf` の `EnableAdvancedLogMode=1`、
`EnableAdvancedLogVerboseMode=1` で `<cache>\CEFDebug.log` に出力される)には
クラッシュしたプロセスの行が1行も残らない。Chromiumのログ初期化に到達する前に
死んでいることを意味する。

#### 試したが効果がなかったこと

`ChronosRunMain()` の前段処理(`SetDllDirectoryW` / `SetSearchPathMode` /
`setlocale`)より前に `--type=` 判定と `CefExecuteProcess()` を移動しても
症状は変わらなかった。CEFの推奨に沿った変更なのでそのまま残してある
(同時に、レンダラーで `CefExecuteProcess()` が二重に呼ばれ得る問題も修正した)。

これにより、原因は **`RunWinMain()` に到達する前** 、すなわち
**ChronosN.dll のロード時に走る静的初期化**(MFCの初期化と `theApp` を含む
全グローバルオブジェクトのコンストラクタ)にある可能性が高くなった。
bootstrap構成では、これらがすべてのサブプロセスで、サンドボックスの
制限下で実行される。cefclient のクライアントDLLが薄いのに対し、
Chronos のDLLはMFCアプリケーション一式を含んでいる点が本質的な差である。

#### 切り分け結果: 静的初期化で落ちている

一時的な計測コードを入れて切り分けた。結果は次のとおり。

**(1) サンドボックス化されたプロセスはファイルに何も書けない。**

`DllMain` と `RunWinMain` の入口で `%LOCALAPPDATA%\Chronos\` にトレースを
書き出したところ、記録が残ったのは `--service-sandbox-type=none` の
プロセスだけだった。

```
pid=13460  dllmain-attach      --type=browser(none)
pid=13460  runwinmain-enter    --type=browser(none)
pid=58896  dllmain-attach      --type=utility --utility-sub-type=network.mojom.NetworkService ... --service-sandbox-type=none
pid=58896  runwinmain-enter    --type=utility --utility-sub-type=network.mojom.NetworkService ... --service-sandbox-type=none
pid=66976  runwinmain-returned --type=utility --utility-sub-type=chrome.mojom.ProcessorMetrics ... --service-sandbox-type=none
```

renderer / gpu / storage は1行も残らない。これらは正常に起動している
プロセスも含めて書けていないので、**「記録がない=クラッシュした」ではない**。
サンドボックス下ではファイル出力は調査手段として使えない。

**(2) 例外は `RunWinMain()` を通っていない。**

サンドボックス下でも観測できる経路はイベントログだけなので、
`RunWinMain()` の本体を `try` / `catch(...)` で囲み、捕捉したら
独自コード `0xE0C40001` で `RaiseException` するようにした。

結果、23回のクラッシュすべてが `0xE06D7363` のままで、`0xE0C40001` は
一度も出なかった。つまり例外は `RunWinMain()` の中では発生していない。

**結論: 例外は `RunWinMain()` に入る前、
すなわち ChronosN.dll のロード時の静的初期化で発生している。**

bootstrap.exe が `LoadLibrary` した時点で例外が投げられ、
そのまま未処理例外としてプロセスが落ちている、という説明と整合する。

なお、Chronosのファイルスコープのグローバルオブジェクトは
`theApp`(`CSazabi`)、`std::mutex`、`std::set`、ポインタ1個だけであり、
`CSazabi` とそのメンバーのコンストラクタは値の初期化しか行っていない。
GDI+やCOMの初期化は `CSazabi::InitFunc_Base()`(ブラウザープロセスのみ)で
行われるため、静的初期化の段階では走らない。
したがって、疑わしいのはMFC自身の静的初期化(`#pragma init_seg(lib)` の
オブジェクト群)ということになる。

#### 例外の観測: VEH経由で型名を取得する

`SetUnhandledExceptionFilter()` で仕掛けたフィルタは呼ばれなかった。
Chromiumは第三者による最上位例外フィルタの差し替えを無効化するため。
一方、**ベクタ例外ハンドラ(`AddVectoredExceptionHandler`)は差し替えを
阻止されない**ので、こちらを使う。

出力経路はイベントログの例外コードしかないため、コードそのものに情報を載せた。

```
0xE0 T C C C    T   = プロセス種別 (2:renderer 3:gpu 4:storage 5:その他utility)
                CCC = 投げられたC++型名の先頭3文字
```

型名は `ExceptionInformation[2]` の `ThrowInfo` から
`CatchableType` → `TypeDescriptor::name` をたどって取得する。
なお **MFCは例外をポインタで投げる**ため、名前は `.?AV...` ではなく
`.PAV...` で始まる点に注意(最初この分岐を誤って空振りした)。

結果:

| 例外コード | 回数 | プロセス種別 | 型名の先頭3文字 |
|---|---|---|---|
| `0xE4435265` | 4 | storage service | `CRe` |
| `0xE2435265` | 2 | renderer | `CRe` |
| `0xE5435265` | 1 | その他のutility | `CRe` |

`CRe` はMFCの **`CResourceException`** と考えられる。
**GPUプロセスでは発生していない**点が重要で、GPUはwin32kを必要とするため
win32kロックダウンが適用されない。一方 renderer / utility には適用され、
user32/gdi32 によるリソース読み込みが不可能になる。この差と症状は整合する。

#### 原因の確定: MFCの `afxGlobalData` がwin32kを呼ぶ

WERのLocalDumpsでフルダンプを採取し、解析して原因が確定した。
WERは未処理例外の時点でダンプを採るため、以下が致命的な例外そのものである。

投げられた例外(`ThrowInfo` の `CatchableType` を展開したもの):

```
.PAVCResourceException@@
.PAVCSimpleException@@
.PAVCException@@
.PAVCObject@@
.PAX
```

コールスタック:

```
__DllMainCRTStartup
 -> dllmain_crt_process_attach        DLLのC++静的初期化
  -> __initterm
   -> ??__EafxGlobalData              MFCのグローバル afxGlobalData の初期化子
    -> AFX_GLOBAL_DATA::AFX_GLOBAL_DATA()
     -> AFX_GLOBAL_DATA::Initialize()
      -> AFX_GLOBAL_DATA::UpdateSysColors()
       -> CWindowDC::CWindowDC(CWnd*)
        -> AfxThrowResourceException()
         -> _CxxThrowException
```

MFCのソース(`atlmfc/src/mfc/afxglobals.cpp`、`wingdi.cpp`)を見ると:

```cpp
void AFX_GLOBAL_DATA::UpdateSysColors()
{
	...
	CWindowDC dc(NULL);
	m_nBitsPerPixel = dc.GetDeviceCaps(BITSPIXEL);
	...
}

CWindowDC::CWindowDC(CWnd* pWnd)
{
	if (!Attach(::GetWindowDC(m_hWnd = pWnd->GetSafeHwnd())))
		AfxThrowResourceException();
}
```

**win32kロックダウン下では `::GetWindowDC(NULL)` がNULLを返すため、
MFCのグローバルオブジェクトの初期化が必ず例外を投げる。**

これは推測どおりの結論だったが、意味合いは当初の想定より重い。
`afxGlobalData` はMFCのスタティックライブラリ内のグローバルオブジェクトであり、
CRTの初期化中に無条件に構築される。Chronos側から抑止する手段はない。
つまり、

> **スタティックリンクされたMFCは、win32kロックダウンされたプロセスには
> ロードできない。**

GPUプロセスだけが無事だったのは、GPUがwin32kを必要とするため
ロックダウンの対象外だからである。renderer / utility / storage には
ロックダウンが適用されるため、すべて同じ場所で落ちる。

なお、この制約はbootstrap構成に固有のものではない。EXE構成のChronosでも
サブプロセスは同じEXEを起動し、`CefExecuteProcess()` の前に
同じ静的初期化を通っていた。サンドボックスを無効にしていたため
表面化していなかっただけである。

#### 検証: APIを個別に差し替える方法は成立しない

まず影響範囲を測るために、遅延ロードの通知フック
(`__pfnDliNotifyHook2` の `dliNotePreGetProcAddress`)で
`user32!GetWindowDC` だけをスタブに差し替えて試した
(`user32.dll` は `DelayLoadDLLs` に含まれるため差し替えできる。
なおVisual Studio 2015 Update 3以降 `__pfnDliNotifyHook2` は const で、
実行時に代入するのではなく自前で定義する)。

結果、**storage サービスのクラッシュループは止まり、プロセス構成は揃った**。
しかしページは表示されず、クラッシュの種類が変わっただけだった。

| 例外 | 発生モジュール | Chronos側の最初のフレーム | 件数 |
|---|---|---|---|
| `0xC0000005` | gdi32.dll | `CGdiObject::DeleteObject` | 13 |
| `0xE06D7363` | KERNELBASE.dll | `CInvalidArgException` | 8 |
| `0x80000003` | ChronosN.exe / libcef.dll | - | 9 |

`AFX_GLOBAL_DATA::Initialize()` は `UpdateSysColors()` の後に
`UpdateFonts()` を呼び、そこだけでwin32k依存の呼び出しが25箇所
(うち `::DeleteObject` が10箇所)ある。フォント生成が失敗した状態で
そのままGDIハンドルを操作するため、次々に別の形で落ちる。

**個別のAPIを差し替えていく方針は、もぐら叩きになり成立しない。**

#### 対応方針の選択肢

1. **MFCをスタティックリンクから共有DLL(`UseOfMfc = Dynamic`)に変更する**
   ← 推奨

   MFCのヘッダ(`afxglobals.h`)に答えが書かれている。

   ```cpp
   // Initialization is not done during the construction of the afxGlobalData
   // object because it requires calling a number of GDI32/USER32 APIs, which
   // violates the best practices for DLLMain.
   inline AFX_GLOBAL_DATA *GetGlobalData()
   {
       if (!afxGlobalData.m_bInitialized)
       {
           afxGlobalData.Initialize();
           afxGlobalData.m_bInitialized = TRUE;
       }
       return &afxGlobalData;
   }
   ```

   そして `AFX_GLOBAL_DATA` のコンストラクタは:

   ```cpp
   #ifndef _AFXDLL
       Initialize();
   #endif
   ```

   つまり **`Initialize()` をコンストラクタから呼ぶのはスタティックリンクの
   ときだけ**で、共有DLL版では実際にUIを使う時点まで遅延される。
   サブプロセスではMFCのUIを一切使わないため、初期化は走らない。
   Microsoft自身が「DLLMainでGDI32/USER32を呼ぶのは作法違反」と
   認めているとおり、スタティックリンク時の挙動のほうが例外的である。

   影響:
   * `RuntimeLibrary` も `/MT` から `/MD` に変更が必要
   * `mfc140u.dll` / `vcruntime140.dll` / `msvcp140.dll` などの再頒布が必要になり、
     現在の自己完結した配布構成が変わる
   * ThinAppパッケージへの影響を確認する必要がある

2. **サブプロセス用の薄いDLLを分ける**(CEFが想定している構成)

   MFCをリンクしない小さなDLLを用意し、`RunWinMain` から
   `CefExecuteProcess()` を呼ぶだけにする。
   `CefBrowserProcessHandler::OnBeforeChildProcessLaunch()` で
   子プロセスのコマンドラインに `--module=<名前>` を追加すれば、
   bootstrap.exeはそちらのDLLを読み込む。
   `AppRenderer` は `CString` を使っているため、MFC非依存への書き換えが必要。
   配布構成は変わらないが、プロジェクトが増え、変更量は1より大きい。

1は設定変更で済み、MFCの設計意図にも沿う。配布物が増える点だけが論点になる。

#### 採用: MFC共有DLLへの切り替え(実施済み・サンドボックス有効で動作)

方針1を実施し、**サンドボックスを有効にしたままページが表示される状態になった**。

```
BROWSER                                    sandboxed
gpu-process                                sandboxed
utility:network.mojom.NetworkService       sandboxed
utility:storage.mojom.StorageService       sandboxed
renderer                                   sandboxed
renderer                                   sandboxed
```

`--no-sandbox` が付与されたプロセスは0、クラッシュダンプも0件。
Debug(D64_CSG)・Release(R64_CSG)の両構成で確認した。

実際に必要だった変更は次の4点。

1. `Sazabi.vcxproj`
   * `UseOfMfc`: `Static` → `Dynamic`
   * `RuntimeLibrary`: `MultiThreaded` → `MultiThreadedDLL`
     (Debugは `MultiThreadedDebug` → `MultiThreadedDebugDLL`)
   * `DelayLoadDLLs` から `IMM32.dll` / `UxTheme.dll` / `WINSPOOL.DRV` /
     `MSIMG32.dll` / `oledlg.dll` / `oleacc.dll` を削除。
     これらのインポートは `mfc140u.dll` 側に移り、ChronosN.dll から
     参照されなくなったため LNK4199 になる(CIは `/warnaserror`)
   * MFC / CRT のランタイムDLLを出力先へコピーするターゲットを追加
2. `setup-cef.bat`
   * CEFラッパーのビルドに `-DCEF_RUNTIME_LIBRARY_FLAG=/MD` を追加。
     CEFの既定は `/MT` で、そのままだとランタイムの不一致で
     `LNK2038` になる
3. `BroFrame.h`
   * `CMyStatusBar::GetPainProgressTotal()` が
     `CMFCStatusBar::_GetPanePtr()` を呼んでいた。これはprotectedで
     MFC共有DLLからエクスポートされないため `LNK2001` になる。
     進捗バーの状態を自前で保持するように変更した
     (`EnablePaneProgressBar()` の呼び出しはすべて `CMyStatusBar` 経由なので、
     非仮想関数を隠蔽するだけで状態を同期できる)
4. 配布物の追加(下記)

##### 配布物に追加が必要なDLL

| 構成 | ファイル |
|---|---|
| Release | `mfc140u.dll`、`msvcp140.dll`、`vcruntime140.dll`、`mfc140jpn.dll` |
| Debug | `mfc140ud.dll`、`msvcp140d.dll`、`vcruntime140d.dll`、`mfc140jpn.dll` |

`mfc140jpn.dll` はリソースのみのDLLでデバッグ版が存在せず、両構成で共通。
`ucrtbase.dll` はWindows 10以降OSに含まれるため不要。

ビルド時は `Sazabi.vcxproj` の `ChronosCopyRedist` ターゲットが
インストール済みツールセットからコピーするため、リポジトリには含めていない。
**リリース配布物やThinAppパッケージにこれらを含める作業は別途必要。**

##### 残作業

* インストーラー/ThinAppパッケージへのランタイムDLLの取り込み
* Debug版のランタイムDLLは再頒布不可のため、配布物に混入しないことの確認
* [SANDBOX.md](SANDBOX.md)後半のチェックリストにもとづく機能回帰試験
  (SGモードの保存先制限、アップロード制限、印刷など)
* ThinApp環境での検証(Phase 3)

#### 調査の途中で誤っていた点

上記には**確定していない点がある**。VEHは「プロセス内で最初に発生した
C++例外」で発火させているが、MFCは内部で例外を投げて自分で捕捉する。
実際、VEHを入れると1回の実行あたりのクラッシュ件数が25件から14件に変化した。
これは**本来生き残るはずのプロセスまで計測が殺している**ことを意味し、
観測した `CResourceException` が致命的な例外そのものとは限らない。

また、`RunWinMain()` を `try` / `catch` で囲んだ切り分けにも穴があった。
`AfxInitialize()` の呼び出しが `try` の外にあったため、
「`RunWinMain()` の中では起きていない」は厳密には
「`ChronosRunMain()` の中では起きていない」でしかない。

加えて、`CWinApp` のコンストラクタはMFCのソースを確認したかぎり
リソースを読み込んでいない。静的初期化で `CResourceException` が
投げられる経路は、現時点では説明できていない。

#### 次に試すこと

1. WERのLocalDumps(`HKLM\SOFTWARE\Microsoft\Windows\
   Windows Error Reporting\LocalDumps`)を設定してミニダンプを採取し、
   スタックを直接見る。ここまでの間接的な観測をすべて置き換えられる、
   もっとも確実な方法。管理者権限とシステム設定の変更が必要
2. VEHを「最初の1回」ではなく「捕捉されずに終わった例外」でのみ
   発火させる方法を用意する(`std::set_terminate` の併用など)
3. 型名の全体(先頭3文字ではなく完全な名前)を取得できるようにする

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

# CEFを更新するときの移植ガイド

## はじめに

Chronosで採用しているCEFを継続的に更新していくために必要な情報を提供することが目的のドキュメント。

## 移植の基本的な流れ

1. [HOWTOBUILD](../HOWTOBUILD.md)を参考に、setup-cef.batを更新し、採用すべきビルド済みバイナリを変更する。
2. Visual Studio 2022を起動しソリューションをビルドするか、Developer Command Prompt for VS 2022を起動し、msbuildでビルドする。参考:[build.yaml](../.github/workflows/build.yaml)

```
msbuild /m /p:Platform=Win32 /p:Configuration=R64_CSG
msbuild /m /p:Platform=Win32 /p:Configuration=D64_CSG
```

3. ビルドエラーが発生したときのログを採取し、問題を調査・修正する

## ビルドエラー時の調査方法

ビルドエラーはどの時点で非互換な変更が入ったのか、を手がかりに問題を調査するとよい。
そのためには、手元に過去のビルド済みアーカイブをダウンロードする。

過去のバージョンを取得する方法は後述の「古いビルド済みのアーカイブを取得する方法」を参照すること。

Chronosで影響を受けやすいポイントは次のとおり。

* include/capiの変更
* include/internalの変更

上記2つのディレクトリ配下の変更を受けることが多い。

1. どのバージョンで該当する変更が入ったのかビルド済みバイナリのヘッダを調査
2. 変更の入ったコミットを https://github.com/chromiumembedded/cef.git から特定
3. 該当コミットに参照先issueの番号が書かれているので、該当するissueを調査する
   https://bitbucket.org/chromiumembedded/cef/issues/xxxx/
4. Chromium本体のバグについて言及されていることもあるので必要に応じて参照する (例: https://crbug.com/1208141 など)

上記を繰り返すことで、どのような非互換な変更が入ったのかを特定し、対策を検討することが可能になる。

## 古いビルド済みのアーカイブを取得する方法

移植を行うときには、どの段階でビルドできなくなる変更点が入ったのかを調査したいことがある。

[Chromium Embedded Framework (CEF) Automated Builds](https://cef-builds.spotifycdn.com/index.html#windows32)では、
最新のBetaもしくはStableのアーカイブしか参照できるようになっていない。

ただし、index.htmlはindex.jsonをもとに描画されているので、 https://cef-builds.spotifycdn.com/index.json から必要な情報を抜き出すことができる。

例えば、安定版のバージョン一覧は次のようにして取得できる。

```
curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows32.versions[] | select(.channel == "stable").cef_version'
```

ベータだと次のようにして取得できる。

```
curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows32.versions[] | select(.channel == "beta").cef_version'
```

例えば、98.2以降の安定版のビルド済みバイナリをダウンロードするには次のようなスクリプトを実行すれば取得できる。
(マイナーバージョンも取得することになるので、必要に応じて間引く工夫をするとよい。)

```
for v in `curl --silent https://cef-builds.spotifycdn.com/index.json | jq --raw-output '.windows32.versions[] | select(.channel == "stable").cef_version'`; do
    echo $v
    command="curl --silent -O https://cef-builds.spotifycdn.com/cef_binary_${v}_windows32_minimal.tar.bz2"
    eval $command
    if [ "$v" = "98.2.1+g29d6e22+chromium-98.0.4758.109" ]; then
        exit 1
    fi
done
```

ベータの場合、ファイル名に_beta_が含まれる点が安定版と異なることに注意。

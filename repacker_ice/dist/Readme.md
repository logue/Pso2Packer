# repacker_ice

v1.3.1

## ICEファイル(.ice)の展開方法

ICEファイルをice.exeにドラッグ＆ドロップすると
ice.exeと同じディレクトリにフォルダが作られ、ファイルが展開されます。
複数ファイルやフォルダの展開にも対応してます。

## ICEファイル(.ice)の作成方法

ファイルを`Pack_compressed.bat`か`Pack.bat`にドラッグ＆ドロップすると
ICEファイルが作成されます。
複数ファイルやフォルダのドラッグ＆ドロップにも対応してます。

## ICEファイル(.ice)に格納されたファイルのリスト作成方法

ICEファイルをファイルリスト作成.batにドラッグ＆ドロップすると
ファイルリストのテキストが作成されます。
複数ファイルやフォルダのドラッグ＆ドロップにも対応してます。

## ICEファイル(.ice)に格納されたファイルの差し替え方法

コマンドプロンプトで以下のようなコマンドを実行してください。
ICEファイル内に差し替えるファイルと同じファイル名があるとき、そのファイルだけ差し替えます。
差し替えるとき、そのままのグループで保存されます。

```bat
ice.exe -r "ここに差し替えるファイルパス" "ここにiceファイルのパス"
```

圧縮したいときは -c オプションを付けます。

```bat
ice.exe -c -r "ここに差し替えるファイルパス" "ここにiceファイルのパス"
```

また、この方法で差し替えるときのみ、CRCを維持する -m オプションが使えます。

```bat
ice.exe -m -r "ここに差し替えるファイルパス" "ここにiceファイルのパス"

ice.exe -c -m -r "ここに差し替えるファイルパス" "ここにiceファイルのパス"
```

## 仕様

現在対応しているICEファイルのバージョンはv3からv9のみです。

ICEファイルは格納ファイルが2つのグループに分けられています。

グループ分けの基準が不明なため、ICEファイルを作成するとき`GroupList.txt`に書かれた拡張子はグループ1へ、その他は全てグループ2に入れています。

-kスイッチで8桁の16進数のパスワードキーを入力できます。

```bat
ice.exe -k CD50379E "ここにiceファイルのパス"
```

## 免責事項

このアプリケーションを利用した事によるいかなる損害も作者は一切の責任を負いません。
自己の責任の上で使用して下さい。

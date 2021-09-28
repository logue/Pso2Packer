※このプロジェクトは、[NgsPacker](https://github.com/logue/NgsPacker)として作り直すことになりました。

 # Pso2Packer

[![Build status](https://ci.appveyor.com/api/projects/status/y7igdk3095jljal3?svg=true)](https://ci.appveyor.com/project/logue/pso2packer)

PSO2のクライアントで読み込めるICE形式のファイルを、開いたり作成したりするためのプログラム。バイナリなどは、自分でコンパイルするか[AppVeyorのArtifact](https://ci.appveyor.com/project/logue/pso2packer/build/artifacts)から取得して下さい。※現時点で使用可能なものはrepacker_iceのみです。

## 概要

|プロジェクト名     |説明
|-------------------|-------------------------------------------------------
|repacker_ice       |ICE形式の展開/圧縮を行うコマンドラインツールのソースコード（ネットから拾った）
|IceResource        |ICE形式のファイルをやり取りするためのコアDLL（の予定）
|IceResource.net    |上記のDLLを.netから使用するためのラッパーDLL（の予定）
|IceBrowser         |MabiPackerのPackViewerの移植（の予定）

## コントリビュート時のお願い

* 開発ツールは[Viual Studio 2017](https://www.visualstudio.com/downloads)および、[Visual Studio Code](https://www.microsoft.com/ja-jp/dev/products/code-vs.aspx)です。いずれも、かならず最新のものを使用して下さい。
* C言語の場合、[clang-format](https://clang.llvm.org/docs/ClangFormat.html)でコード整形をした後、エンコードはUnicode LEに統一してください。
  * 別途[LLVM](http://llvm.org/builds/)の32bit版をインストールする必要があります。
  * Visual Studio用プラグインは、機能拡張・更新からインストールできます。

## 免責事項

このソフトウェアは、PSO2のインストール環境にダメージを与える可能性があることについて留意して下さい。
また、いかなる場合においてもSEGAは、Moddingに関して支持したり黙認するものではありません。潜在的にバンされる可能性があります。

本ツールを使用したことによって、アカウントをバンされるなどの被害があっても一切の責任を負いかねます。利用は、自己責任でお願いします。

## see also

* [PSO2ModManager](https://github.com/PolCPP/PSO2-Mod-Manager) - 日本語化（厳密には多言語化）とGUI改良を担当。

## ライセンス

販売するアホがいるのでGPLというわけじゃなく、流用元の[MabiPacker](https://github.com/logue/MabiPack)のライセンスがGPLなため。

[GPL3](LICENSE)

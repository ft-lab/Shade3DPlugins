＜＜ 概要 ＞＞ ----------------

Shade 13.2.2 Plugin SDK(463042)用のテンプレートプロジェクトです。
RenderManのRIBファイルと画像をエクスポートするプラグインです。

Shade 3D ver.14/ver.15で動作。

 SDKを解凍後、以下のようなディレクトリ配置にします。

[boost_1_43_0]
[doc]
[include]
[samples]

[projects]         <== なければ作成
   [RIBExporter]   <== 複製
      [libtiff]
      [mac]
      [source]
      [win]

Win環境の場合は、projects/win/Template.sln をVisual Studio 2010で起動します。
Mac環境の場合は、projects/mac/plugins/xplugins.xcodeproj を XCode 6.1.xで起動します。

Winの場合はRelease、Macの場合はDeploymentでビルドするとプラグインが生成されます。


＜＜ libtiffのビルドについて ＞＞ ----------------

GitHub上では、libtiffのincludeとlibは置いていません。
以下より、libtiffのライブラリのビルドを行い、ヘッダをコピーしておきます。

http://www.remotesensing.org/libtiff/
より、「tiff-4.0.3.zip」をダウンロードしてビルドします。

[libtiff]ディレクトリ内は以下の配置になります。
--------
[include]
  t4.h
  tiff.h
  tiffconf.h
  tiffconf.vc.h
  tiffconf.wince.h
  tiffio.h
  tiffio.hxx
  tiffiop.h
  tiffvers.h
  tif_config.h
  tif_config.vc.h
  tif_config.wince.h
  tif_dir.h
  tif_fax3.h
  tif_predict.h
  uvcode.h

[lib]
  [mac]
    libtiff.a      <== Macのstaticライブラリ
  [win]
    [win32]
      libtiff.lib  <== Win 32bitのstaticライブラリ
    [x64]
      libtiff.lib  <== Win 64bitのstaticライブラリ
--------

Winは、VS 2010でビルドしました。
Visual Studio Toolsの「Visual Studio コマンド プロンプト (2010)」で32bit版のlibをnmake。
「Visual Studio x64 Win64 コマンド プロンプト (2010)」で64bit版のlibをnmake。

Macは、ターミナルで

./configure CFLAGS='-arch x86_64 -arch i386'
make

としてmakeすると「libtiff/.libs」内に「libtiff.a」ができているのでこれを使用。
Macのみ、tiff.hのTIFF_INT64_Tをlong longに変更。TIFF_UINT64_Tをコメントアウト。
（これをしないと、ビルド時にエラーになります）


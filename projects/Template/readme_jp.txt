Shade 13.2.2 Plugin SDK(463042)用のテンプレートプロジェクトです。
plugin_interfaceとしてメッセージを表示するだけのもの。
Shade 3D ver.14/ver.15で動作（未確認ですが、ver13.xでも動作すると思います）。

 SDKを解凍後、以下のようなディレクトリ配置にします。

[boost_1_43_0]
[doc]
[include]
[samples]

[projects]      <== なければ作成
   [Template]   <== 複製
      [mac]
      [source]
      [win]

Win環境の場合は、projects/win/Template.sln をVisual Studio 2010で起動します。
Mac環境の場合は、projects/mac/plugins/xplugins.xcodeproj を XCode 6.1.xで起動します。

Winの場合はRelease、Macの場合はDeploymentでビルドするとプラグインが生成されます。


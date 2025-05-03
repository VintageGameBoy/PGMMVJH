如何编译《Pixel Game Maker MV》的开源版本

准备工作：  
首先安装 Visual Studio 2022 Community 版。  
在安装 Visual Studio 时，应该选择“桌面开发（C++）”工作负载。  
如果你使用的是 Visual Studio 2022 而不是 Community 版，请在 build_player.bat 文件中修改为 "C:\Program Files\Microsoft Visual Studio 2022"。

构建步骤：  
1. 打开 Installer 文件夹。  
2. 运行 build_player.bat。  
如果构建成功，Installer 文件夹中会创建三个文件夹：player、runtime 和 debugMenuRuntime。  
*如果需要重新构建 libchipmunk.lib，请参考 ../Player/cocos2d/external/win32-specific/chipmunk/libchipmunk/readme_e.txt。  
*构建路径名称中不应包含多字节字符（例如日文）。

替换步骤：  
1. 打开 Pixel Game Maker MV 的 Steam 版本安装文件夹。（我们称其为 PGM 文件夹。）

- 替换测试播放时使用的播放器。  
  用 PGM/player-win 文件夹覆盖 Installer/player 文件夹。

- 替换在“构建游戏”中使用的播放器（“启用调试功能”关闭时）。  
  解压 PGM/resource/runtime.zip。（会创建 PGM/resource/runtime 文件夹。）  
  用 PGM/resource/runtime 文件夹覆盖 Installer/runtime 文件夹。  
  将 PGM/resource/runtime 文件夹重新打包为 zip 文件，并用它覆盖 PGM/resource/runtime.zip。

- 替换在“构建游戏”中使用的播放器（“启用调试功能”开启时）。  
  解压 PGM/resource/debugMenuRuntime.zip。（会创建 PGM/resource/debugMenuRuntime 文件夹。）  
  用 PGM/resource/debugMenuRuntime 文件夹覆盖 Installer/debugMenuRuntime 文件夹。  
  将 PGM/resource/debugMenuRuntime 文件夹重新打包为 zip 文件，并用它覆盖 PGM/resource/debugMenuRuntime.zip。

注意事项：  
开源版本不支持加密构建。加密的游戏将无法启动。

完成。
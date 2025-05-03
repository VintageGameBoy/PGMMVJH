# Pixel Game Maker MV Player Open Source Edition

[English README](README.md)

## 介绍

"Pixel Game Maker MV Player Open Source Edition" 是 "Pixel Game Maker MV" (PGMMV) 运行游戏和软件所需的播放器应用的开源版本，适用于 Windows 平台。

## 这个项目是什么？

本项目旨在通过社区和游戏开发者的支持来改进 PGMMV 播放器应用。
本项目开发的改进将由 Gotcha Gotcha Games 分发给所有 PGMMV 用户。
本项目当前使用的播放器版本为 1.0.6.5，并未包含 1.0.6.6 及之后版本的修复。

## 如何加入

- 本项目主要使用英语进行交流，但也允许使用日语。
- 本项目遵循 Github Flow 规则。请为每个 Pull Request (PR) 创建一个新的分支。一旦 PR 获得批准，它必须合并到 master 分支。http://scottchacon.com/2011/08/31/github-flow.html
- 本项目需要使用 "Visual Studio 2022 Community" 并安装 C++ 组件。
- 本项目使用的 "cocos2d-x" 版本为 3.17.1。
- 项目讨论将在官方的 Pixel Game Maker MV Discord 频道进行：https://discord.com/invite/FtRVRkx5tP

## 规划路线

本项目最初的目标是修复现有的 bug。
贡献者可以创建自定义版本的播放器来优化其游戏的性能，但目前仅接受 bug 修复相关的 PR。
贡献者的名字将被添加到 "Pixel Game Maker MV" 的开发者名单中。

## 如何构建

请查看附带的 README 文件：[readme_zh.txt](/Installer/readme_zh.txt)

## 如何使用自定义版本的播放器

从 2023 年 11 月 21 日起，可以通过 Steam 上的 "1.0.6-Osplayer-beta" 分支使用自定义的 Player 组件。
启用该分支后，您将可以选择用于 Player、Runtime 和 Debugging Runtime 组件的版本。

### Player:
这是 PGMMV 编辑器中进行测试运行时使用的 Player 组件版本。
默认版本是随 PGMMV 编辑器安装的 Player 组件，位于 (安装目录)/player-win/player.exe。

### Runtime:
这是在构建游戏时打包的 Player 组件版本，也是玩家运行游戏时使用的版本。
默认版本是 PGMMV 安装目录中的版本，路径为 (安装目录)/resource/runtime.zip。

### Debugging Runtime:
当游戏使用调试标志构建时，所使用的 Runtime 版本。
默认版本是 PGMMV 安装目录中的版本，路径为 (安装目录)/resource/debugMenuRuntime.zip。

## 许可证

本项目基于 MIT 许可证发布。
[LICENSE](LICENSE)

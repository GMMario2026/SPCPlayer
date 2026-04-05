<h1 align="center">SPC 播放器</h1>

<p align="center">
    <img src="Images/SPCPlayer Icon.ico" width="256" style=""/>
</p>

<p align="center">
    <a href="https://github.com/GMMario2026/SPCPlayer/releases/"><img src="https://img.shields.io/github/release/GMMario2026/SPCPlayer.svg" alt="GitHub release"></a>
    <a href="https://github.com/GMMario2026/SPCPlayer/blob/main/LICENSE"><img src="https://img.shields.io/badge/license-MIT-blue.svg" alt="MIT license"></a>
    <a href="https://github.com/GMMario2026/SPCPlayer/pulls"><img src="https://img.shields.io/badge/PRs-welcome-brightgreen.svg" alt="PRs Welcome"></a>
    <a href="https://github.com/GMMario2026"><img src="https://img.shields.io/badge/Donate-contact%20me-green.svg" alt="Donate"></a>
</p>

## 功能

播放 SNES SPC 音乐文件，并以钢琴卷帘的方式可视化。

## 特色

- **SPC 加载器**：
  - 使用 split700 库提取 BRR samples，用 vamp 的 pYin 算法分析样本的基准频率，其效果比 [spc-presenter-rs](https://github.com/nununoisy/spc-presenter-rs) 更准确。
  - 使用 blargg 的 [snes-spc](https://github.com/blarggs-audio-libraries/snes_spc) 库进行模拟，捕获事件并提取音频波形。
- **OpenGL 渲染器**：
  - 使用 [PianoFromAbove](https://github.com/brian-pantano/PianoFromAbove) 的游戏和渲染引擎，基于 OpenGL 进行重新实现。
  - 支持跳转、调节音量、静音、修改音轨颜色、修改音符长度、隐藏键盘等功能。
- **界面引擎**：
  - 基于 Qt6 框架搭建 UI 界面和多媒体模块，实现跨平台。

## 安装

- **Windows**：Releases 页面提供了使用 LLVM-MinGW 编译的 x86_64 版本，下载解压后双击运行 SPCPlayer.exe 即可。
- **其它**：目前还没有二进制发布，但是你可以手动编译，参考下面的**编译**部分。

## 编译

- 克隆本仓库。
- 在本机上安装 Qt6（建议 6.8 LTS），注意要勾选安装 Qt6 Multimedia 模块和 Qt Creator。
- 打开 Qt Creator，配置完成后用其打开仓库根目录下的 SPCPlayer.pro。
- 点击左下角的编译按钮完成编译（release）。
- 将根目录下的 qt.conf 文件复制到可执行文件所在的目录下。

## 后续计划

- 调整 SNES SPC 事件提取算法。
- 新增播放速度调节功能（防止调速时音高变化）。
- 优化 SNES SPC 文件的加载与模拟速度（目前一个 3 分钟的文件要 10 秒加载）。
- 完成多语言支持。

如果你发现了 BUG，或者有好的点子，可以提交 issue 或 PR，这会对我的代码完善有很大帮助。

## 许可证

本项目基于 [MIT](https://github.com/GMMario2026/SPCPlayer/blob/main/LICENSE) 许可证开源。

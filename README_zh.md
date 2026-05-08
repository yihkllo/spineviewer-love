<div align="center">

# SpineLove

</div>

SpineLove 是一个基于 Direct3D 11 的 Spine 动画预览工具，重点是本地预览、
多版本运行时适配、多 Spine 图层、slot 检查和轻量动画控制。

## 功能

- 支持预览 Spine 2.1 到 4.2 的骨骼资源。
- 支持加载 `.skel` 和 `.json`，并配合 atlas 贴图使用。
- 使用 D3D11 渲染，支持预乘 Alpha、背景图、多 Spine 图层。
- 支持动画播放、队列播放、Track Mix、Skin Mix 和单独图层变换。
- 支持 slot 显示控制、名称过滤、鼠标悬停选择和轮廓显示。
- 支持文件夹扫描、收藏、语言切换、主题调整和全屏模式。

## 构建

用 Visual Studio 2022 打开 `spinelove.sln`，选择 `x64` / `Release`，
构建 `SpineLove` 项目。

生成的程序位置：

```text
out/build/sln/bin/x64/Release/spine love.exe
```

## 操作

| 输入 | 功能 |
|---|---|
| 鼠标滚轮 | 以鼠标位置为中心缩放当前 Spine。 |
| Shift + 鼠标滚轮 | 同时缩放所有可见 Spine 图层。 |
| 左键拖动 | 移动当前 Spine。 |
| Shift + 左键拖动 | 同时移动所有可见 Spine 图层。 |
| 鼠标中键 | 重置当前 Spine 的缩放并适配窗口。 |
| Shift + 鼠标中键 | 重置所有可见 Spine 图层。 |
| Ctrl + 左键拖动 | 移动背景图。 |
| Ctrl + 鼠标滚轮 | 缩放背景图。 |
| F11 | 切换全屏。 |

## 面板

| 面板 | 功能 |
|---|---|
| File | 加载单个 Spine 骨骼文件。 |
| Select Folder | 扫描文件夹并列出 Spine 文件。 |
| Spines | 管理多个已加载 Spine 图层。 |
| Animations | 播放骨骼中的动画。 |
| Skin / Skin Mix | 应用单个或多个皮肤。 |
| Size / Flip | 查看尺寸、镜像、旋转和调整变换。 |
| Track Mix | 把额外动画叠加到副轨道。 |
| Slot | 过滤、隐藏、悬停、固定和描边 slot。 |
| Queue | 建立按顺序播放的动画队列。 |

## 第三方代码

项目自身代码使用仓库许可证。随仓库保留的第三方库按各自源码中的许可声明使用，
包括 Dear ImGui 和 Esoteric Software 的 Spine runtimes。

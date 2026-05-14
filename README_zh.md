<div align="center">

# 🌸 SpineLove

**一款轻巧、好看的 Windows Spine 查看器 —— 预览、组合、导出，一气呵成。**

[![Platform](https://img.shields.io/badge/平台-Windows%2010%2B-0078D6?logo=windows&logoColor=white)](https://www.microsoft.com/windows)
[![Renderer](https://img.shields.io/badge/渲染-Direct3D%2011-7B4FFF?logo=microsoft)](https://learn.microsoft.com/windows/win32/direct3d11/)
[![Spine](https://img.shields.io/badge/Spine-2.1%20→%204.2-FF6E1F)](http://esotericsoftware.com/)
[![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?logo=cplusplus&logoColor=white)](https://isocpp.org/)
[![License](https://img.shields.io/badge/许可-参见%20LICENSE-2EA44F)](LICENSE)
[![English](https://img.shields.io/badge/docs-English-EE6677)](README.md)

<br/>

<img src="app.png" alt="SpineLove 截图" width="860"/>

</div>

---

## ✨ 亮点功能

- 🎯 **一个程序覆盖 9 个 Spine 运行时** —— 2.1、3.4、3.5、3.6、3.7、3.8、4.0、4.1、4.2
- 🗂️ **文件夹递归浏览**，支持收藏、方向键快速翻阅
- 🪄 **多 Spine 图层组合** —— 同一画布叠加多个骨骼，可重排、隐藏、统一移动
- 🎬 **动画队列** —— 串联多个动画，按顺序播放或一次性导出
- 🔍 **Slot 检查** —— 鼠标悬停识别 Slot、点击钉住、按勾选或名称隐藏
- 🖼️ **多格式导出** —— PNG / JPG 截图、PNG / JPG 帧序列、MP4 / WebM / GIF（基于 ffmpeg）
- 🌐 **5 种界面语言** —— English、简体中文、繁體中文、日本語、한국어
- 🎨 **自绘标题栏 + 主题 + 背景图**，背景图可独立平移缩放

---

## 📦 支持内容

| 项目 | 说明 |
|---|---|
| 骨骼文件 | `.skel`、`.json` |
| Atlas 文件 | `.atlas`、`.atlas.txt`（同名，放在骨骼文件旁） |
| Spine 版本 | **2.1** · **3.4** · **3.5** · **3.6** · **3.7** · **3.8** · **4.0** · **4.1** · **4.2** |
| 背景图 | PNG / JPG，可参与截图、帧序列与视频导出 |
| 图层 | 单个 Spine，或多个 Spine 在同一画布上组合 |

> 一次打开多个骨骼文件时，请使用 **相同 Spine 版本** 且 **相同数据格式**（全部 `.json` 或全部 `.skel`）的文件。

---

## 🚀 快速开始

1. 运行 **`spine love.exe`**。
2. 点击 **`File`** 打开一个或多个 `.skel` / `.json` 文件。
3. 点击 **`Select Folder`** 递归扫描文件夹，然后用方向键或列表浏览。
4. 在 **Animations** 面板中选择动画，或用 **←** / **→** 切换。
5. 用右侧面板调整变换、轨道混合、Slot、队列和导出。

---

## 🖱️ 鼠标与快捷键

| 输入 | 功能 |
|---|---|
| 画布左键单击 | 切换到下一个动画 *（未开启鼠标 Slot 悬停时）* |
| 左键拖动 | 移动当前 Spine 图层 |
| **Shift** + 左键拖动 | **同时** 移动所有可见 Spine 图层 |
| 鼠标滚轮 | 以鼠标位置为中心缩放当前图层 |
| **Shift** + 滚轮 | 以鼠标位置为中心缩放 **所有** 可见图层 |
| 鼠标中键 | 重置并适配当前图层 |
| **Shift** + 中键 | 重置并适配 **所有** 可见图层 |
| **Ctrl** + 左键拖动 | 移动背景图 |
| **Ctrl** + 滚轮 | 缩放背景图 |
| 左键 + 右键拖动 | 移动窗口 |
| **←** / **→** | 上一个 / 下一个动画 |
| **↑** / **↓** | 文件夹列表中的上一个 / 下一个骨骼文件 |
| **F11** | 切换全屏 |

> 正在输入文字或操作 UI 控件时，键盘快捷键不会触发。

---

## 🧩 面板速览

### 📁 File 与文件夹
**`File`** 用来打开骨骼文件。**`Select Folder`** 递归扫描 `.json` 和 `.skel`，列表支持快速预览、收藏、打开所在文件夹，也可把文件加入为新的 Spine 图层。多 Spine 模式下用普通方式加载文件时，程序会先询问是否替换当前图层。

### ⚙️ Setting
语言、主题、背景色等界面选项。语言选择会保存，下次启动自动应用。

### 🖼️ Background
加载背景图用于预览或导出。**Ctrl + 拖动** 移动，**Ctrl + 滚轮** 缩放。

### 🎚️ Alpha Premultiplied · Load At (0,0)
- **Alpha Premultiplied** —— 当前 Spine 的预乘 Alpha 渲染开关（默认开启）。
- **Load At (0,0)** —— 开启时新加载的骨骼使用 `(0,0)` 偏移；关闭时按打开时的姿势居中显示。

### 📐 Scale / Speed / Mix
当前图层的显示缩放、播放速度，以及切换动画时的混合时间。

### 🎞️ Animations
动画名称与时长。点击播放，也可用 **←** / **→** 切换。

### 👗 Skin
普通模式下选择一个皮肤。开启 **Mix** 后可勾选多个皮肤组合显示。

### 📏 Size / Flip
显示窗口尺寸、骨骼尺寸和当前偏移；提供 **Mirror**（水平镜像）和 **Rotate**（顺时针 90°）。

### 🧵 Track Mix
选择一个或多个动画后点击 **Add**，作为副轨叠加在主动画上。**Clear** 清除副轨。

### 🔎 Slot 工具
- **Exclude slot by items** —— 用勾选列表显示或隐藏 Slot。
- **Hide slots by name text** —— 输入 Slot 名称的一部分再应用，英文忽略大小写。
- **Mouse slot hover** —— 鼠标悬停识别 Slot，点击钉住并绘制轮廓。
- 可显示当前识别或固定 Slot 的边界信息。

### 🎬 Queue
建立有序动画队列。
- 选择动画，点击 **+ Add** 加入队列。
- **Play** 按顺序播放、**Stop** 停止、**Clear** 清空。
- 播放或导出时，当前队列项会高亮显示。

### 🪄 Spines
同时加载多个 Spine 时出现浮动 **Spines** 面板：
- 点击名称切换当前操作对象。
- 单独显示 / 隐藏每个图层。
- 用上下按钮调整绘制顺序。
- 配合 **Shift** 拖动 / 滚轮 / 中键，可统一操作所有可见图层。

---

## 📤 导出

点击右侧 **`Export`** 打开导出面板。

### 截图
| 按钮 | 输出 |
|---|---|
| `PNG` | 当前画面导出为 PNG |
| `JPG` | 当前画面导出为 JPG |
| `Alpha ON` | 保留透明背景（PNG） |
| `Alpha OFF` | 将当前背景烘焙进图像 |

### 帧序列
输出 PNG 或 JPG 图片序列。**Image FPS** 控制帧率。

### 视频
先渲染 PNG 帧，再调用 **`ffmpeg.exe`** 合成。

| 格式 | 备注 |
|---|---|
| `MP4` | H.264 / AAC 封装 |
| `WebM` | VP9 / Opus 封装 |
| `GIF` | 动态 GIF |

**Video FPS** 控制 MP4 / WebM / GIF 的帧率。

> 请将 `ffmpeg.exe` 放在 `spine love.exe` 同目录、`tools/` 子目录，或加入系统 `PATH`。找不到时程序会弹出下载提示。

### 队列导出
打开 **`Queue ON`** 后会导出动画队列，而不是只导出当前动画。队列为空时导出会停止并提示错误。

---

## 🌐 界面语言

| 代码 | 语言 |
|---|---|
| `en` | English *(默认)* |
| `zh_CN` | 简体中文 |
| `zh_TW` | 繁體中文 |
| `ja_JP` | 日本語 |
| `ko_KR` | 한국어 |

在 **Setting → Language** 切换，下次启动会保留。

---

## 📦 发布包目录

正式发布包通常包含：

```
spine love.exe
lang/
render_d3d11/
app.png
NotoSansSC-Regular.ttf
ffmpeg.exe      （可选）
```

请 **不要** 打包 `.pdb`、`.lib`、`.exp`、`.obj` 或 `out/` 等构建产物。

---

## 🛠️ 从源码构建

**环境要求**
- Windows 10 或更新版本
- Visual Studio 2022
- C++17 工具集

**步骤**
1. 用 Visual Studio 打开 `spinelove.sln`。
2. 选择 **`x64` / `Release`**。
3. 构建 **`SpineLove`** 项目。

默认输出路径：

```text
out/build/sln/bin/x64/Release/spine love.exe
```

---

## 📂 项目结构

```
spinelove/
├─ main/
│  ├─ viewer/              # 窗口、输入、面板、渲染主循环
│  ├─ render_d3d11/        # D3D11 渲染器、Overlay、纹理、Shader
│  ├─ runtime_v2/          # 按版本拆分的 Spine 运行时 (2.1 – 4.2)
│  ├─ runtime_shared/      # 与版本无关的骨骼 / 图层 / 探测 API
│  ├─ spine/               # 上游 Spine C++ 源码（按版本）
│  ├─ imgui/               # ImGui 接入、字体、主题、i18n、标题栏
│  └─ image_exporter.*     # PNG / JPG 编码管线
├─ third_party/            # Dear ImGui、stb
├─ vs/                     # Visual Studio 工程文件
└─ spinelove.sln
```

---

## ⚖️ 声明

本项目与 Esoteric Software **无任何关联**。*Spine* 名称及随附的运行时代码归各自权利方所有，并遵循对应许可条款。仓库携带的第三方组件保留各自的许可声明，详见 [`LICENSE`](LICENSE) 与 `third_party/` 目录中的头部说明。

<div align="center">

—— 为 Spine 创作者与爱好者，用 💖 打造 ——

</div>

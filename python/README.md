# PAG Python 绑定

这个目录包含了 libpag 的 Python 绑定，使用 pybind11 实现。

## ⚡ 快速开始

```bash
# 1. 创建虚拟环境
python3 -m venv venv
source venv/bin/activate

# 2. 安装依赖
pip install -r requirements.txt

# 3. 编译安装
./install.sh

# 4. 测试
python -c "import pypag; print('Success!', pypag.__version__)"
```

## 📦 当前功能

目前这是一个**简化版本**的Python绑定，支持：

✅ **PAGFile** - 加载和查询 PAG 文件基本信息
  - `PAGFile.Load(path)` - 加载文件
  - `width()`, `height()` - 获取尺寸  
  - `duration()` - 获取时长
  - `frameRate()` - 获取帧率

✅ **基础类型**
  - `Point` - 2D 点
  - `Color` - RGB 颜色

## 🚧 开发中

以下功能正在开发中：
- PAGSurface - 渲染表面
- PAGPlayer - 播放器控制
- PAGImage - 图片替换
- PAGLayer - 图层操作
- 文本替换功能
- 渲染到 NumPy 数组

## 📖 详细文档

查看 [BUILD_GUIDE.md](./BUILD_GUIDE.md) 了解：
- 完整的构建步骤
- 故障排除指南
- 使用示例

## 🎯 使用示例

```python
import pypag

# 加载 PAG 文件
pag_file = pypag.PAGFile.Load("animation.pag")

if pag_file:
    print(f"尺寸: {pag_file.width()}x{pag_file.height()}")
    print(f"时长: {pag_file.duration() / 1000000.0} 秒")
    print(f"帧率: {pag_file.frameRate()} FPS")
```

## 🛠 支持的平台

- ✅ macOS 10.15+
- ⏳ Linux (即将支持)
- ⏳ Windows (即将支持)

## ⚙️ 系统要求

- Python 3.7+
- CMake 3.13+
- C++17 编译器
- 已同步 libpag 第三方依赖（运行 `../sync_deps.sh`）

# 构建 libpag 的 Python 绑定

## 快速开始

### 推荐方法：使用安装脚本

这是最简单的方法：

```bash
cd python

# 创建虚拟环境（首次）
python3 -m venv venv

# 激活环境
source venv/bin/activate

# 安装依赖
pip install -r requirements.txt

# 运行安装脚本
./install.sh
```

安装脚本会自动完成以下步骤：
1. 清理旧的构建文件
2. 运行 CMake 配置
3. 编译 libpag 和 Python 绑定
4. 将编译好的模块复制到虚拟环境
5. 验证安装

### 方法二：使用 Python venv手动编译

```bash
cd python

# 创建虚拟环境
python3 -m venv venv

# 激活环境
source venv/bin/activate

# 安装依赖
pip install -r requirements.txt

# 手动编译
mkdir -p build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)

# 复制模块
cp pypag*.so ../venv/lib/python3.*/site-packages/pypag.so
```

### 方法三：使用 Conda 环境

如果你已安装 conda：

```bash
# 创建 conda 环境
conda env create -f environment.yml

# 激活环境
conda activate libpag-py

# 运行安装脚本
./install.sh
```

### 2. 验证安装

```bash
# 确保虚拟环境已激活
source venv/bin/activate

# 测试导入
python -c "import pypag; print('pypag version:', pypag.__version__)"

# 测试加载 PAG 文件
python -c "
import pypag
pag = pypag.PAGFile.Load('../assets/like.pag')
if pag:
    print(f'✓ 成功加载 PAG 文件: {pag.width()}x{pag.height()}')
"
```

## 使用示例

### 基础示例：加载和渲染 PAG 文件
## 使用示例

### 基础示例：加载 PAG 文件

```python
import pypag

# 加载 PAG 文件
pag_file = pypag.PAGFile.Load("test.pag")

if pag_file is None:
    print("加载失败")
else:
    print(f"尺寸: {pag_file.width()}x{pag_file.height()}")
    print(f"时长: {pag_file.duration() / 1000000.0} 秒")
    print(f"帧率: {pag_file.frameRate()} FPS")
```

**注意：目前这是一个简化版本的Python绑定，主要支持PAGFile的基本功能。更多功能（如渲染、文本替换等）正在开发中。**
```python
import pypag

# 加载文件
pag_file = pypag.PAGFile.Load("text_template.pag")

# 查看可编辑文本
num_texts = pag_file.numTexts()
for i in range(num_texts):
    text_data = pag_file.getTextData(i)
    print(f"文本 {i}: {text_data.text}")

# 替换文本
pag_file.replaceText(0, "Hello Python!")

# 渲染...
```

### 图片替换

```python
import pypag

# 加载 PAG 文件
pag_file = pypag.PAGFile.Load("template.pag")

# 加载替换图片
image = pypag.PAGImage.FromPath("photo.jpg")

# 替换指定索引的图片
pag_file.replaceImage(0, image)

# 渲染...
```

### 动画帧序列渲染

```python
import pypag
from PIL import Image
import os

pag_file = pypag.PAGFile.Load("animation.pag")
surface = pypag.PAGSurface.MakeOffscreen(pag_file.width(), pag_file.height())
player = pypag.PAGPlayer()
player.setSurface(surface)
player.setComposition(pag_file)

# 渲染 30 帧
os.makedirs("frames", exist_ok=True)
for i in range(30):
    progress = i / 29.0
    player.setProgress(progress)
    player.flush()
    
    pixels = surface.readPixels()
    image = Image.fromarray(pixels, 'RGBA')
    image.save(f"frames/frame_{i:04d}.png")
```

## 运行示例

我们提供了几个示例脚本：

### 简单渲染

```bash
# 渲染单帧
python examples/simple_render.py test.pag output.png 0.5

# 渲染多帧
python examples/simple_render.py test.pag output_dir --frames 30
```

### 文本替换

```bash
# 列出所有可编辑文本
python examples/text_replacement.py test.pag --list

# 替换文本
python examples/text_replacement.py test.pag 0 "New Text" output.png
```

## 故障排除

### 编译错误

如果遇到编译错误，确保：

1. **已安装所有编译依赖**：
   ```bash
   # macOS
   brew install cmake ninja
   
   # Linux
   sudo apt-get install cmake ninja-build
   ```

2. **已同步第三方依赖**：
   ```bash
   cd ..  # 回到 libpag 根目录
   ./sync_deps.sh
   ```

3. **使用正确的 Python 版本**（>= 3.7）：
   ```bash
   python --version
   ```

### 导入错误

如果 `import pypag` 失败：

1. 确认安装成功：
   ```bash
   pip list | grep pypag
   ```

2. 检查 Python 路径：
   ```bash
   python -c "import sys; print(sys.path)"
   ```

### 运行时错误

如果运行时崩溃或出错：

1. 确保 PAG 文件有效
2. 检查文件路径是否正确
3. 确认图片尺寸合理（不要太大）

## API 参考

完整的 API 文档请参考：
- [C++ API 文档](https://pag.io/apis/ios/index.html)（Python API 与 C++ API 基本一致）
- Python 绑定源码：`python/src/bindings/`

## 性能优化建议

1. **启用缓存**：
   ```python
   player.setCacheEnabled(True)
   player.setCacheScale(1.0)
   ```

2. **限制帧率**：
   ```python
   player.setMaxFrameRate(30)
   ```

3. **批量渲染时重用对象**：
   - 不要频繁创建/销毁 Surface 和 Player
   - 重用同一个 Player 渲染多帧

## 贡献

欢迎提交 PR 改进 Python 绑定！

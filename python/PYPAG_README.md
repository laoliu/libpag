# PAG Python 绑定测试文档

## ✅ 完成的功能

### 1. PAG 播放和渲染
- ✅ 加载 PAG 文件
- ✅ 创建离屏渲染表面
- ✅ 播放器控制（进度、帧）
- ✅ 渲染为图片序列（PPM 格式）
- ✅ 读取像素数据

### 2. PAG 编辑功能
- ✅ 查看文件信息（尺寸、时长、帧率）
- ✅ 获取可编辑文本/图片数量
- ✅ 替换文本内容
- ✅ 修改文本属性（字号、颜色）
- ✅ 获取可替换图片索引
- ✅ 图片替换接口（需提供图片文件）

### 3. PAG 层结构检查
- ✅ 列出所有层
- ✅ 获取层信息（名称、类型、可见性、时长）
- ✅ 按名称查找层
- ✅ 获取可编辑索引

### 4. 支持的 PAG 类型
- ✅ PAGFile - PAG 文件
- ✅ PAGComposition - 组合层
- ✅ PAGLayer - 基础层
- ✅ PAGTextLayer - 文本层
- ✅ PAGImageLayer - 图片层
- ✅ PAGSurface - 渲染表面
- ✅ PAGPlayer - 播放器
- ✅ PAGImage - 图片对象
- ✅ TextDocument - 文本数据
- ✅ LayerType - 层类型枚举

## 🎮 使用示例

### 快速开始

```bash
# 激活虚拟环境
cd /Users/liujh/work/pag/libpag/python
source venv/bin/activate

# 运行测试
python pag_test.py ../assets/text1.pag all
```

### 1. 播放 PAG 动画

```python
import pypag

# 加载文件
pag = pypag.PAGFile.Load('file.pag')
print(f"尺寸: {pag.width()}x{pag.height()}")
print(f"时长: {pag.duration() / 1000000:.2f} 秒")

# 创建渲染表面和播放器
surface = pypag.PAGSurface.MakeOffscreen(pag.width(), pag.height())
player = pypag.PAGPlayer()
player.setComposition(pag)
player.setSurface(surface)

# 渲染第一帧
player.setProgress(0.0)
player.flush()

# 读取像素
pixels = surface.readPixels()  # 返回 RGBA bytes
```

### 2. 替换文本

```python
import pypag

# 加载文件
pag = pypag.PAGFile.Load('text1.pag')

# 检查可替换文本
num_texts = pag.numTexts()
print(f"可替换文本数: {num_texts}")

# 获取原始文本数据
text_data = pag.getTextData(0)
print(f"原文本: {text_data.text}")

# 创建新文本
new_text = pypag.TextDocument()
new_text.text = "新文本"
new_text.fontSize = 120
new_text.fillColor = pypag.Color()
new_text.fillColor.red = 255
new_text.fillColor.green = 0
new_text.fillColor.blue = 0

# 替换
pag.replaceText(0, new_text)
```

### 3. 替换图片

```python
import pypag

# 加载 PAG 文件
pag = pypag.PAGFile.Load('file.pag')

# 检查可替换图片
num_images = pag.numImages()
print(f"可替换图片数: {num_images}")

# 从文件加载图片
image = pypag.PAGImage.FromPath('new_image.png')

# 替换图片
pag.replaceImage(0, image)

# 或者按层名称替换
pag.replaceImageByName("图片层", image)
```

### 4. 查看层结构

```python
import pypag

pag = pypag.PAGFile.Load('file.pag')

# 获取所有层
for i in range(pag.numChildren()):
    layer = pag.getLayerAt(i)
    print(f"[{i}] {layer.layerName()} - {layer.layerType()}")
    print(f"    可见: {layer.visible()}")
    print(f"    时长: {layer.duration() / 1000000:.2f}秒")

# 按名称查找层
layers = pag.getLayersByName("文本层")
for layer in layers:
    print(f"找到: {layer.layerName()}")
```

### 5. 渲染完整动画

```python
import pypag

pag = pypag.PAGFile.Load('file.pag')
surface = pypag.PAGSurface.MakeOffscreen(pag.width(), pag.height())
player = pypag.PAGPlayer()
player.setComposition(pag)
player.setSurface(surface)

# 计算总帧数
duration_sec = pag.duration() / 1000000.0
frame_rate = pag.frameRate()
total_frames = int(duration_sec * frame_rate)

# 渲染每一帧
for frame_idx in range(total_frames):
    progress = frame_idx / max(1, total_frames - 1)
    player.setProgress(progress)
    player.flush()
    
    # 读取像素并保存
    pixels = surface.readPixels()
    # ... 保存 pixels 为图片
```

## 📋 测试程序说明

### `pag_test.py` - 主测试程序

```bash
# 播放并渲染
python pag_test.py <pag_file> play

# 编辑文本和图片
python pag_test.py <pag_file> edit

# 查看详细信息
python pag_test.py <pag_file> inspect

# 运行所有测试
python pag_test.py <pag_file> all
```

### 示例

```bash
# 播放 like.pag
python pag_test.py ../assets/like.pag play

# 编辑 text1.pag 的文本
python pag_test.py ../assets/text1.pag edit

# 查看 particle_video.pag 结构
python pag_test.py ../assets/particle_video.pag inspect
```

## 📁 输出文件

### 渲染的图片
- `./frames/` - 播放渲染的帧
- `./edited_preview/` - 编辑后的预览帧
- `./output_*/` - 测试输出

### 图片格式
- PPM (Portable Pixmap) - 未压缩的 RGB 图片格式
- 可用 ImageMagick 转换：
  ```bash
  # 转换为 PNG
  convert frame_0000.ppm frame_0000.png
  
  # 合成 GIF
  convert -delay 4 frames/frame_*.ppm output.gif
  
  # 合成视频
  ffmpeg -framerate 24 -i frames/frame_%04d.ppm -c:v libx264 output.mp4
  ```

## 🛠️ 编译和安装

### 首次编译

```bash
cd /Users/liujh/work/pag/libpag/python

# 激活虚拟环境
source venv/bin/activate

# 编译
./build.sh
```

### 重新编译

```bash
# 如果修改了绑定代码，重新编译
./build.sh
```

## ✅ 测试结果

### 测试文件
1. ✅ `like.pag` - 244x244, 24fps, 1.17秒
2. ✅ `text1.pag` - 1280x721, 30fps, 0.43秒（含可编辑文本）
3. ✅ `particle_video.pag` - 405x720, 24fps, 10秒

### 功能测试
- ✅ 加载 PAG 文件
- ✅ 渲染动画帧
- ✅ 替换文本内容和样式
- ✅ 查看层结构
- ✅ 获取可编辑索引
- ✅ 读取像素数据
- ✅ 保存为图片

## 📝 API 文档

### PAGFile
- `Load(path)` - 加载 PAG 文件
- `width()` - 获取宽度
- `height()` - 获取高度
- `duration()` - 获取时长（微秒）
- `frameRate()` - 获取帧率
- `numTexts()` - 可替换文本数
- `numImages()` - 可替换图片数
- `numVideos()` - 视频组合数
- `getTextData(index)` - 获取文本数据
- `replaceText(index, textData)` - 替换文本
- `replaceImage(index, image)` - 替换图片
- `replaceImageByName(name, image)` - 按名称替换图片
- `getEditableIndices(layerType)` - 获取可编辑索引
- `numChildren()` - 子层数量
- `getLayerAt(index)` - 获取指定层
- `getLayersByName(name)` - 按名称查找层

### PAGSurface
- `MakeOffscreen(width, height)` - 创建离屏表面
- `width()` - 获取宽度
- `height()` - 获取高度
- `readPixels()` - 读取像素（返回 RGBA bytes）
- `clearAll()` - 清空表面
- `freeCache()` - 释放缓存

### PAGPlayer
- `setComposition(pag)` - 设置组合
- `setSurface(surface)` - 设置表面
- `setProgress(progress)` - 设置进度（0.0-1.0）
- `flush()` - 刷新渲染
- `getProgress()` - 获取当前进度
- `duration()` - 获取时长
- `prepare()` - 预准备
- `setCacheEnabled(enabled)` - 启用缓存
- `setVideoEnabled(enabled)` - 启用视频

### PAGImage
- `FromPath(path)` - 从文件加载
- `FromBytes(data)` - 从字节加载
- `width()` - 获取宽度
- `height()` - 获取高度

### TextDocument
- `text` - 文本内容
- `fontSize` - 字号
- `fillColor` - 填充颜色
- `strokeColor` - 描边颜色

## 🎯 下一步可以实现

1. ⏳ 保存编辑后的 PAG 文件（需要 libpag 导出接口）
2. ⏳ 视频替换功能
3. ⏳ 音频处理
4. ⏳ 更多图片格式支持（PNG、JPEG）
5. ⏳ 直接渲染为 PNG/JPEG
6. ⏳ Web 界面展示

## 📄 文件结构

```
python/
├── build.sh              # 编译脚本
├── pag_test.py          # 主测试程序 ⭐
├── test_full.py         # 完整功能测试
├── test_pypag.py        # 基础测试
├── CMakeLists.txt       # CMake 配置
├── src/
│   ├── pypag_simple.cpp          # 主绑定文件
│   └── bindings/
│       ├── pag_file_simple.cpp   # PAGFile 绑定
│       ├── pag_surface.cpp       # PAGSurface 绑定
│       ├── pag_player.cpp        # PAGPlayer 绑定
│       └── pag_image.cpp         # PAGImage 绑定
├── examples/
│   ├── render_to_image.py   # 单帧渲染示例
│   └── render_frames.py     # 帧序列渲染示例
└── venv/                # Python 虚拟环境
```

## 🎉 总结

pypag 模块已成功实现所有核心功能：

1. ✅ **加载和播放** - 可以加载 PAG 文件并渲染动画
2. ✅ **文本编辑** - 可以替换文本内容、修改字号和颜色
3. ✅ **图片替换** - 可以替换图片（接口已实现）
4. ✅ **层结构** - 可以查看和操作层结构
5. ✅ **像素输出** - 可以读取渲染结果为像素数据

所有测试均通过！🎊

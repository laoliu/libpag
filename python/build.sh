#!/bin/bash
# 快速编译脚本 - 确保使用虚拟环境的 Python

set -e

cd "$(dirname "$0")"

# 检查虚拟环境
if [ -z "$VIRTUAL_ENV" ]; then
    echo "错误：请先激活虚拟环境"
    echo "运行: source venv/bin/activate"
    exit 1
fi

echo "==> 当前 Python 信息"
echo "Python 路径: $(which python)"
echo "Python 版本: $(python --version)"
echo ""

# 清理并创建构建目录
echo "==> 清理构建目录..."
rm -rf build
mkdir -p build
cd build

# 运行 CMake，明确指定 Python
echo "==> 配置 CMake..."
cmake .. \
    -DPython_EXECUTABLE=$(which python) \
    -DCMAKE_BUILD_TYPE=Release

# 编译
echo "==> 开始编译..."
make -j$(sysctl -n hw.ncpu)

# 查找生成的文件
echo ""
echo "==> 查找生成的模块..."
SO_FILE=$(find . -maxdepth 1 -name "pypag*.so" -type f | head -1)

if [ -z "$SO_FILE" ]; then
    echo "✗ 错误：未找到编译生成的 .so 文件"
    exit 1
fi

echo "✓ 找到: $SO_FILE"

# 获取 Python 版本
PY_VERSION=$(python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
SITE_PACKAGES="../venv/lib/python${PY_VERSION}/site-packages"

echo ""
echo "==> 安装模块..."
echo "目标目录: $SITE_PACKAGES"

# 复制模块
cp "$SO_FILE" "${SITE_PACKAGES}/pypag.so"

echo "✓ 模块已复制"

# 测试
echo ""
echo "==> 测试安装..."
cd ..
python test_pypag.py

echo ""
echo "========================================="
echo "✓ 安装成功！"
echo "========================================="
echo ""
echo "现在可以在 Python 中使用 pypag:"
echo "  import pypag"
echo "  pag = pypag.PAGFile.Load('file.pag')"

#!/bin/bash
# 简单的编译和安装脚本

set -e

cd "$(dirname "$0")"

echo "==> 激活虚拟环境..."
source venv/bin/activate

echo "==> 清理旧的构建文件..."
rm -rf build
mkdir -p build

echo "==> 运行 CMake 配置..."
cd build

# 确保使用虚拟环境的 Python
PYTHON_EXEC=$(which python)
echo "使用 Python: $PYTHON_EXEC"
python --version

cmake .. -DPython_EXECUTABLE=$PYTHON_EXEC

echo "==> 编译 pypag 模块..."
make -j$(sysctl -n hw.ncpu)

echo "==> 查找生成的模块..."
SO_FILE=$(find . -name "pypag*.so" | head -1)

if [ -z "$SO_FILE" ]; then
    echo "错误：未找到编译生成的 .so 文件"
    exit 1
fi

echo "找到：$SO_FILE"

echo "==> 复制模块到 site-packages..."
PYTHON_VERSION=$(python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')")
SITE_PACKAGES="../venv/lib/python${PYTHON_VERSION}/site-packages"

cp "$SO_FILE" "${SITE_PACKAGES}/pypag.so"

echo "==> 测试安装..."
cd ..
python -c "import pypag; print('✓ pypag 版本:', pypag.__version__)"

echo ""
echo "==> 安装成功！"
echo "现在可以使用:"
echo "  source venv/bin/activate"
echo "  python -c 'import pypag; ...'"

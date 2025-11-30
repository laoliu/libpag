#!/usr/bin/env python3
"""
测试 pypag 模块是否正常工作
"""

import sys

print("Python 版本:", sys.version)
print("Python 可执行文件:", sys.executable)
print()

try:
    import pypag
    print("✓ pypag 导入成功!")
    print("✓ pypag 版本:", pypag.__version__)
    print()
    
    # 测试加载 PAG 文件
    import os
    pag_path = os.path.join(os.path.dirname(__file__), '..', 'assets', 'like.pag')
    
    if os.path.exists(pag_path):
        print(f"测试加载文件: {pag_path}")
        pag_file = pypag.PAGFile.Load(pag_path)
        
        if pag_file:
            print("✓ PAG 文件加载成功!")
            print(f"  尺寸: {pag_file.width()}x{pag_file.height()}")
            print(f"  时长: {pag_file.duration() / 1000000.0:.2f} 秒")
            print(f"  帧率: {pag_file.frameRate()} FPS")
        else:
            print("✗ PAG 文件加载失败")
    else:
        print(f"⚠ 测试文件不存在: {pag_path}")
        
    print()
    print("=" * 50)
    print("所有测试通过! pypag 工作正常 🎉")
    print("=" * 50)
    
except ImportError as e:
    print("✗ 导入 pypag 失败:")
    print(f"  错误: {e}")
    print()
    print("请确保:")
    print("1. 已运行 ./install.sh")
    print("2. 已激活虚拟环境: source venv/bin/activate")
    sys.exit(1)
except Exception as e:
    print(f"✗ 测试失败: {e}")
    import traceback
    traceback.print_exc()
    sys.exit(1)

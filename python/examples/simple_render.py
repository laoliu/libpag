#!/usr/bin/env python3
"""
简单的 PAG 文件渲染示例
这个脚本演示如何加载 PAG 文件并渲染为图片
"""

import sys
import pypag
try:
    from PIL import Image
    import numpy as np
except ImportError:
    print("请先安装 Pillow: pip install Pillow")
    sys.exit(1)


def render_pag_to_image(pag_path, output_path, progress=0.0):
    """
    渲染 PAG 文件到图片
    
    Args:
        pag_path: PAG 文件路径
        output_path: 输出图片路径
        progress: 渲染进度 (0.0-1.0)
    """
    # 加载 PAG 文件
    print(f"正在加载 PAG 文件: {pag_path}")
    pag_file = pypag.PAGFile.Load(pag_path)
    
    if pag_file is None:
        print(f"错误: 无法加载 PAG 文件")
        return False
    
    # 打印文件信息
    print(f"文件信息:")
    print(f"  尺寸: {pag_file.width()} x {pag_file.height()}")
    print(f"  时长: {pag_file.duration() / 1000000.0:.2f} 秒")
    print(f"  帧率: {pag_file.frameRate()} FPS")
    print(f"  可编辑文本数: {pag_file.numTexts()}")
    print(f"  可替换图片数: {pag_file.numImages()}")
    
    # 创建离屏渲染表面
    width = pag_file.width()
    height = pag_file.height()
    surface = pypag.PAGSurface.MakeOffscreen(width, height)
    
    if surface is None:
        print("错误: 无法创建渲染表面")
        return False
    
    # 创建播放器
    player = pypag.PAGPlayer()
    player.setSurface(surface)
    player.setComposition(pag_file)
    
    # 设置进度
    player.setProgress(progress)
    
    # 渲染
    print(f"正在渲染 (进度: {progress * 100:.1f}%)...")
    if not player.flush():
        print("错误: 渲染失败")
        return False
    
    # 读取像素数据
    pixels = surface.readPixels()
    
    # 转换为 PIL Image 并保存
    # pixels 是 RGBA 格式的 numpy 数组
    image = Image.fromarray(pixels, 'RGBA')
    image.save(output_path)
    print(f"已保存到: {output_path}")
    
    return True


def render_animation_frames(pag_path, output_dir, num_frames=10):
    """
    渲染 PAG 动画的多个帧
    
    Args:
        pag_path: PAG 文件路径
        output_dir: 输出目录
        num_frames: 要渲染的帧数
    """
    import os
    os.makedirs(output_dir, exist_ok=True)
    
    for i in range(num_frames):
        progress = i / (num_frames - 1) if num_frames > 1 else 0.0
        output_path = os.path.join(output_dir, f"frame_{i:04d}.png")
        render_pag_to_image(pag_path, output_path, progress)


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("用法:")
        print(f"  {sys.argv[0]} <pag文件> <输出图片> [进度0-1]")
        print(f"  {sys.argv[0]} <pag文件> <输出目录> --frames <帧数>")
        print()
        print("示例:")
        print(f"  {sys.argv[0]} test.pag output.png 0.5")
        print(f"  {sys.argv[0]} test.pag output_dir --frames 30")
        sys.exit(1)
    
    pag_path = sys.argv[1]
    output = sys.argv[2]
    
    if len(sys.argv) > 3 and sys.argv[3] == "--frames":
        num_frames = int(sys.argv[4]) if len(sys.argv) > 4 else 10
        render_animation_frames(pag_path, output, num_frames)
    else:
        progress = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0
        render_pag_to_image(pag_path, output, progress)

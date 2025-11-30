#!/usr/bin/env python3
"""
PAG 文本替换示例
演示如何替换 PAG 文件中的可编辑文本
"""

import sys
import pypag


def list_texts(pag_path):
    """列出 PAG 文件中的所有可编辑文本"""
    pag_file = pypag.PAGFile.Load(pag_path)
    
    if pag_file is None:
        print(f"错误: 无法加载 PAG 文件 {pag_path}")
        return
    
    num_texts = pag_file.numTexts()
    print(f"找到 {num_texts} 个可编辑文本:")
    print()
    
    for i in range(num_texts):
        text_data = pag_file.getTextData(i)
        print(f"文本 #{i}:")
        print(f"  内容: {text_data.text}")
        print(f"  字体: {text_data.fontFamily}")
        print(f"  大小: {text_data.fontSize}")
        print()


def replace_text(pag_path, index, new_text, output_path):
    """替换文本并渲染"""
    try:
        from PIL import Image
    except ImportError:
        print("请先安装 Pillow: pip install Pillow")
        return
    
    # 加载文件
    pag_file = pypag.PAGFile.Load(pag_path)
    if pag_file is None:
        print(f"错误: 无法加载 PAG 文件")
        return
    
    # 替换文本
    print(f"替换文本 #{index} 为: {new_text}")
    pag_file.replaceText(index, new_text)
    
    # 创建表面和播放器
    surface = pypag.PAGSurface.MakeOffscreen(pag_file.width(), pag_file.height())
    player = pypag.PAGPlayer()
    player.setSurface(surface)
    player.setComposition(pag_file)
    
    # 渲染
    player.setProgress(0.5)  # 渲染中间帧
    player.flush()
    
    # 保存
    pixels = surface.readPixels()
    image = Image.fromarray(pixels, 'RGBA')
    image.save(output_path)
    print(f"已保存到: {output_path}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法:")
        print(f"  列出文本: {sys.argv[0]} <pag文件> --list")
        print(f"  替换文本: {sys.argv[0]} <pag文件> <索引> <新文本> <输出图片>")
        print()
        print("示例:")
        print(f"  {sys.argv[0]} test.pag --list")
        print(f"  {sys.argv[0]} test.pag 0 'Hello World' output.png")
        sys.exit(1)
    
    pag_path = sys.argv[1]
    
    if len(sys.argv) > 2 and sys.argv[2] == "--list":
        list_texts(pag_path)
    elif len(sys.argv) >= 5:
        index = int(sys.argv[2])
        new_text = sys.argv[3]
        output_path = sys.argv[4]
        replace_text(pag_path, index, new_text, output_path)
    else:
        print("参数错误，使用 --help 查看帮助")

#!/usr/bin/env python3
"""
PAG 测试页面 - 加载、播放和编辑 PAG 文件

功能：
1. 加载并播放 PAG 动画，渲染为图片序列
2. 替换 PAG 中的文本、图片等元素
3. 查看 PAG 文件结构和层信息
"""

import pypag
import sys
import os
from pathlib import Path


def save_as_ppm(pixels: bytes, width: int, height: int, output_path: str):
    """将 RGBA 像素数据保存为 PPM 格式图片"""
    with open(output_path, 'wb') as f:
        f.write(f'P6\n{width} {height}\n255\n'.encode())
        for i in range(0, len(pixels), 4):
            f.write(pixels[i:i+3])


def play_pag(pag_path: str, output_dir: str = "./frames", num_frames: int = 10):
    """
    播放 PAG 文件并渲染为帧序列
    
    Args:
        pag_path: PAG 文件路径
        output_dir: 输出目录
        num_frames: 要渲染的帧数
    """
    print("\n" + "=" * 70)
    print(f"播放 PAG: {Path(pag_path).name}")
    print("=" * 70)
    
    # 加载文件
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print("✗ 加载失败")
        return False
    
    width = pag_file.width()
    height = pag_file.height()
    duration = pag_file.duration() / 1000000.0
    frame_rate = pag_file.frameRate()
    total_frames = int(duration * frame_rate)
    
    print(f"✓ 文件信息:")
    print(f"  尺寸: {width}x{height}")
    print(f"  时长: {duration:.2f} 秒")
    print(f"  帧率: {frame_rate} fps")
    print(f"  总帧数: {total_frames}")
    
    # 创建渲染表面和播放器
    surface = pypag.PAGSurface.MakeOffscreen(width, height)
    if not surface:
        print("✗ 无法创建渲染表面")
        return False
    
    player = pypag.PAGPlayer()
    player.setComposition(pag_file)
    player.setSurface(surface)
    
    # 创建输出目录
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # 渲染帧
    frames_to_render = min(num_frames, total_frames)
    print(f"\n渲染 {frames_to_render} 帧到: {output_dir}")
    
    for i in range(frames_to_render):
        progress = i / max(1, total_frames - 1) if total_frames > 1 else 0.0
        player.setProgress(progress)
        player.flush()
        
        pixels = surface.readPixels()
        if pixels:
            output_file = Path(output_dir) / f"frame_{i:04d}.ppm"
            save_as_ppm(pixels, width, height, str(output_file))
            
            if (i + 1) % 5 == 0 or i == frames_to_render - 1:
                print(f"  进度: {i+1}/{frames_to_render} ({(i+1)/frames_to_render*100:.0f}%)")
    
    print(f"✓ 完成！帧已保存到: {output_dir}")
    print(f"\n提示: 使用 ImageMagick 合成 GIF:")
    print(f"  convert -delay {int(100/frame_rate)} {output_dir}/frame_*.ppm output.gif")
    
    return True


def edit_pag(pag_path: str):
    """
    编辑 PAG 文件 - 替换文本和图片
    
    Args:
        pag_path: PAG 文件路径
    """
    print("\n" + "=" * 70)
    print(f"编辑 PAG: {Path(pag_path).name}")
    print("=" * 70)
    
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print("✗ 加载失败")
        return False
    
    # 文件信息
    num_texts = pag_file.numTexts()
    num_images = pag_file.numImages()
    num_videos = pag_file.numVideos()
    num_children = pag_file.numChildren()
    
    print(f"\n✓ 文件信息:")
    print(f"  可替换文本: {num_texts}")
    print(f"  可替换图片: {num_images}")
    print(f"  视频组合: {num_videos}")
    print(f"  层数量: {num_children}")
    
    # 列出所有层
    if num_children > 0:
        print(f"\n层列表:")
        for i in range(num_children):
            layer = pag_file.getLayerAt(i)
            layer_type = layer.layerType()
            layer_name = layer.layerName()
            visible = "✓" if layer.visible() else "✗"
            print(f"  [{i}] {visible} {layer_name} ({layer_type})")
    
    # 替换文本
    if num_texts > 0:
        print(f"\n替换文本:")
        text_indices = pag_file.getEditableIndices(pypag.LayerType.Text)
        
        for idx in text_indices:
            text_data = pag_file.getTextData(idx)
            if text_data:
                print(f"\n  文本索引 {idx}:")
                print(f"    原文本: '{text_data.text}'")
                print(f"    原字号: {text_data.fontSize}")
                print(f"    原颜色: RGB({text_data.fillColor.red}, {text_data.fillColor.green}, {text_data.fillColor.blue})")
                
                # 创建新文本
                new_text = pypag.TextDocument()
                new_text.text = f"新文本-{idx}"
                new_text.fontSize = text_data.fontSize * 1.5
                new_text.fillColor = pypag.Color()
                new_text.fillColor.red = 255
                new_text.fillColor.green = 100
                new_text.fillColor.blue = 0
                
                pag_file.replaceText(idx, new_text)
                print(f"    ✓ 已替换为: '{new_text.text}'")
                print(f"    新字号: {new_text.fontSize}")
                print(f"    新颜色: 橙色")
    
    # 可替换图片信息
    if num_images > 0:
        print(f"\n可替换图片:")
        image_indices = pag_file.getEditableIndices(pypag.LayerType.Image)
        
        for idx in image_indices:
            layers = pag_file.getLayersByEditableIndex(idx, pypag.LayerType.Image)
            print(f"\n  图片索引 {idx}:")
            print(f"    关联层数: {len(layers)}")
            for layer in layers:
                print(f"      - {layer.layerName()}")
            print(f"    提示: 使用 pypag.PAGImage.FromPath('image.png') 替换图片")
    
    print(f"\n✓ 编辑完成！")
    
    # 渲染编辑后的结果
    print(f"\n渲染编辑后的预览...")
    play_pag_edited(pag_file, "./edited_preview", 5)
    
    return True


def play_pag_edited(pag_file, output_dir: str, num_frames: int = 5):
    """渲染已编辑的 PAG 文件"""
    width = pag_file.width()
    height = pag_file.height()
    duration = pag_file.duration() / 1000000.0
    frame_rate = pag_file.frameRate()
    total_frames = int(duration * frame_rate)
    
    surface = pypag.PAGSurface.MakeOffscreen(width, height)
    player = pypag.PAGPlayer()
    player.setComposition(pag_file)
    player.setSurface(surface)
    
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    frames_to_render = min(num_frames, total_frames)
    for i in range(frames_to_render):
        progress = i / max(1, total_frames - 1) if total_frames > 1 else 0.0
        player.setProgress(progress)
        player.flush()
        
        pixels = surface.readPixels()
        if pixels:
            output_file = Path(output_dir) / f"edited_{i:04d}.ppm"
            save_as_ppm(pixels, width, height, str(output_file))
    
    print(f"  ✓ 预览帧已保存到: {output_dir}")


def inspect_pag(pag_path: str):
    """
    查看 PAG 文件详细信息
    
    Args:
        pag_path: PAG 文件路径
    """
    print("\n" + "=" * 70)
    print(f"检查 PAG: {Path(pag_path).name}")
    print("=" * 70)
    
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print("✗ 加载失败")
        return False
    
    # 基本信息
    print(f"\n基本信息:")
    print(f"  文件路径: {pag_file.path()}")
    print(f"  尺寸: {pag_file.width()}x{pag_file.height()}")
    print(f"  时长: {pag_file.duration() / 1000000.0:.2f} 秒")
    print(f"  帧率: {pag_file.frameRate()} fps")
    
    # 可编辑内容
    print(f"\n可编辑内容:")
    print(f"  文本: {pag_file.numTexts()}")
    print(f"  图片: {pag_file.numImages()}")
    print(f"  视频: {pag_file.numVideos()}")
    
    # 层结构
    print(f"\n层结构:")
    
    def print_layers(composition, indent=0):
        for i in range(composition.numChildren()):
            layer = composition.getLayerAt(i)
            prefix = "  " * indent
            layer_type = layer.layerType()
            layer_name = layer.layerName()
            visible = "✓" if layer.visible() else "✗"
            duration_sec = layer.duration() / 1000000.0
            
            print(f"{prefix}[{i}] {visible} {layer_name}")
            print(f"{prefix}    类型: {layer_type}")
            print(f"{prefix}    时长: {duration_sec:.2f}s")
            print(f"{prefix}    帧率: {layer.frameRate()}fps")
    
    print_layers(pag_file)
    
    return True


def main():
    """主程序"""
    print("=" * 70)
    print(" PAG Python 测试页面")
    print("=" * 70)
    print("\n功能:")
    print("  1. 播放 PAG 动画并渲染")
    print("  2. 编辑 PAG 文件（替换文本/图片）")
    print("  3. 查看 PAG 文件详细信息")
    
    if len(sys.argv) < 2:
        print("\n用法:")
        print("  python pag_test.py <pag_file> [命令]")
        print("\n命令:")
        print("  play   - 播放并渲染（默认）")
        print("  edit   - 编辑文本和图片")
        print("  inspect - 查看详细信息")
        print("  all    - 运行所有测试")
        print("\n示例:")
        print("  python pag_test.py ../assets/text1.pag play")
        print("  python pag_test.py ../assets/text1.pag edit")
        print("  python pag_test.py ../assets/like.pag all")
        return 1
    
    pag_path = sys.argv[1]
    command = sys.argv[2] if len(sys.argv) > 2 else "play"
    
    if not Path(pag_path).exists():
        print(f"\n✗ 错误: 文件不存在: {pag_path}")
        return 1
    
    if command == "play":
        play_pag(pag_path, num_frames=10)
    elif command == "edit":
        edit_pag(pag_path)
    elif command == "inspect":
        inspect_pag(pag_path)
    elif command == "all":
        inspect_pag(pag_path)
        play_pag(pag_path, num_frames=5)
        edit_pag(pag_path)
    else:
        print(f"\n✗ 未知命令: {command}")
        return 1
    
    print("\n" + "=" * 70)
    print(" 测试完成！")
    print("=" * 70)
    return 0


if __name__ == "__main__":
    sys.exit(main())

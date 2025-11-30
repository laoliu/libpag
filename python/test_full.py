#!/usr/bin/env python3
"""
完整功能测试：加载、播放、编辑 PAG 文件
"""

import pypag
import sys
import os
from pathlib import Path


def test_playback(pag_path: str, output_dir: str = "./output"):
    """测试 PAG 播放和渲染"""
    print("=" * 60)
    print("测试 1: PAG 播放和渲染")
    print("=" * 60)
    
    # 加载 PAG 文件
    print(f"\n加载文件: {pag_path}")
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print("✗ 加载失败")
        return False
    
    width = pag_file.width()
    height = pag_file.height()
    duration = pag_file.duration() / 1000000.0  # 转换为秒
    frame_rate = pag_file.frameRate()
    total_frames = int(duration * frame_rate)
    
    print(f"✓ 加载成功")
    print(f"  尺寸: {width}x{height}")
    print(f"  时长: {duration:.2f} 秒")
    print(f"  帧率: {frame_rate} fps")
    print(f"  总帧数: {total_frames}")
    
    # 创建渲染表面
    print(f"\n创建渲染表面...")
    surface = pypag.PAGSurface.MakeOffscreen(width, height)
    if not surface:
        print("✗ 创建表面失败")
        return False
    
    # 创建播放器
    print("创建播放器...")
    player = pypag.PAGPlayer()
    player.setComposition(pag_file)
    player.setSurface(surface)
    
    # 创建输出目录
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    # 渲染几帧作为示例
    test_frames = [0, total_frames // 4, total_frames // 2, total_frames * 3 // 4, total_frames - 1]
    print(f"\n渲染测试帧到 {output_dir}...")
    
    for frame_idx in test_frames:
        if frame_idx >= total_frames:
            continue
            
        progress = frame_idx / max(1, total_frames - 1) if total_frames > 1 else 0.0
        player.setProgress(progress)
        player.flush()
        
        pixels = surface.readPixels()
        if pixels:
            output_file = Path(output_dir) / f"frame_{frame_idx:04d}.ppm"
            save_as_ppm(pixels, width, height, str(output_file))
            print(f"  ✓ 帧 {frame_idx} ({progress*100:.1f}%) -> {output_file.name}")
    
    print(f"\n✓ 播放测试完成！帧已保存到: {output_dir}")
    return True


def test_editing(pag_path: str, output_path: str = "./edited.pag"):
    """测试 PAG 编辑功能"""
    print("\n" + "=" * 60)
    print("测试 2: PAG 编辑功能")
    print("=" * 60)
    
    # 加载文件
    print(f"\n加载文件: {pag_path}")
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print("✗ 加载失败")
        return False
    
    print(f"✓ 加载成功")
    
    # 检查可编辑的文本
    num_texts = pag_file.numTexts()
    num_images = pag_file.numImages()
    num_videos = pag_file.numVideos()
    
    print(f"\n文件信息:")
    print(f"  可替换文本数: {num_texts}")
    print(f"  可替换图片数: {num_images}")
    print(f"  视频组合数: {num_videos}")
    
    # 列出所有层
    num_children = pag_file.numChildren()
    print(f"  子层数量: {num_children}")
    
    if num_children > 0:
        print(f"\n层列表:")
        for i in range(num_children):
            layer = pag_file.getLayerAt(i)
            layer_type = layer.layerType()
            layer_name = layer.layerName()
            print(f"  [{i}] {layer_name} - 类型: {layer_type}")
    
    # 尝试替换文本
    if num_texts > 0:
        print(f"\n替换文本:")
        text_indices = pag_file.getEditableIndices(pypag.LayerType.Text)
        print(f"  可编辑文本索引: {text_indices}")
        
        for idx in text_indices[:3]:  # 只处理前3个
            try:
                text_data = pag_file.getTextData(idx)
                if text_data:
                    old_text = text_data.text
                    print(f"  [索引 {idx}] 原文本: '{old_text}'")
                    
                    # 创建新的文本数据
                    new_text_data = pypag.TextDocument()
                    new_text_data.text = f"新文本 {idx}"
                    new_text_data.fillColor = pypag.Color()
                    new_text_data.fillColor.red = 255
                    new_text_data.fillColor.green = 0
                    new_text_data.fillColor.blue = 0
                    new_text_data.fontSize = text_data.fontSize * 1.2
                    
                    pag_file.replaceText(idx, new_text_data)
                    print(f"  [索引 {idx}] 已替换为: '{new_text_data.text}' (红色, 字号放大20%)")
            except Exception as e:
                print(f"  [索引 {idx}] 替换失败: {e}")
    
    # 尝试替换图片
    if num_images > 0:
        print(f"\n可替换图片:")
        image_indices = pag_file.getEditableIndices(pypag.LayerType.Image)
        print(f"  可编辑图片索引: {image_indices}")
        
        # 这里我们只是演示 API，实际替换需要提供图片文件
        for idx in image_indices[:3]:
            layers = pag_file.getLayersByEditableIndex(idx, pypag.LayerType.Image)
            print(f"  [索引 {idx}] 关联层数: {len(layers)}")
            for layer in layers:
                print(f"    - {layer.layerName()}")
    
    print(f"\n✓ 编辑测试完成！")
    print(f"\n注意: 实际保存功能需要 PAGFile 提供导出方法")
    return True


def save_as_ppm(pixels: bytes, width: int, height: int, output_path: str):
    """将 RGBA 像素数据保存为 PPM 格式图片"""
    with open(output_path, 'wb') as f:
        f.write(f'P6\n{width} {height}\n255\n'.encode())
        for i in range(0, len(pixels), 4):
            f.write(pixels[i:i+3])  # 只写入 RGB


def test_layer_inspection(pag_path: str):
    """测试层检查功能"""
    print("\n" + "=" * 60)
    print("测试 3: 层结构检查")
    print("=" * 60)
    
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        return False
    
    print(f"\n文件: {Path(pag_path).name}")
    print(f"尺寸: {pag_file.width()}x{pag_file.height()}")
    
    def print_layer_tree(composition, indent=0):
        """递归打印层树"""
        prefix = "  " * indent
        for i in range(composition.numChildren()):
            layer = composition.getLayerAt(i)
            layer_type = layer.layerType()
            layer_name = layer.layerName()
            visible = "✓" if layer.visible() else "✗"
            
            print(f"{prefix}[{i}] {visible} {layer_name} ({layer_type})")
            
            # 如果是文本层，显示文本
            if layer_type == pypag.LayerType.Text:
                try:
                    text_layer = pypag.PAGTextLayer.__class__(layer)  # 尝试转换
                    # 注意：实际使用中需要正确的类型转换
                except:
                    pass
    
    print(f"\n层结构:")
    print_layer_tree(pag_file)
    
    return True


def main():
    print("PAG Python 绑定 - 完整功能测试")
    print("=" * 60)
    
    # 测试文件
    test_files = [
        "../assets/like.pag",
        "../assets/text1.pag",
        "../assets/particle_video.pag"
    ]
    
    success_count = 0
    
    for pag_path in test_files:
        if not Path(pag_path).exists():
            print(f"\n跳过不存在的文件: {pag_path}")
            continue
        
        print(f"\n\n{'=' * 60}")
        print(f"测试文件: {pag_path}")
        print(f"{'=' * 60}")
        
        # 测试播放
        if test_playback(pag_path, f"./output_{Path(pag_path).stem}"):
            success_count += 1
        
        # 测试编辑
        if test_editing(pag_path):
            success_count += 1
        
        # 测试层检查
        if test_layer_inspection(pag_path):
            success_count += 1
    
    print(f"\n\n{'=' * 60}")
    print(f"测试完成！成功: {success_count}")
    print(f"{'=' * 60}")


if __name__ == "__main__":
    main()

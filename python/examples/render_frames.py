#!/usr/bin/env python3
"""
示例：将 PAG 动画渲染为多帧图片序列
"""

import pypag
import sys
from pathlib import Path


def save_as_ppm(pixels: bytes, width: int, height: int, output_path: str):
    """将 RGBA 像素数据保存为 PPM 格式图片"""
    with open(output_path, 'wb') as f:
        f.write(f'P6\n{width} {height}\n255\n'.encode())
        for i in range(0, len(pixels), 4):
            f.write(pixels[i:i+3])


def render_pag_frames(pag_path: str, output_dir: str, frame_count: int = 10):
    """
    将 PAG 文件渲染为帧序列
    
    Args:
        pag_path: PAG 文件路径
        output_dir: 输出目录
        frame_count: 要渲染的帧数
    """
    print(f"加载 PAG 文件: {pag_path}")
    
    # 加载 PAG 文件
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print(f"错误：无法加载 PAG 文件")
        return False
    
    width = pag_file.width()
    height = pag_file.height()
    frame_rate = pag_file.frameRate()
    duration = pag_file.duration() / 1000000.0
    total_frames = int(duration * frame_rate)
    
    print(f"\nPAG 信息:")
    print(f"  尺寸: {width}x{height}")
    print(f"  帧率: {frame_rate} fps")
    print(f"  时长: {duration:.2f} 秒")
    print(f"  总帧数: {total_frames}")
    
    # 确定实际渲染帧数
    frames_to_render = min(frame_count, total_frames)
    print(f"\n将渲染 {frames_to_render} 帧")
    
    # 创建输出目录
    output_path = Path(output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    
    # 创建渲染表面和播放器
    print(f"创建渲染表面 ({width}x{height})...")
    surface = pypag.PAGSurface.MakeOffscreen(width, height)
    if not surface:
        print("错误：无法创建渲染表面")
        return False
    
    player = pypag.PAGPlayer()
    player.setComposition(pag_file)
    player.setSurface(surface)
    
    # 渲染每一帧
    print("\n开始渲染...")
    for i in range(frames_to_render):
        progress = i / max(1, total_frames - 1) if total_frames > 1 else 0.0
        player.setProgress(progress)
        
        # 渲染
        player.flush()
        
        # 读取像素
        pixels = surface.readPixels(pypag.ColorType.RGBA_8888,
                                   pypag.AlphaType.Premultiplied)
        
        if pixels is None:
            print(f"警告：第 {i} 帧读取失败")
            continue
        
        # 保存
        output_file = output_path / f"frame_{i:04d}.ppm"
        save_as_ppm(pixels, width, height, str(output_file))
        
        # 进度显示
        percent = (i + 1) / frames_to_render * 100
        print(f"  [{i+1}/{frames_to_render}] {percent:.1f}% - {output_file.name}")
    
    print(f"\n✓ 完成！所有帧已保存到: {output_path}")
    print(f"\n可以使用 ImageMagick 合成 GIF:")
    print(f"  convert -delay {int(100/frame_rate)} {output_path}/frame_*.ppm output.gif")
    
    return True


def main():
    if len(sys.argv) < 2:
        print("用法: python render_frames.py <pag_file> [output_dir] [frame_count]")
        print("示例: python render_frames.py ../assets/like.pag ./frames 30")
        return 1
    
    pag_path = sys.argv[1]
    output_dir = sys.argv[2] if len(sys.argv) > 2 else "./frames"
    frame_count = int(sys.argv[3]) if len(sys.argv) > 3 else 10
    
    if not Path(pag_path).exists():
        print(f"错误：文件不存在: {pag_path}")
        return 1
    
    success = render_pag_frames(pag_path, output_dir, frame_count)
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
示例：将 PAG 动画渲染为图片
"""

import pypag
import sys
from pathlib import Path


def save_as_ppm(pixels: bytes, width: int, height: int, output_path: str):
    """将 RGBA 像素数据保存为 PPM 格式图片"""
    with open(output_path, 'wb') as f:
        # PPM header
        f.write(f'P6\n{width} {height}\n255\n'.encode())
        
        # 转换 RGBA 到 RGB (跳过 alpha 通道)
        for i in range(0, len(pixels), 4):
            f.write(pixels[i:i+3])  # 只写入 RGB


def render_pag_to_image(pag_path: str, output_path: str, progress: float = 0.0):
    """
    将 PAG 文件渲染为图片
    
    Args:
        pag_path: PAG 文件路径
        output_path: 输出图片路径
        progress: 渲染进度 (0.0 到 1.0)
    """
    print(f"加载 PAG 文件: {pag_path}")
    
    # 加载 PAG 文件
    pag_file = pypag.PAGFile.Load(pag_path)
    if not pag_file:
        print(f"错误：无法加载 PAG 文件: {pag_path}")
        return False
    
    width = pag_file.width()
    height = pag_file.height()
    frame_rate = pag_file.frameRate()
    duration = pag_file.duration() / 1000000.0  # 转换为秒
    
    print(f"PAG 信息:")
    print(f"  尺寸: {width}x{height}")
    print(f"  帧率: {frame_rate} fps")
    print(f"  时长: {duration:.2f} 秒")
    print(f"  渲染进度: {progress * 100:.1f}%")
    
    # 创建离屏渲染表面
    print(f"\n创建渲染表面 ({width}x{height})...")
    surface = pypag.PAGSurface.MakeOffscreen(width, height)
    if not surface:
        print("错误：无法创建渲染表面")
        return False
    
    # 创建播放器
    print("创建播放器...")
    player = pypag.PAGPlayer()
    player.setComposition(pag_file)
    player.setSurface(surface)
    
    # 设置渲染进度
    player.setProgress(progress)
    
    # 渲染
    print(f"渲染帧 {int(progress * duration * frame_rate)}...")
    if not player.flush():
        print("警告：内容未改变")
    
    # 读取像素数据
    print("读取像素数据...")
    pixels = surface.readPixels(pypag.ColorType.RGBA_8888, 
                               pypag.AlphaType.Premultiplied)
    
    if pixels is None:
        print("错误：无法读取像素数据")
        return False
    
    # 保存图片
    print(f"保存图片: {output_path}")
    save_as_ppm(pixels, width, height, output_path)
    
    print(f"✓ 渲染完成！")
    return True


def main():
    if len(sys.argv) < 2:
        print("用法: python render_to_image.py <pag_file> [output.ppm] [progress]")
        print("示例: python render_to_image.py ../assets/like.pag output.ppm 0.5")
        return 1
    
    pag_path = sys.argv[1]
    output_path = sys.argv[2] if len(sys.argv) > 2 else "output.ppm"
    progress = float(sys.argv[3]) if len(sys.argv) > 3 else 0.0
    
    if not Path(pag_path).exists():
        print(f"错误：文件不存在: {pag_path}")
        return 1
    
    success = render_pag_to_image(pag_path, output_path, progress)
    return 0 if success else 1


if __name__ == "__main__":
    sys.exit(main())

#!/usr/bin/env python3
"""
PAG Web 服务器
提供 PAG 文件的加载、编辑和渲染功能
"""

from flask import Flask, render_template, request, jsonify, send_file
from flask_cors import CORS
import pypag
import os
import json
import base64
from pathlib import Path
from io import BytesIO
import tempfile
from datetime import datetime

app = Flask(__name__)
CORS(app)

# 存储当前加载的 PAG 文件列表
pag_playlist = []
pag_cache = {}  # 缓存 PAG 对象


def save_as_png_bytes(pixels: bytes, width: int, height: int) -> bytes:
    """将 RGBA 像素转换为 PNG 格式的字节"""
    try:
        from PIL import Image
        import numpy as np
        
        # 将 bytes 转换为 numpy 数组
        pixel_array = np.frombuffer(pixels, dtype=np.uint8)
        pixel_array = pixel_array.reshape((height, width, 4))
        
        # 创建 PIL 图像
        img = Image.fromarray(pixel_array, 'RGBA')
        
        # 转换为 PNG bytes
        img_bytes = BytesIO()
        img.save(img_bytes, format='PNG')
        img_bytes.seek(0)
        
        return img_bytes.getvalue()
    except ImportError:
        # 如果没有 PIL，返回简单的 PPM 格式
        return save_as_ppm_bytes(pixels, width, height)


def save_as_ppm_bytes(pixels: bytes, width: int, height: int) -> bytes:
    """将 RGBA 像素转换为 PPM 格式的字节"""
    output = BytesIO()
    output.write(f'P6\n{width} {height}\n255\n'.encode())
    
    for i in range(0, len(pixels), 4):
        output.write(pixels[i:i+3])  # 只写入 RGB
    
    output.seek(0)
    return output.getvalue()


def render_pag_frame(pag_file, progress: float = 0.0, cache_item: dict = None) -> bytes:
    """渲染 PAG 文件的一帧为 PNG"""
    width = pag_file.width()
    height = pag_file.height()
    
    # 尝试复用缓存的 player 和 surface
    player = None
    surface = None
    
    if cache_item and 'player' in cache_item and 'surface' in cache_item:
        player = cache_item['player']
        surface = cache_item['surface']
    
    # 如果没有缓存，创建新的
    if not player or not surface:
        surface = pypag.PAGSurface.MakeOffscreen(width, height)
        if not surface:
            return None
        
        player = pypag.PAGPlayer()
        player.setComposition(pag_file)
        player.setSurface(surface)
        
        # 保存到缓存
        if cache_item is not None:
            cache_item['player'] = player
            cache_item['surface'] = surface
    
    # 设置进度并渲染
    player.setProgress(progress)
    player.flush()
    
    # 读取像素
    pixels = surface.readPixels()
    if not pixels:
        return None
    
    return save_as_png_bytes(pixels, width, height)


def get_pag_info(pag_file):
    """获取 PAG 文件信息"""
    info = {
        'width': pag_file.width(),
        'height': pag_file.height(),
        'duration': pag_file.duration() / 1000000.0,  # 转换为秒
        'frameRate': pag_file.frameRate(),
        'numTexts': pag_file.numTexts(),
        'numImages': pag_file.numImages(),
        'numVideos': pag_file.numVideos(),
        'numLayers': pag_file.numChildren(),
    }
    
    # 获取总帧数
    info['totalFrames'] = int(info['duration'] * info['frameRate'])
    
    # 获取可编辑文本信息
    text_info = []
    if info['numTexts'] > 0:
        text_indices = pag_file.getEditableIndices(pypag.LayerType.Text)
        for idx in text_indices:
            text_data = pag_file.getTextData(idx)
            if text_data:
                text_info.append({
                    'index': idx,
                    'text': text_data.text,
                    'fontSize': text_data.fontSize,
                    'fillColor': {
                        'r': text_data.fillColor.red,
                        'g': text_data.fillColor.green,
                        'b': text_data.fillColor.blue
                    }
                })
    info['texts'] = text_info
    
    # 获取可编辑图片信息
    image_info = []
    if info['numImages'] > 0:
        image_indices = pag_file.getEditableIndices(pypag.LayerType.Image)
        for idx in image_indices:
            layers = pag_file.getLayersByEditableIndex(idx, pypag.LayerType.Image)
            layer_names = [layer.layerName() for layer in layers]
            image_info.append({
                'index': idx,
                'layers': layer_names
            })
    info['images'] = image_info
    
    # 获取层信息
    layers = []
    for i in range(info['numLayers']):
        layer = pag_file.getLayerAt(i)
        layers.append({
            'index': i,
            'name': layer.layerName(),
            'type': str(layer.layerType()),
            'visible': layer.visible(),
            'duration': layer.duration() / 1000000.0
        })
    info['layers'] = layers
    
    return info


@app.route('/')
def index():
    """主页"""
    return render_template('index.html')


@app.route('/api/list-files', methods=['GET'])
def list_files():
    """列出可用的 PAG 文件"""
    assets_dir = Path(__file__).parent.parent.parent / 'assets'
    pag_files = []
    
    if assets_dir.exists():
        for pag_file in sorted(assets_dir.glob('*.pag')):
            pag_files.append({
                'name': pag_file.name,
                'path': str(pag_file),
                'size': pag_file.stat().st_size
            })
    
    return jsonify({'files': pag_files})


@app.route('/api/load', methods=['POST'])
def load_pag():
    """加载 PAG 文件"""
    data = request.json
    file_path = data.get('path')
    
    if not file_path or not os.path.exists(file_path):
        return jsonify({'error': '文件不存在'}), 400
    
    try:
        pag_file = pypag.PAGFile.Load(file_path)
        if not pag_file:
            return jsonify({'error': '无法加载 PAG 文件'}), 400
        
        # 生成唯一 ID
        pag_id = len(pag_playlist)
        
        # 缓存 PAG 对象
        pag_cache[pag_id] = {
            'pag': pag_file,
            'path': file_path,
            'name': Path(file_path).name,
            'edited': False,
            'edits': {}
        }
        
        # 添加到播放列表
        pag_playlist.append(pag_id)
        
        # 获取文件信息
        info = get_pag_info(pag_file)
        info['id'] = pag_id
        info['name'] = Path(file_path).name
        
        return jsonify(info)
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/playlist', methods=['GET'])
def get_playlist():
    """获取播放列表"""
    playlist_info = []
    for pag_id in pag_playlist:
        if pag_id in pag_cache:
            cache_item = pag_cache[pag_id]
            info = get_pag_info(cache_item['pag'])
            info['id'] = pag_id
            info['name'] = cache_item['name']
            info['edited'] = cache_item['edited']
            
            # 如果有已编辑的文本数据，替换为编辑后的值
            if 'edits' in cache_item and 'texts' in cache_item['edits']:
                for text_item in info['texts']:
                    idx = text_item['index']
                    if idx in cache_item['edits']['texts']:
                        edited = cache_item['edits']['texts'][idx]
                        if 'text' in edited and edited['text'] is not None:
                            text_item['text'] = edited['text']
                        if 'fontSize' in edited and edited['fontSize'] is not None:
                            text_item['fontSize'] = edited['fontSize']
                        if 'color' in edited and edited['color'] is not None:
                            text_item['fillColor'] = edited['color']
            
            # 如果有已替换的图片，添加替换信息
            if 'edits' in cache_item and 'images' in cache_item['edits']:
                for image_item in info['images']:
                    idx = image_item['index']
                    if idx in cache_item['edits']['images']:
                        edited = cache_item['edits']['images'][idx]
                        image_item['replaced'] = True
                        image_item['filename'] = edited.get('filename', '')
                        # 生成图片预览 URL
                        if 'path' in edited:
                            image_item['previewUrl'] = f'/api/preview-image/{pag_id}/{idx}'
            
            playlist_info.append(info)
    
    return jsonify({'playlist': playlist_info})


@app.route('/api/reorder', methods=['POST'])
def reorder_playlist():
    """重新排序播放列表"""
    data = request.json
    new_order = data.get('order', [])
    
    global pag_playlist
    pag_playlist = [id for id in new_order if id in pag_cache]
    
    return jsonify({'success': True})


@app.route('/api/preview-image/<int:pag_id>/<int:image_index>', methods=['GET'])
def preview_image(pag_id, image_index):
    """预览已替换的图片"""
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    cache_item = pag_cache[pag_id]
    if 'edits' not in cache_item or 'images' not in cache_item['edits']:
        return jsonify({'error': '没有替换图片'}), 404
    
    if image_index not in cache_item['edits']['images']:
        return jsonify({'error': '图片未被替换'}), 404
    
    image_path = cache_item['edits']['images'][image_index].get('path')
    if not image_path or not Path(image_path).exists():
        return jsonify({'error': '图片文件不存在'}), 404
    
    return send_file(image_path, mimetype='image/png')


@app.route('/api/remove/<int:pag_id>', methods=['DELETE'])
def remove_from_playlist(pag_id):
    """从播放列表中移除"""
    if pag_id in pag_playlist:
        pag_playlist.remove(pag_id)
    
    if pag_id in pag_cache:
        del pag_cache[pag_id]
    
    return jsonify({'success': True})


@app.route('/api/render/<int:pag_id>', methods=['GET'])
def render_frame(pag_id):
    """渲染指定 PAG 的一帧"""
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    progress = float(request.args.get('progress', 0.0))
    
    try:
        cache_item = pag_cache[pag_id]
        pag_file = cache_item['pag']
        
        # 渲染帧（传递 cache_item 以复用 player 和 surface）
        png_bytes = render_pag_frame(pag_file, progress, cache_item)
        
        if not png_bytes:
            return jsonify({'error': '渲染失败'}), 500
        
        # 返回 PNG 图片
        return send_file(
            BytesIO(png_bytes),
            mimetype='image/png',
            as_attachment=False
        )
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500
        
        # 返回 PNG 图片
        return send_file(
            BytesIO(png_bytes),
            mimetype='image/png',
            as_attachment=False
        )
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/preview/<int:pag_id>', methods=['GET'])
def preview_thumbnail(pag_id):
    """获取预览缩略图（第一帧）"""
    return render_frame(pag_id)


@app.route('/api/edit/text', methods=['POST'])
def edit_text():
    """编辑文本"""
    data = request.json
    pag_id = data.get('pag_id')
    text_index = data.get('index')
    new_text = data.get('text')
    font_size = data.get('fontSize')
    color = data.get('color', {})
    
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    try:
        cache_item = pag_cache[pag_id]
        pag_file = cache_item['pag']
        
        # 创建新文本数据
        text_data = pypag.TextDocument()
        text_data.text = new_text
        
        if font_size:
            text_data.fontSize = float(font_size)
        
        if color:
            text_data.fillColor = pypag.Color()
            text_data.fillColor.red = int(color.get('r', 255))
            text_data.fillColor.green = int(color.get('g', 255))
            text_data.fillColor.blue = int(color.get('b', 255))
        
        # 替换文本
        pag_file.replaceText(text_index, text_data)
        
        # 标记为已编辑
        cache_item['edited'] = True
        if 'texts' not in cache_item['edits']:
            cache_item['edits']['texts'] = {}
        cache_item['edits']['texts'][text_index] = {
            'text': new_text,
            'fontSize': font_size,
            'color': color
        }
        
        # 清除 player 和 surface 缓存，强制重新创建
        if 'player' in cache_item:
            del cache_item['player']
        if 'surface' in cache_item:
            del cache_item['surface']
        
        return jsonify({'success': True})
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/edit/image', methods=['POST'])
def edit_image():
    """编辑图片"""
    pag_id = int(request.form.get('pag_id'))
    image_index = int(request.form.get('index'))
    
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    if 'image' not in request.files:
        return jsonify({'error': '没有上传图片'}), 400
    
    try:
        cache_item = pag_cache[pag_id]
        pag_file = cache_item['pag']
        
        # 获取上传的图片
        image_file = request.files['image']
        
        # 使用 PIL 转换图片为标准 PNG 格式
        from PIL import Image
        import io
        
        # 读取并转换图片
        img = Image.open(image_file.stream)
        
        # 转换为 RGBA 模式
        if img.mode not in ('RGBA', 'RGB'):
            img = img.convert('RGBA')
        
        # 保存为标准 PNG
        with tempfile.NamedTemporaryFile(delete=False, suffix='.png') as tmp:
            img.save(tmp, format='PNG')
            tmp_path = tmp.name
        
        print(f"[图片编辑] 图片转换完成: {img.size}, 模式={img.mode}, 临时文件={tmp_path}")
        
        # 加载图片
        pag_image = pypag.PAGImage.FromPath(tmp_path)
        
        if not pag_image:
            os.unlink(tmp_path)
            return jsonify({'error': '无法加载图片'}), 400
        
        print(f"[图片编辑] PAGImage 创建成功: width={pag_image.width()}, height={pag_image.height()}")
        
        # 获取原始图片信息用于对比
        layers = pag_file.getLayersByEditableIndex(image_index, pypag.LayerType.Image)
        if layers:
            print(f"[图片编辑] 图层信息: {[layer.layerName() for layer in layers]}")
        
        # 替换图片
        result = pag_file.replaceImage(image_index, pag_image)
        print(f"[图片编辑] replaceImage 返回值: {result}")
        
        # 删除临时文件
        os.unlink(tmp_path)
        
        print(f"[图片编辑] 成功替换图片 index={image_index}, 文件={image_file.filename}")
        
        # 标记为已编辑
        cache_item['edited'] = True
        if 'images' not in cache_item['edits']:
            cache_item['edits']['images'] = {}
        cache_item['edits']['images'][image_index] = {
            'filename': image_file.filename,
            'path': tmp_path  # 保存路径用于重新应用
        }
        
        # 保存图片到持久化位置
        import shutil
        persist_dir = Path(__file__).parent / 'uploads'
        persist_dir.mkdir(exist_ok=True)
        persist_path = persist_dir / f"pag_{pag_id}_img_{image_index}_{image_file.filename}"
        
        # 重新保存图片
        img.save(persist_path, format='PNG')
        cache_item['edits']['images'][image_index]['path'] = str(persist_path)
        print(f"[图片编辑] 图片已保存到: {persist_path}")
        
        # 关键修复：重新加载 PAG 文件并重新应用所有编辑
        print(f"[图片编辑] 重新加载 PAG 文件以应用编辑")
        pag_file = pypag.PAGFile.Load(cache_item['path'])
        
        # 重新应用所有文本编辑
        if 'texts' in cache_item['edits']:
            for text_idx, text_edit in cache_item['edits']['texts'].items():
                text_data = pypag.TextDocument()
                if 'text' in text_edit:
                    text_data.text = text_edit['text']
                if 'fontSize' in text_edit:
                    text_data.fontSize = float(text_edit['fontSize'])
                if 'color' in text_edit:
                    text_data.fillColor = pypag.Color()
                    text_data.fillColor.red = int(text_edit['color'].get('r', 255))
                    text_data.fillColor.green = int(text_edit['color'].get('g', 255))
                    text_data.fillColor.blue = int(text_edit['color'].get('b', 255))
                pag_file.replaceText(text_idx, text_data)
                print(f"[图片编辑] 重新应用文本编辑: index={text_idx}")
        
        # 重新应用所有图片编辑
        for img_idx, img_edit in cache_item['edits']['images'].items():
            if 'path' in img_edit and Path(img_edit['path']).exists():
                pag_img = pypag.PAGImage.FromPath(img_edit['path'])
                if pag_img:
                    pag_file.replaceImage(img_idx, pag_img)
                    print(f"[图片编辑] 重新应用图片编辑: index={img_idx}")
        
        # 更新缓存中的 PAG 文件
        cache_item['pag'] = pag_file
        
        # 清除 player 和 surface 缓存，强制重新创建
        if 'player' in cache_item:
            del cache_item['player']
            print(f"[图片编辑] 已清除 player 缓存")
        if 'surface' in cache_item:
            del cache_item['surface']
            print(f"[图片编辑] 已清除 surface 缓存")
        
        return jsonify({'success': True})
    
    except Exception as e:
        import traceback
        traceback.print_exc()
        return jsonify({'error': str(e)}), 500


@app.route('/api/reset/<int:pag_id>', methods=['POST'])
def reset_pag(pag_id):
    """重置 PAG 到原始状态"""
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    try:
        cache_item = pag_cache[pag_id]
        
        # 重新加载原始文件
        pag_file = pypag.PAGFile.Load(cache_item['path'])
        if not pag_file:
            return jsonify({'error': '重新加载失败'}), 500
        
        cache_item['pag'] = pag_file
        cache_item['edited'] = False
        cache_item['edits'] = {'texts': {}, 'images': {}}
        
        # 清除缓存的 player 和 surface
        if 'player' in cache_item:
            del cache_item['player']
        if 'surface' in cache_item:
            del cache_item['surface']
        
        print(f"[重置] PAG {pag_id} 已重置到原始状态")
        return jsonify({'success': True})
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/export-config/<int:pag_id>', methods=['GET'])
def export_config(pag_id):
    """导出 PAG 的编辑配置"""
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    try:
        cache_item = pag_cache[pag_id]
        
        config = {
            'version': '1.0',
            'pagFile': os.path.basename(cache_item['path']),
            'pagPath': cache_item['path'],
            'edits': cache_item.get('edits', {'texts': {}, 'images': {}}),
            'timestamp': datetime.now().isoformat()
        }
        
        # 生成文件名
        base_name = os.path.splitext(os.path.basename(cache_item['path']))[0]
        filename = f"{base_name}_config.json"
        
        response = jsonify(config)
        response.headers['Content-Disposition'] = f'attachment; filename="{filename}"'
        response.headers['Content-Type'] = 'application/json'
        
        return response
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


@app.route('/api/import-config/<int:pag_id>', methods=['POST'])
def import_config(pag_id):
    """导入 PAG 的编辑配置"""
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    try:
        if 'config' not in request.files:
            return jsonify({'error': '未上传配置文件'}), 400
        
        config_file = request.files['config']
        config_data = json.load(config_file.stream)
        
        # 验证配置格式
        if 'version' not in config_data or 'edits' not in config_data:
            return jsonify({'error': '配置文件格式不正确'}), 400
        
        cache_item = pag_cache[pag_id]
        
        # 重新加载原始 PAG 文件
        pag_file = pypag.PAGFile.Load(cache_item['path'])
        if not pag_file:
            return jsonify({'error': '重新加载 PAG 文件失败'}), 500
        
        cache_item['pag'] = pag_file
        cache_item['edits'] = {'texts': {}, 'images': {}}
        
        # 应用文本编辑
        texts = config_data['edits'].get('texts', {})
        for index_str, text_data in texts.items():
            index = int(index_str)
            if index < pag_file.numTexts():
                text_doc = pag_file.getTextData(index)
                text_doc.text = text_data.get('text', text_doc.text)
                
                if 'fontSize' in text_data:
                    text_doc.fontSize = text_data['fontSize']
                
                if 'fillColor' in text_data:
                    color = text_data['fillColor']
                    text_doc.fillColor = pypag.Color(
                        color.get('r', 255),
                        color.get('g', 255),
                        color.get('b', 255)
                    )
                
                pag_file.replaceText(index, text_doc)
                cache_item['edits']['texts'][index] = text_data
        
        # 应用图片编辑
        images = config_data['edits'].get('images', {})
        for index_str, image_data in images.items():
            index = int(index_str)
            image_path = image_data.get('path')
            
            if image_path and os.path.exists(image_path) and index < pag_file.numImages():
                pag_image = pypag.PAGImage.FromPath(image_path)
                if pag_image:
                    pag_file.replaceImage(index, pag_image)
                    cache_item['edits']['images'][index] = image_data
        
        # 清除缓存
        if 'player' in cache_item:
            del cache_item['player']
        if 'surface' in cache_item:
            del cache_item['surface']
        
        cache_item['edited'] = len(cache_item['edits']['texts']) > 0 or len(cache_item['edits']['images']) > 0
        
        print(f"[导入配置] PAG {pag_id} 配置导入成功")
        return jsonify({'success': True})
    
    except Exception as e:
        import traceback
        traceback.print_exc()
        return jsonify({'error': str(e)}), 500


@app.route('/api/clear-cache/<int:pag_id>', methods=['POST'])
def clear_cache(pag_id):
    """清除 PAG 的渲染缓存"""
    if pag_id not in pag_cache:
        return jsonify({'error': 'PAG 不存在'}), 404
    
    try:
        cache_item = pag_cache[pag_id]
        
        # 清除 player 和 surface 缓存
        if 'player' in cache_item:
            del cache_item['player']
        if 'surface' in cache_item:
            del cache_item['surface']
        
        return jsonify({'success': True})
    
    except Exception as e:
        return jsonify({'error': str(e)}), 500


if __name__ == '__main__':
    import logging
    
    # 禁用 Flask 的请求日志以提升性能
    log = logging.getLogger('werkzeug')
    log.setLevel(logging.ERROR)
    
    print("=" * 70)
    print(" PAG Web 服务器")
    print("=" * 70)
    print("\n访问地址: http://localhost:5001")
    print("\n功能:")
    print("  - 加载多个 PAG 文件")
    print("  - 按顺序播放")
    print("  - 编辑文本和图片")
    print("  - 实时预览")
    print("\n按 Ctrl+C 停止服务器")
    print("=" * 70)
    
    app.run(debug=False, host='0.0.0.0', port=5001)

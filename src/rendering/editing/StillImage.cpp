/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Tencent is pleased to support the open source community by making libpag available.
//
//  Copyright (C) 2021 Tencent. All rights reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
//  except in compliance with the License. You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  unless required by applicable law or agreed to in writing, software distributed under the
//  license is distributed on an "as is" basis, without warranties or conditions of any kind,
//  either express or implied. see the license for the specific language governing permissions
//  and limitations under the license.
//
/////////////////////////////////////////////////////////////////////////////////////////////////

#include "StillImage.h"
#include <algorithm>
#include <cctype>
#include "base/utils/TGFXCast.h"
#include "base/utils/UniqueID.h"
#include "codec/utils/WebpDecoder.h"
#include "pag/pag.h"
#include "rendering/caches/RenderCache.h"
#include "rendering/graphics/Graphic.h"
#include "rendering/graphics/Picture.h"
#include "tgfx/core/ImageCodec.h"
#include "tgfx/core/Pixmap.h"
#include "tgfx/gpu/opengl/GLDevice.h"

namespace pag {
std::shared_ptr<PAGImage> PAGImage::FromPath(const std::string& filePath) {
  auto data = tgfx::Data::MakeFromFile(filePath);
  if (!data) {
    return nullptr;
  }
  // 先创建图片（不移动 data）
  auto image = tgfx::Image::MakeFromEncoded(data);
  auto stillImage = StillImage::MakeFrom(std::move(image));
  if (stillImage) {
    // 保存原始字节数据
    static_cast<StillImage*>(stillImage.get())->originalBytes = data;
  }
  return stillImage;
}

std::shared_ptr<PAGImage> PAGImage::FromBytes(const void* bytes, size_t length) {
  auto fileBytes = tgfx::Data::MakeWithCopy(bytes, length);
  auto image = tgfx::Image::MakeFromEncoded(fileBytes);
  auto stillImage = StillImage::MakeFrom(std::move(image));
  if (stillImage) {
    // 保存原始字节数据
    static_cast<StillImage*>(stillImage.get())->originalBytes = fileBytes;
  }
  return stillImage;
}

std::shared_ptr<PAGImage> PAGImage::FromPixels(const void* pixels, int width, int height,
                                               size_t rowBytes, ColorType colorType,
                                               AlphaType alphaType) {
  auto info = tgfx::ImageInfo::Make(width, height, ToTGFX(colorType), ToTGFX(alphaType), rowBytes);
  tgfx::Bitmap bitmap(width, height, info.isAlphaOnly());
  bitmap.writePixels(info, pixels);
  auto image = tgfx::Image::MakeFrom(bitmap);
  return StillImage::MakeFrom(image);
}

std::shared_ptr<StillImage> StillImage::MakeFrom(std::shared_ptr<tgfx::Image> image) {
  if (image == nullptr) {
    return nullptr;
  }
  auto pagImage = std::shared_ptr<StillImage>(new StillImage(image->width(), image->height()));
  auto picture = Picture::MakeFrom(pagImage->uniqueID(), image);
  if (!picture) {
    return nullptr;
  }
  pagImage->graphic = picture;
  return pagImage;
}

std::shared_ptr<PAGImage> PAGImage::FromTexture(const BackendTexture& texture, ImageOrigin origin) {
  auto context = tgfx::GLDevice::CurrentNativeHandle();
  if (context == nullptr) {
    LOGE("PAGImage.FromTexture() There is no current GPU context on the calling thread.");
    return nullptr;
  }
  auto pagImage = std::shared_ptr<StillImage>(new StillImage(texture.width(), texture.height()));
  auto picture = Picture::MakeFrom(pagImage->uniqueID(), ToTGFX(texture), ToTGFX(origin));
  if (!picture) {
    LOGE("PAGImage.MakeFrom() The texture is invalid.");
    return nullptr;
  }
  pagImage->graphic = picture;
  return pagImage;
}

ByteData* PAGImage::toBytes() const {
  // 只有 StillImage 支持导出字节数据
  auto stillImage = dynamic_cast<const StillImage*>(this);
  if (!stillImage) {
    return nullptr;
  }
  
  // 如果有原始字节数据，检查是否为 WebP 格式
  if (stillImage->originalBytes) {
    auto data = stillImage->originalBytes;
    
    // 检查是否为 WebP 格式
    int width = 0, height = 0;
    if (WebPGetInfo(reinterpret_cast<const uint8_t*>(data->data()), data->size(), 
                    &width, &height)) {
      // 已经是 WebP 格式，直接返回
      auto byteData = ByteData::Make(data->size()).release();
      if (byteData) {
        memcpy(const_cast<void*>(static_cast<const void*>(byteData->data())), 
               data->data(), data->size());
      }
      return byteData;
    }
    
    // 不是 WebP 格式（JPEG, PNG 等），需要转换为 WebP
    // 使用 tgfx::ImageCodec 解码为 RGBA
    auto codec = tgfx::ImageCodec::MakeFrom(data);
    if (!codec) {
      return nullptr;
    }
    
    auto imageWidth = codec->width();
    auto imageHeight = codec->height();
    auto info = tgfx::ImageInfo::Make(imageWidth, imageHeight, 
                                      tgfx::ColorType::RGBA_8888,
                                      tgfx::AlphaType::Premultiplied);
    auto rowBytes = imageWidth * 4;
    auto pixelSize = rowBytes * imageHeight;
    auto pixels = new uint8_t[pixelSize];
    
    if (!codec->readPixels(info, pixels)) {
      delete[] pixels;
      return nullptr;
    }
    
    // 使用 tgfx::ImageCodec::Encode 编码为 WebP
    auto pixmap = tgfx::Pixmap(info, pixels);
    auto encoded = tgfx::ImageCodec::Encode(pixmap, tgfx::EncodedFormat::WEBP, 90);
    
    delete[] pixels;
    
    if (!encoded) {
      return nullptr;
    }
    
    // 复制编码后的数据
    auto bytes = new uint8_t[encoded->size()];
    memcpy(bytes, encoded->data(), encoded->size());
    
    return ByteData::MakeAdopted(bytes, encoded->size()).release();
  }
  
  // 如果没有原始字节（例如 FromPixels 创建的图片），返回 nullptr
  // 因为我们无法在没有渲染上下文的情况下编码图片
  return nullptr;
}

ByteData* PAGImage::toPNG() const {
  // 只有 StillImage 支持导出字节数据
  auto stillImage = dynamic_cast<const StillImage*>(this);
  if (!stillImage || !stillImage->originalBytes) {
    return nullptr;
  }
  
  auto data = stillImage->originalBytes;
  
  // 检查是否已经是 PNG 格式
  static const uint8_t PNG_SIGNATURE[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
  if (data->size() >= 8 && 
      memcmp(data->data(), PNG_SIGNATURE, 8) == 0) {
    // 已经是 PNG 格式，直接返回
    auto byteData = ByteData::Make(data->size()).release();
    if (byteData) {
      memcpy(const_cast<void*>(static_cast<const void*>(byteData->data())), 
             data->data(), data->size());
    }
    return byteData;
  }
  
  // 需要转换为 PNG
  auto codec = tgfx::ImageCodec::MakeFrom(data);
  if (!codec) {
    return nullptr;
  }
  
  auto imageWidth = codec->width();
  auto imageHeight = codec->height();
  auto info = tgfx::ImageInfo::Make(imageWidth, imageHeight, 
                                    tgfx::ColorType::RGBA_8888,
                                    tgfx::AlphaType::Premultiplied);
  auto rowBytes = imageWidth * 4;
  auto pixelSize = rowBytes * imageHeight;
  auto pixels = new uint8_t[pixelSize];
  
  if (!codec->readPixels(info, pixels)) {
    delete[] pixels;
    return nullptr;
  }
  
  // 使用 tgfx::ImageCodec::Encode 编码为 PNG
  auto pixmap = tgfx::Pixmap(info, pixels);
  auto encoded = tgfx::ImageCodec::Encode(pixmap, tgfx::EncodedFormat::PNG, 100);
  
  delete[] pixels;
  
  if (!encoded) {
    return nullptr;
  }
  
  // 复制编码后的数据
  auto bytes = new uint8_t[encoded->size()];
  memcpy(bytes, encoded->data(), encoded->size());
  
  return ByteData::MakeAdopted(bytes, encoded->size()).release();
}

ByteData* PAGImage::toJPEG(int quality) const {
  // 只有 StillImage 支持导出字节数据
  auto stillImage = dynamic_cast<const StillImage*>(this);
  if (!stillImage || !stillImage->originalBytes) {
    return nullptr;
  }
  
  // 限制质量范围在 0-100
  if (quality < 0) quality = 0;
  if (quality > 100) quality = 100;
  
  auto data = stillImage->originalBytes;
  
  // 检查是否已经是 JPEG 格式
  static const uint8_t JPEG_SIGNATURE[] = {0xFF, 0xD8, 0xFF};
  if (data->size() >= 3 && 
      memcmp(data->data(), JPEG_SIGNATURE, 3) == 0) {
    // 已经是 JPEG 格式
    // 如果质量要求是100或90（默认高质量），直接返回原始数据
    if (quality >= 90) {
      auto byteData = ByteData::Make(data->size()).release();
      if (byteData) {
        memcpy(const_cast<void*>(static_cast<const void*>(byteData->data())), 
               data->data(), data->size());
      }
      return byteData;
    }
  }
  
  // 需要转换为 JPEG 或重新编码
  auto codec = tgfx::ImageCodec::MakeFrom(data);
  if (!codec) {
    return nullptr;
  }
  
  auto imageWidth = codec->width();
  auto imageHeight = codec->height();
  auto info = tgfx::ImageInfo::Make(imageWidth, imageHeight, 
                                    tgfx::ColorType::RGBA_8888,
                                    tgfx::AlphaType::Premultiplied);
  auto rowBytes = imageWidth * 4;
  auto pixelSize = rowBytes * imageHeight;
  auto pixels = new uint8_t[pixelSize];
  
  if (!codec->readPixels(info, pixels)) {
    delete[] pixels;
    return nullptr;
  }
  
  // 使用 tgfx::ImageCodec::Encode 编码为 JPEG
  auto pixmap = tgfx::Pixmap(info, pixels);
  auto encoded = tgfx::ImageCodec::Encode(pixmap, tgfx::EncodedFormat::JPEG, quality);
  
  delete[] pixels;
  
  if (!encoded) {
    return nullptr;
  }
  
  // 复制编码后的数据
  auto bytes = new uint8_t[encoded->size()];
  memcpy(bytes, encoded->data(), encoded->size());
  
  return ByteData::MakeAdopted(bytes, encoded->size()).release();
}

ByteData* PAGImage::encode(const std::string& format, int quality) const {
  // 将格式字符串转换为大写以便比较
  std::string upperFormat = format;
  std::transform(upperFormat.begin(), upperFormat.end(), upperFormat.begin(), ::toupper);
  
  if (upperFormat == "PNG") {
    return toPNG();
  } else if (upperFormat == "JPEG" || upperFormat == "JPG") {
    return toJPEG(quality);
  } else if (upperFormat == "WEBP") {
    // toBytes() 默认返回 WebP 格式
    // 但我们可以提供质量参数的版本
    auto stillImage = dynamic_cast<const StillImage*>(this);
    if (!stillImage || !stillImage->originalBytes) {
      return nullptr;
    }
    
    auto data = stillImage->originalBytes;
    auto codec = tgfx::ImageCodec::MakeFrom(data);
    if (!codec) {
      return nullptr;
    }
    
    auto imageWidth = codec->width();
    auto imageHeight = codec->height();
    auto info = tgfx::ImageInfo::Make(imageWidth, imageHeight, 
                                      tgfx::ColorType::RGBA_8888,
                                      tgfx::AlphaType::Premultiplied);
    auto rowBytes = imageWidth * 4;
    auto pixelSize = rowBytes * imageHeight;
    auto pixels = new uint8_t[pixelSize];
    
    if (!codec->readPixels(info, pixels)) {
      delete[] pixels;
      return nullptr;
    }
    
    auto pixmap = tgfx::Pixmap(info, pixels);
    auto encoded = tgfx::ImageCodec::Encode(pixmap, tgfx::EncodedFormat::WEBP, quality);
    
    delete[] pixels;
    
    if (!encoded) {
      return nullptr;
    }
    
    auto bytes = new uint8_t[encoded->size()];
    memcpy(bytes, encoded->data(), encoded->size());
    
    return ByteData::MakeAdopted(bytes, encoded->size()).release();
  }
  
  // 不支持的格式
  return nullptr;
}

}  // namespace pag

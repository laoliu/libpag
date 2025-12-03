#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <fstream>
#include "pag/pag.h"
#include "pag/file.h"

namespace py = pybind11;

void bind_pag_file(py::module& m) {
    // Rect 类型 - 添加到主模块中定义，这里只是确保使用
    // Point 和 Matrix 已在主模块中定义
    
    // TextDocument 类
    py::class_<pag::TextDocument, std::shared_ptr<pag::TextDocument>>(m, "TextDocument")
        .def(py::init<>())
        .def_readwrite("text", &pag::TextDocument::text)
        .def_readwrite("fillColor", &pag::TextDocument::fillColor)
        .def_readwrite("strokeColor", &pag::TextDocument::strokeColor)
        .def_readwrite("fontSize", &pag::TextDocument::fontSize);
    
    // LayerType 枚举
    py::enum_<pag::LayerType>(m, "LayerType")
        .value("Unknown", pag::LayerType::Unknown)
        .value("Null", pag::LayerType::Null)
        .value("Solid", pag::LayerType::Solid)
        .value("Text", pag::LayerType::Text)
        .value("Shape", pag::LayerType::Shape)
        .value("Image", pag::LayerType::Image)
        .value("PreCompose", pag::LayerType::PreCompose)
        .export_values();
    
    // PAGLayer 基类
    py::class_<pag::PAGLayer, std::shared_ptr<pag::PAGLayer>>(m, "PAGLayer")
        .def("layerType", &pag::PAGLayer::layerType)
        .def("layerName", &pag::PAGLayer::layerName)
        .def("setVisible", &pag::PAGLayer::setVisible)
        .def("visible", &pag::PAGLayer::visible)
        .def("startTime", &pag::PAGLayer::startTime)
        .def("duration", &pag::PAGLayer::duration)
        .def("frameRate", &pag::PAGLayer::frameRate)
        
        // 变换方法
        .def("matrix", &pag::PAGLayer::matrix, "Get transformation matrix")
        .def("setMatrix", &pag::PAGLayer::setMatrix, py::arg("matrix"),
             "Set transformation matrix")
        .def("resetMatrix", &pag::PAGLayer::resetMatrix, "Reset transformation matrix")
        .def("getTotalMatrix", &pag::PAGLayer::getTotalMatrix,
             "Get total transformation matrix")
        
        // 变换分解方法
        .def("getPosition", &pag::PAGLayer::getPosition,
             "Get position as Point(x, y)")
        .def("setPosition", &pag::PAGLayer::setPosition, 
             py::arg("x"), py::arg("y"),
             "Set position (x, y)")
        .def("getScale", &pag::PAGLayer::getScale,
             "Get scale as Point(scaleX, scaleY)")
        .def("setScale", &pag::PAGLayer::setScale,
             py::arg("scaleX"), py::arg("scaleY"),
             "Set scale (scaleX, scaleY)")
        .def("getRotation", &pag::PAGLayer::getRotation,
             "Get rotation angle in degrees")
        .def("setRotation", &pag::PAGLayer::setRotation,
             py::arg("degrees"),
             "Set rotation angle in degrees")
        .def("getSkew", &pag::PAGLayer::getSkew,
             "Get skew as Point(skewX, skewY) in degrees")
        .def("setSkew", &pag::PAGLayer::setSkew,
             py::arg("skewX"), py::arg("skewY"),
             "Set skew (skewX, skewY) in degrees")
        
        .def("getAnchorPoint", &pag::PAGLayer::getAnchorPoint,
             "Get anchor point as Point(x, y)")
        .def("setAnchorPoint", &pag::PAGLayer::setAnchorPoint,
             py::arg("x"), py::arg("y"),
             "Set anchor point (x, y)")
        
        .def("alpha", &pag::PAGLayer::alpha, "Get layer alpha")
        .def("setAlpha", &pag::PAGLayer::setAlpha, py::arg("alpha"),
             "Set layer alpha (0-1)");
    
    // PAGTextLayer
    py::class_<pag::PAGTextLayer, pag::PAGLayer, std::shared_ptr<pag::PAGTextLayer>>(m, "PAGTextLayer")
        .def("text", &pag::PAGTextLayer::text)
        .def("setText", &pag::PAGTextLayer::setText)
        .def("fillColor", &pag::PAGTextLayer::fillColor)
        .def("setFillColor", &pag::PAGTextLayer::setFillColor)
        .def("fontSize", &pag::PAGTextLayer::fontSize)
        .def("setFontSize", &pag::PAGTextLayer::setFontSize)
        .def("strokeColor", &pag::PAGTextLayer::strokeColor)
        .def("setStrokeColor", &pag::PAGTextLayer::setStrokeColor)
        .def("reset", &pag::PAGTextLayer::reset);
    
    // PAGImageLayer
    py::class_<pag::PAGImageLayer, pag::PAGLayer, std::shared_ptr<pag::PAGImageLayer>>(m, "PAGImageLayer")
        .def("replaceImage", &pag::PAGImageLayer::replaceImage)
        .def("setImage", &pag::PAGImageLayer::setImage)
        .def("contentDuration", &pag::PAGImageLayer::contentDuration)
        .def("getReplacedImage", &pag::PAGImageLayer::getReplacedImage,
             "Returns the current replaced image if any, otherwise returns None")
        .def("getOriginalImage", &pag::PAGImageLayer::getOriginalImage,
             "Returns the original placeholder image from the PAG file")
        .def("getCurrentImage", &pag::PAGImageLayer::getCurrentImage,
             "Returns the current image (replaced or original)")
        .def("getOriginalImageBounds", &pag::PAGImageLayer::getOriginalImageBounds,
             "Returns the bounds of the original placeholder image")
        .def("getOriginalImageMatrix", &pag::PAGImageLayer::getOriginalImageMatrix,
             "Returns the transformation matrix of the original placeholder image")
        .def("getOriginalScaleFactor", &pag::PAGImageLayer::getOriginalScaleFactor,
             "Returns the scale factor of the original placeholder image")
        .def("getOriginalAnchorPoint", &pag::PAGImageLayer::getOriginalAnchorPoint,
             "Returns the anchor point of the original placeholder image");
    
    // PAGComposition 基类
    py::class_<pag::PAGComposition, pag::PAGLayer, std::shared_ptr<pag::PAGComposition>>(m, "PAGComposition")
        .def("width", &pag::PAGComposition::width)
        .def("height", &pag::PAGComposition::height)
        .def("numChildren", &pag::PAGComposition::numChildren)
        .def("getLayerAt", &pag::PAGComposition::getLayerAt)
        .def("getLayersByName", &pag::PAGComposition::getLayersByName)
        .def("addLayer", &pag::PAGComposition::addLayer)
        .def("removeLayer", &pag::PAGComposition::removeLayer)
        .def("removeAllLayers", &pag::PAGComposition::removeAllLayers);
    
    // PAGFile (继承自 PAGComposition)
    py::class_<pag::PAGFile, pag::PAGComposition, std::shared_ptr<pag::PAGFile>>(m, "PAGFile")
        .def_static("Load", py::overload_cast<const std::string&>(&pag::PAGFile::Load),
                   py::arg("filePath"),
                   "Load a PAG file from the specified path")
        
        .def("numTexts", &pag::PAGFile::numTexts,
             "Returns the number of replaceable texts")
        
        .def("numImages", &pag::PAGFile::numImages,
             "Returns the number of replaceable images")
        
        .def("numVideos", &pag::PAGFile::numVideos,
             "Returns the number of video compositions")
        
        .def("getTextData", &pag::PAGFile::getTextData,
             py::arg("editableTextIndex"),
             "Get text data at specified index")
        
        .def("replaceText", &pag::PAGFile::replaceText,
             py::arg("editableTextIndex"),
             py::arg("textData"),
             "Replace text at specified index")
        
        .def("replaceImage", &pag::PAGFile::replaceImage,
             py::arg("editableImageIndex"),
             py::arg("image"),
             "Replace image at specified index")
        
        .def("replaceImageByName", &pag::PAGFile::replaceImageByName,
             py::arg("layerName"),
             py::arg("image"),
             "Replace image by layer name")
        
        .def("getLayersByEditableIndex", &pag::PAGFile::getLayersByEditableIndex,
             py::arg("editableIndex"),
             py::arg("layerType"),
             "Get layers by editable index")
        
        .def("getEditableIndices", &pag::PAGFile::getEditableIndices,
             py::arg("layerType"),
             "Get editable indices for specified layer type")
        
        .def("setDuration", &pag::PAGFile::setDuration,
             py::arg("duration"),
             "Set the duration in microseconds")
        
        .def("copyOriginal", &pag::PAGFile::copyOriginal,
             "Make a copy of the original file")
        
        .def("path", &pag::PAGFile::path,
             "Returns the file path")
        
        .def("save", [](pag::PAGFile& self, const std::string& filePath) -> bool {
            // 创建原始文件的副本
            auto copiedFile = self.copyOriginal();
            if (!copiedFile) {
                return false;
            }
            
            // 获取副本的 File 对象
            auto file = copiedFile->getFile();
            if (!file) {
                return false;
            }
            
            // 将当前的所有文本修改直接应用到 File 对象
            int numTexts = self.numTexts();
            for (int i = 0; i < numTexts; i++) {
                auto textLayers = self.getLayersByEditableIndex(i, pag::LayerType::Text);
                if (!textLayers.empty()) {
                    // 获取当前文本层的数据
                    auto textLayer = std::static_pointer_cast<pag::PAGTextLayer>(textLayers[0]);
                    
                    // 创建新的 TextDocument 包含当前值
                    auto textData = std::make_shared<pag::TextDocument>();
                    textData->text = textLayer->text();
                    textData->fillColor = textLayer->fillColor();
                    textData->strokeColor = textLayer->strokeColor();
                    textData->fontSize = textLayer->fontSize();
                    
                    // 直接修改 File 对象的文本数据
                    file->setTextData(i, textData);
                }
            }
            
            // 将当前的所有图片修改应用到副本
            int numImages = self.numImages();
            for (int i = 0; i < numImages; i++) {
                auto imageLayers = self.getLayersByEditableIndex(i, pag::LayerType::Image);
                if (!imageLayers.empty()) {
                    auto imageLayer = std::static_pointer_cast<pag::PAGImageLayer>(imageLayers[0]);
                    // 获取当前替换的图片
                    auto currentImage = imageLayer->getReplacedImage();
                    if (currentImage) {
                        // 使用新的 API 将图片数据直接写入 File 对象
                        auto imageBytes = currentImage->toBytes();
                        if (imageBytes) {
                            file->setImageData(i, imageBytes, currentImage->width(), currentImage->height());
                            delete imageBytes;
                        }
                    }
                }
            }
            
            // 编码 File 对象
            auto byteData = pag::Codec::Encode(file);
            if (!byteData || byteData->length() == 0) {
                return false;
            }
            
            // 写入文件
            std::ofstream outFile(filePath, std::ios::binary);
            if (!outFile.is_open()) {
                return false;
            }
            outFile.write(reinterpret_cast<const char*>(byteData->data()), byteData->length());
            outFile.close();
            return true;
        }, py::arg("filePath"),
        "Save the PAG file with all current modifications (replaceText and replaceImage) to the specified path")
        
        .def("toBytes", [](pag::PAGFile& self) -> py::bytes {
            // 创建原始文件的副本
            auto copiedFile = self.copyOriginal();
            if (!copiedFile) {
                return py::bytes();
            }
            
            // 获取副本的 File 对象
            auto file = copiedFile->getFile();
            if (!file) {
                return py::bytes();
            }
            
            // 将当前的所有文本修改直接应用到 File 对象
            int numTexts = self.numTexts();
            for (int i = 0; i < numTexts; i++) {
                auto textLayers = self.getLayersByEditableIndex(i, pag::LayerType::Text);
                if (!textLayers.empty()) {
                    // 获取当前文本层的数据
                    auto textLayer = std::static_pointer_cast<pag::PAGTextLayer>(textLayers[0]);
                    
                    // 创建新的 TextDocument 包含当前值
                    auto textData = std::make_shared<pag::TextDocument>();
                    textData->text = textLayer->text();
                    textData->fillColor = textLayer->fillColor();
                    textData->strokeColor = textLayer->strokeColor();
                    textData->fontSize = textLayer->fontSize();
                    
                    // 直接修改 File 对象的文本数据
                    file->setTextData(i, textData);
                }
            }
            
            // 将当前的所有图片修改应用到副本
            int numImages = self.numImages();
            for (int i = 0; i < numImages; i++) {
                auto imageLayers = self.getLayersByEditableIndex(i, pag::LayerType::Image);
                if (!imageLayers.empty()) {
                    auto imageLayer = std::static_pointer_cast<pag::PAGImageLayer>(imageLayers[0]);
                    // 获取当前替换的图片
                    auto currentImage = imageLayer->getReplacedImage();
                    if (currentImage) {
                        // 使用新的 API 将图片数据直接写入 File 对象
                        auto imageBytes = currentImage->toBytes();
                        if (imageBytes) {
                            file->setImageData(i, imageBytes, currentImage->width(), currentImage->height());
                            delete imageBytes;
                        }
                    }
                }
            }
            
            // 编码 File 对象
            auto byteData = pag::Codec::Encode(file);
            if (!byteData || byteData->length() == 0) {
                return py::bytes();
            }
            
            return py::bytes(reinterpret_cast<const char*>(byteData->data()), byteData->length());
        }, "Encode the PAG file with all current modifications (replaceText and replaceImage) to bytes");
}

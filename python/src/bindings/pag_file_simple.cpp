#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pag/pag.h"

namespace py = pybind11;

void bind_pag_file(py::module& m) {
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
        .def("frameRate", &pag::PAGLayer::frameRate);
    
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
        .def("contentDuration", &pag::PAGImageLayer::contentDuration);
    
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
             "Returns the file path");
}

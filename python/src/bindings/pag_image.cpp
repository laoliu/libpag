#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "pag/pag.h"

namespace py = pybind11;

void bind_pag_image(py::module& m) {
    py::class_<pag::PAGImage, std::shared_ptr<pag::PAGImage>>(m, "PAGImage")
        .def_static("FromPath", &pag::PAGImage::FromPath,
                   py::arg("filePath"),
                   "Creates a PAGImage from an image file path")
        
        .def_static("FromBytes", [](py::bytes data) {
            std::string str = data;
            return pag::PAGImage::FromBytes(str.data(), str.size());
        }, py::arg("data"),
           "Creates a PAGImage from image bytes")
        
        .def("width", &pag::PAGImage::width,
             "Returns the width in pixels")
        
        .def("height", &pag::PAGImage::height,
             "Returns the height in pixels")
        
        .def("scaleMode", &pag::PAGImage::scaleMode,
             "Returns the current scale mode")
        
        .def("setScaleMode", &pag::PAGImage::setScaleMode,
             py::arg("mode"),
             "Set the scale mode")
        
        .def("matrix", &pag::PAGImage::matrix,
             "Returns a copy of the current matrix")
        
        .def("setMatrix", &pag::PAGImage::setMatrix,
             py::arg("matrix"),
             "Sets the transformation matrix")
        
        .def("toBytes", [](const pag::PAGImage& self) -> py::object {
            auto byteData = self.toBytes();
            if (!byteData) {
                return py::none();
            }
            auto result = py::bytes(reinterpret_cast<const char*>(byteData->data()), 
                                   byteData->length());
            delete byteData;  // 手动释放内存
            return result;
        }, "Encodes the image to WebP format bytes")
        
        .def("toPNG", [](const pag::PAGImage& self) -> py::object {
            auto byteData = self.toPNG();
            if (!byteData) {
                return py::none();
            }
            auto result = py::bytes(reinterpret_cast<const char*>(byteData->data()), 
                                   byteData->length());
            delete byteData;  // 手动释放内存
            return result;
        }, "Encodes the image to PNG format bytes (lossless)")
        
        .def("toJPEG", [](const pag::PAGImage& self, int quality) -> py::object {
            auto byteData = self.toJPEG(quality);
            if (!byteData) {
                return py::none();
            }
            auto result = py::bytes(reinterpret_cast<const char*>(byteData->data()), 
                                   byteData->length());
            delete byteData;  // 手动释放内存
            return result;
        }, py::arg("quality") = 90,
           "Encodes the image to JPEG format bytes. Quality: 0-100, default 90")
        
        .def("encode", [](const pag::PAGImage& self, const std::string& format, int quality) -> py::object {
            auto byteData = self.encode(format, quality);
            if (!byteData) {
                return py::none();
            }
            auto result = py::bytes(reinterpret_cast<const char*>(byteData->data()), 
                                   byteData->length());
            delete byteData;  // 手动释放内存
            return result;
        }, py::arg("format"), py::arg("quality") = 90,
           "Encodes the image to specified format (PNG/JPEG/WEBP). Quality: 0-100 for lossy formats");
}

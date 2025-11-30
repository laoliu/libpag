#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pag/pag.h"

namespace py = pybind11;

void bind_pag_file(py::module& m) {
    py::class_<pag::PAGFile, std::shared_ptr<pag::PAGFile>>(m, "PAGFile")
        .def_static("Load", [](const std::string& path) {
            return pag::PAGFile::Load(path);
        }, py::arg("path"), "Load a PAG file from path")
        
        .def_static("LoadFromBytes", [](py::bytes data) {
            std::string str = data;
            return pag::PAGFile::Load(
                reinterpret_cast<const uint8_t*>(str.data()), 
                str.size()
            );
        }, py::arg("data"), "Load a PAG file from bytes")
        
        .def("width", &pag::PAGFile::width, "Get the width of the PAG file")
        .def("height", &pag::PAGFile::height, "Get the height of the PAG file")
        .def("duration", &pag::PAGFile::duration, "Get the duration in microseconds")
        .def("frameRate", &pag::PAGFile::frameRate, "Get the frame rate")
        .def("numTexts", &pag::PAGFile::numTexts, "Get the number of editable texts")
        .def("numImages", &pag::PAGFile::numImages, "Get the number of replaceable images")
        .def("numVideos", &pag::PAGFile::numVideos, "Get the number of video sequences")
        
        .def("getTextData", &pag::PAGFile::getTextData, py::arg("index"),
             "Get text data at index")
        .def("replaceText", py::overload_cast<int, const std::string&>(&pag::PAGFile::replaceText),
             py::arg("index"), py::arg("text"), "Replace text at index")
        
        .def("getLayersByName", &pag::PAGFile::getLayersByName, py::arg("name"),
             "Get layers by name")
        .def("getLayersUnderPoint", &pag::PAGFile::getLayersUnderPoint, 
             py::arg("x"), py::arg("y"), "Get layers under point")
        
        .def("timeStretchMode", &pag::PAGFile::timeStretchMode, "Get time stretch mode")
        .def("setTimeStretchMode", &pag::PAGFile::setTimeStretchMode, py::arg("mode"),
             "Set time stretch mode")
        
        .def("setDuration", &pag::PAGFile::setDuration, py::arg("duration"),
             "Set the duration in microseconds")
        
        .def("copyOriginal", &pag::PAGFile::copyOriginal, "Make a copy of the original file")
        
        .def("__repr__", [](const pag::PAGFile& file) {
            return "<PAGFile width=" + std::to_string(file.width()) + 
                   " height=" + std::to_string(file.height()) +
                   " duration=" + std::to_string(file.duration()) + ">";
        });

    // TimeStretchMode 枚举
    py::enum_<pag::Enum>(m, "TimeStretchMode")
        .value("None", pag::Enum::None)
        .value("Scale", pag::Enum::Scale)
        .value("Repeat", pag::Enum::Repeat)
        .value("RepeatInverted", pag::Enum::RepeatInverted)
        .export_values();
}

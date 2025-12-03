#include <pybind11/pybind11.h>
#include "pag/pag.h"

namespace py = pybind11;

void bind_pag_layer(py::module& m) {
    py::class_<pag::PAGLayer, std::shared_ptr<pag::PAGLayer>>(m, "PAGLayer")
        .def("layerType", &pag::PAGLayer::layerType, "Get layer type")
        .def("layerName", &pag::PAGLayer::layerName, "Get layer name")
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
        
        .def("alpha", &pag::PAGLayer::alpha, "Get layer alpha")
        .def("setAlpha", &pag::PAGLayer::setAlpha, py::arg("alpha"),
             "Set layer alpha (0-1)")
        .def("visible", &pag::PAGLayer::visible, "Check if layer is visible")
        .def("setVisible", &pag::PAGLayer::setVisible, py::arg("visible"),
             "Set layer visibility")
        .def("editableIndex", &pag::PAGLayer::editableIndex,
             "Get editable index")
        .def("parent", &pag::PAGLayer::parent, "Get parent layer")
        .def("markers", &pag::PAGLayer::markers, "Get layer markers")
        .def("localTimeToGlobal", &pag::PAGLayer::localTimeToGlobal, py::arg("localTime"),
             "Convert local time to global time")
        .def("globalToLocalTime", &pag::PAGLayer::globalToLocalTime, py::arg("globalTime"),
             "Convert global time to local time")
        .def("duration", &pag::PAGLayer::duration, "Get layer duration")
        .def("frameRate", &pag::PAGLayer::frameRate, "Get layer frame rate")
        .def("startTime", &pag::PAGLayer::startTime, "Get layer start time")
        .def("setStartTime", &pag::PAGLayer::setStartTime, py::arg("time"),
             "Set layer start time")
        .def("currentTime", &pag::PAGLayer::currentTime, "Get current time")
        .def("setCurrentTime", &pag::PAGLayer::setCurrentTime, py::arg("time"),
             "Set current time")
        .def("getProgress", &pag::PAGLayer::getProgress, "Get progress (0-1)")
        .def("setProgress", &pag::PAGLayer::setProgress, py::arg("progress"),
             "Set progress (0-1)")
        .def("trackMatteLayer", &pag::PAGLayer::trackMatteLayer,
             "Get track matte layer")
        .def("getBounds", &pag::PAGLayer::getBounds, "Get layer bounds")
        
        .def("__repr__", [](const pag::PAGLayer& layer) {
            return "<PAGLayer name=\"" + layer.layerName() + "\">";
        });

    // PAGLayerType 枚举
    py::enum_<pag::LayerType>(m, "LayerType")
        .value("Unknown", pag::LayerType::Unknown)
        .value("Null", pag::LayerType::Null)
        .value("Solid", pag::LayerType::Solid)
        .value("Text", pag::LayerType::Text)
        .value("Shape", pag::LayerType::Shape)
        .value("Image", pag::LayerType::Image)
        .value("PreCompose", pag::LayerType::PreCompose)
        .export_values();
}

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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>
#include "pag/pag.h"

namespace py = pybind11;

// 前向声明绑定函数
void bind_pag_file(py::module& m);
void bind_pag_surface(py::module& m);
void bind_pag_player(py::module& m);
void bind_pag_image(py::module& m);

PYBIND11_MODULE(pypag, m) {
    m.doc() = "Python bindings for libpag - Portable Animated Graphics";

    // 绑定基础类型
    py::class_<pag::Point>(m, "Point")
        .def(py::init<>())
        .def(py::init<float, float>())
        .def_readwrite("x", &pag::Point::x)
        .def_readwrite("y", &pag::Point::y)
        .def("__repr__", [](const pag::Point& p) {
            return "Point(x=" + std::to_string(p.x) + ", y=" + std::to_string(p.y) + ")";
        });

    py::class_<pag::Color>(m, "Color")
        .def(py::init<>())
        .def_readwrite("red", &pag::Color::red)
        .def_readwrite("green", &pag::Color::green)
        .def_readwrite("blue", &pag::Color::blue)
        .def("__repr__", [](const pag::Color& c) {
            return "Color(r=" + std::to_string(c.red) + 
                   ", g=" + std::to_string(c.green) + 
                   ", b=" + std::to_string(c.blue) + ")";
        });

    // 绑定枚举类型
    py::enum_<pag::ColorType>(m, "ColorType")
        .value("RGBA_8888", pag::ColorType::RGBA_8888)
        .value("BGRA_8888", pag::ColorType::BGRA_8888)
        .export_values();
    
    py::enum_<pag::AlphaType>(m, "AlphaType")
        .value("Premultiplied", pag::AlphaType::Premultiplied)
        .value("Unpremultiplied", pag::AlphaType::Unpremultiplied)
        .export_values();
    
    py::enum_<pag::PAGScaleMode>(m, "PAGScaleMode")
        .value("None", pag::PAGScaleMode::None)
        .value("Stretch", pag::PAGScaleMode::Stretch)
        .value("LetterBox", pag::PAGScaleMode::LetterBox)
        .value("Zoom", pag::PAGScaleMode::Zoom)
        .export_values();
    
    py::class_<pag::Matrix>(m, "Matrix")
        .def(py::init<>())
        .def_static("MakeTrans", &pag::Matrix::MakeTrans)
        .def_static("MakeScale", 
                   py::overload_cast<float, float>(&pag::Matrix::MakeScale))
        .def("setAffine", &pag::Matrix::setAffine);

    // 绑定各个模块
    bind_pag_file(m);
    bind_pag_surface(m);
    bind_pag_player(m);
    bind_pag_image(m);

    // 版本信息
    m.attr("__version__") = "0.1.0";
}

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pag/pag.h"

namespace py = pybind11;

void bind_pag_surface(py::module& m) {
    py::class_<pag::PAGSurface, std::shared_ptr<pag::PAGSurface>>(m, "PAGSurface")
        .def_static("MakeOffscreen", &pag::PAGSurface::MakeOffscreen,
                   py::arg("width"), py::arg("height"),
                   "Creates a new PAGSurface for off-screen rendering")
        
        .def("width", &pag::PAGSurface::width,
             "Returns the width in pixels of the surface")
        
        .def("height", &pag::PAGSurface::height,
             "Returns the height in pixels of the surface")
        
        .def("updateSize", &pag::PAGSurface::updateSize,
             "Update the size of the surface, and reset the internal surface")
        
        .def("clearAll", &pag::PAGSurface::clearAll,
             "Erases all pixels of the surface with transparent color")
        
        .def("freeCache", &pag::PAGSurface::freeCache,
             "Free the cache created by the surface immediately")
        
        .def("readPixels", [](pag::PAGSurface& self, pag::ColorType colorType, 
                             pag::AlphaType alphaType) -> py::object {
            size_t rowBytes = self.width() * 4; // RGBA
            std::vector<uint8_t> pixels(self.height() * rowBytes);
            
            bool success = self.readPixels(colorType, alphaType, 
                                          pixels.data(), rowBytes);
            
            if (!success) {
                return py::none();
            }
            
            // 返回 Python bytes
            return py::bytes(reinterpret_cast<const char*>(pixels.data()), 
                           pixels.size());
        }, py::arg("colorType") = pag::ColorType::RGBA_8888,
           py::arg("alphaType") = pag::AlphaType::Premultiplied,
           "Reads pixels from the surface and returns as bytes");
}

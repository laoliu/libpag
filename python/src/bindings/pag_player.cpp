#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include "pag/pag.h"

namespace py = pybind11;

void bind_pag_player(py::module& m) {
    py::class_<pag::PAGPlayer, std::shared_ptr<pag::PAGPlayer>>(m, "PAGPlayer")
        .def(py::init<>(),
             "Creates a new PAGPlayer")
        
        .def("setComposition", &pag::PAGPlayer::setComposition,
             py::arg("composition"),
             "Sets a new PAGComposition for PAGPlayer to render as content")
        
        .def("getComposition", &pag::PAGPlayer::getComposition,
             "Returns the current PAGComposition")
        
        .def("setSurface", &pag::PAGPlayer::setSurface,
             py::arg("surface"),
             "Set the PAGSurface object for PAGPlayer to render onto")
        
        .def("getSurface", &pag::PAGPlayer::getSurface,
             "Returns the PAGSurface object")
        
        .def("videoEnabled", &pag::PAGPlayer::videoEnabled,
             "Returns whether video rendering is enabled")
        
        .def("setVideoEnabled", &pag::PAGPlayer::setVideoEnabled,
             py::arg("enabled"),
             "Enable or disable video rendering")
        
        .def("cacheEnabled", &pag::PAGPlayer::cacheEnabled,
             "Returns whether caching is enabled")
        
        .def("setCacheEnabled", &pag::PAGPlayer::setCacheEnabled,
             py::arg("enabled"),
             "Enable or disable caching")
        
        .def("useDiskCache", &pag::PAGPlayer::useDiskCache,
             "Returns whether disk cache is enabled")
        
        .def("setUseDiskCache", &pag::PAGPlayer::setUseDiskCache,
             py::arg("enabled"),
             "Enable or disable disk cache")
        
        .def("cacheScale", &pag::PAGPlayer::cacheScale,
             "Returns the cache scale factor")
        
        .def("setCacheScale", &pag::PAGPlayer::setCacheScale,
             py::arg("scale"),
             "Set the cache scale factor (0.0 to 1.0)")
        
        .def("maxFrameRate", &pag::PAGPlayer::maxFrameRate,
             "Returns the maximum frame rate")
        
        .def("setMaxFrameRate", &pag::PAGPlayer::setMaxFrameRate,
             py::arg("frameRate"),
             "Set the maximum frame rate (1 to 60)")
        
        .def("scaleMode", &pag::PAGPlayer::scaleMode,
             "Returns the current scale mode")
        
        .def("setScaleMode", &pag::PAGPlayer::setScaleMode,
             py::arg("mode"),
             "Set the scale mode")
        
        .def("matrix", &pag::PAGPlayer::matrix,
             "Returns a copy of the current matrix")
        
        .def("setMatrix", &pag::PAGPlayer::setMatrix,
             py::arg("matrix"),
             "Sets the transformation matrix")
        
        .def("duration", &pag::PAGPlayer::duration,
             "Returns the duration in microseconds")
        
        .def("getProgress", &pag::PAGPlayer::getProgress,
             "Returns the current progress (0.0 to 1.0)")
        
        .def("setProgress", &pag::PAGPlayer::setProgress,
             py::arg("progress"),
             "Sets the progress (0.0 to 1.0)")
        
        .def("currentFrame", &pag::PAGPlayer::currentFrame,
             "Returns the current frame number")
        
        .def("nextFrame", &pag::PAGPlayer::nextFrame,
             "Advance to the next frame")
        
        .def("preFrame", &pag::PAGPlayer::preFrame,
             "Go back to the previous frame")
        
        .def("autoClear", &pag::PAGPlayer::autoClear,
             "Returns whether auto clear is enabled")
        
        .def("setAutoClear", &pag::PAGPlayer::setAutoClear,
             py::arg("autoClear"),
             "Enable or disable auto clear")
        
        .def("prepare", &pag::PAGPlayer::prepare,
             "Prepares the player for the next flush")
        
        .def("flush", &pag::PAGPlayer::flush,
             "Apply all pending changes to the target surface");
}

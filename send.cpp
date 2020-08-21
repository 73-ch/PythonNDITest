#include <cstdio>
#include <chrono>
#include <iostream>
#include <Processing.NDI.Lib.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace py = pybind11;

class NDISender {

    int buffer_size;

    bool checkShape(const std::vector<ssize_t> &shape) {
        if (ndi_frame.xres != shape[0] || ndi_frame.yres != shape[1]) {
            changeResolution(shape[1], shape[0]);
        }

        if (shape[2] == 3) {
            ndi_frame.FourCC = NDIlib_FourCC_type_BGRX;
        } else if (shape[2] == 4) {
            ndi_frame.FourCC = NDIlib_FourCC_type_BGRA;
        } else {
            std::cerr << "shape[2] must be 3 or 4." << std::endl;
            return false;
        }

        return true;
    }

    void copyFrame(const py::array_t<unsigned char> &frame) const {

        if (ndi_frame.FourCC == NDIlib_FourCC_type_BGRA) {
            memcpy((unsigned char *) ndi_frame.p_data, frame.data(), frame.size());
        } else if (ndi_frame.FourCC == NDIlib_FourCC_type_BGRX) {
            for (int i = 0; i < ndi_frame.yres; ++i) {
                for (int j = 0; j < ndi_frame.xres; ++j) {
                    ndi_frame.p_data[(i * ndi_frame.xres + j) * 4] = (uint8_t) *frame.data(i, j, 0);
                    ndi_frame.p_data[(i * ndi_frame.xres + j) * 4 + 1] = (uint8_t) *frame.data(i, j, 1);
                    ndi_frame.p_data[(i * ndi_frame.xres + j) * 4 + 2] = (uint8_t) *frame.data(i, j, 2);
                    ndi_frame.p_data[(i * ndi_frame.xres + j) * 4 + 3] = 255;
                }
            }
        }
    }

    void changeResolution(const int xres, const int yres) {
        std::cout << "resize" << std::endl;
        uint8_t *tmp;

        ndi_frame.xres = xres;
        ndi_frame.yres = yres;
        buffer_size = xres * yres * 4;

        if ((tmp = (uint8_t *) realloc(ndi_frame.p_data, buffer_size)) != nullptr) {
            ndi_frame.p_data = tmp;
        } else {
            std::cout << "allocation error" << std::endl;
        }
    }

public:
    NDIlib_send_instance_t ndi_send;
    NDIlib_video_frame_v2_t ndi_frame;

    NDISender() {
        if (!NDIlib_initialize()) return;

        // We create the NDI sender
        ndi_send = NDIlib_send_create();
        if (!ndi_send) return;

        ndi_frame.p_data = (uint8_t *) malloc(sizeof(uint8_t) * 256 * 256 * 4);
        ndi_frame.FourCC = NDIlib_FourCC_type_BGRX;
    }

    ~NDISender() {
        std::cout << "destroy" << std::endl;

        free(ndi_frame.p_data);

        // Destroy the NDI sender
        NDIlib_send_destroy(ndi_send);

        // Not required, but nice
        NDIlib_destroy();
    }

    void send(const py::array_t<unsigned char> &frame) {
        const auto info = frame.request();

        if (!checkShape(info.shape)) return;

        copyFrame(frame);

        NDIlib_send_send_video_v2(ndi_send, &ndi_frame);

        std::cout << "send" << std::endl;
    }
};

PYBIND11_MODULE(pythonndi, m) {
    m.doc() = "Python binding of NDI";

    py::class_<NDISender>(m, "NDISender")
            .def(py::init<>())
            .def("send", (void (NDISender::*)(py::array_t<unsigned char>)) &NDISender::send);
}
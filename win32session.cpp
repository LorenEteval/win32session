#include <atomic>

#include <pybind11/pybind11.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(UNICODE) || !defined(_UNICODE)
    #error "Define unicode platform instead"
#endif

namespace py = pybind11;

namespace {
    using atomic_window_t = std::atomic<void *>;

    atomic_window_t Window;

    py::function Callback;

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message) {
        case WM_CLOSE:
            DestroyWindow(hWnd);
            break;
        case WM_DESTROY:
            PostQuitMessage(0U);
            break;
        case WM_QUERYENDSESSION:
            /*
             * The 'Window' is in an invalid state. But there is no need to reset
             * it since the system is about to get shutdown
             */
            if (Callback) {
                py::gil_scoped_acquire acquire;

                Callback();

                py::gil_scoped_release release;
            }
            return TRUE;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }

        return 0;
    }

    void set(py::function callback)
    {
        Callback = std::move(callback);
    }

    bool off()
    {
        if (Window.load()) {
            return static_cast<bool>(PostMessage(static_cast<HWND>(Window.exchange(nullptr)), WM_CLOSE, 0, 0));
        }

        // no daemon running
        return true;
    }

    bool run()
    {
        if (Window.load()) {
            // daemon already started
            return true;
        }

        WNDCLASSEX wx{};

        wx.cbSize        = sizeof(wx);
        wx.lpfnWndProc   = WndProc;
        wx.lpszClassName = L"win32session_class";

        if (!RegisterClassEx(&wx))
            return false;

        Window = CreateWindowEx(0, wx.lpszClassName, L"win32session", 0, 0, 0, 0, 0, nullptr, nullptr, nullptr, nullptr);

        if (Window.load() == nullptr)
            return false;

        {
            py::gil_scoped_release release;

            MSG msg{};

            while (GetMessage(&msg, nullptr, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }

            py::gil_scoped_acquire acquire;
        }

        return true;
    }

    PYBIND11_MODULE(win32session, m) {
        m.def("set", &set,
            "Set session callback", py::arg("callback"));

        m.def("off", &off,
            "Set session daemon off");

        m.def("run", &run,
            "Run session daemon");
    }
}

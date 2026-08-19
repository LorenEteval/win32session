#include <mutex>
#include <utility>

#include <pybind11/pybind11.h>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#if !defined(UNICODE) || !defined(_UNICODE)
    #error "Define unicode platform instead"
#endif

namespace py = pybind11;

namespace {
    constexpr wchar_t WindowClassName[] = L"win32session_class";
    constexpr char StateCapsuleName[] = "win32session.state";

    class SessionState {
    public:
        SessionState() = default;
        SessionState(const SessionState &) = delete;
        SessionState &operator=(const SessionState &) = delete;

        ~SessionState()
        {
            request_stop();
            clear_callback();
        }

        void set_callback(py::function callback)
        {
            py::function previous;
            {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                previous = std::move(callback_);
                callback_ = std::move(callback);
            }
        }

        void clear_callback()
        {
            py::function previous;
            {
                std::lock_guard<std::mutex> lock(callback_mutex_);
                previous = std::move(callback_);
            }
        }

        py::function callback() const
        {
            std::lock_guard<std::mutex> lock(callback_mutex_);
            return callback_;
        }

        bool begin_run()
        {
            std::lock_guard<std::mutex> lock(lifecycle_mutex_);
            if (running_) {
                return false;
            }

            running_ = true;
            stop_requested_ = false;
            return true;
        }

        bool publish_window(HWND window)
        {
            std::lock_guard<std::mutex> lock(lifecycle_mutex_);
            window_ = window;
            return stop_requested_;
        }

        void window_destroyed(HWND window)
        {
            std::lock_guard<std::mutex> lock(lifecycle_mutex_);
            if (window_ == window) {
                window_ = nullptr;
            }
        }

        void finish_run()
        {
            std::lock_guard<std::mutex> lock(lifecycle_mutex_);
            window_ = nullptr;
            running_ = false;
            stop_requested_ = false;
        }

        bool request_stop()
        {
            HWND window = nullptr;
            {
                std::lock_guard<std::mutex> lock(lifecycle_mutex_);
                if (running_) {
                    stop_requested_ = true;
                }
                window = window_;
            }

            return window == nullptr
                || static_cast<bool>(PostMessageW(window, WM_CLOSE, 0, 0));
        }

    private:
        mutable std::mutex callback_mutex_;
        py::function callback_;

        std::mutex lifecycle_mutex_;
        HWND window_ = nullptr;
        bool running_ = false;
        bool stop_requested_ = false;
    };

    SessionState &state_from(const py::capsule &capsule)
    {
        return *static_cast<SessionState *>(capsule.get_pointer());
    }

    void destroy_state(void *pointer)
    {
        // CPython invokes the capsule destructor while its interpreter context
        // is still valid, so SessionState can safely release its callback.
        delete static_cast<SessionState *>(pointer);
    }

    LRESULT CALLBACK WndProc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
    {
        if (message == WM_NCCREATE) {
            const auto *create = reinterpret_cast<const CREATESTRUCTW *>(l_param);
            SetWindowLongPtrW(
                window,
                GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        }

        auto *state = reinterpret_cast<SessionState *>(
            GetWindowLongPtrW(window, GWLP_USERDATA));

        switch (message) {
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;

        case WM_DESTROY:
            if (state != nullptr) {
                state->window_destroyed(window);
            }
            PostQuitMessage(0);
            return 0;

        case WM_QUERYENDSESSION:
            if (state != nullptr) {
                py::gil_scoped_acquire acquire;
                py::function callback = state->callback();

                if (callback) {
                    try {
                        callback();
                    } catch (py::error_already_set &error) {
                        error.restore();
                        PyErr_WriteUnraisable(callback.ptr());
                    }
                }
            }
            return TRUE;

        default:
            return DefWindowProcW(window, message, w_param, l_param);
        }
    }

    bool register_window_class()
    {
        WNDCLASSEXW window_class{};
        window_class.cbSize = sizeof(window_class);
        window_class.lpfnWndProc = WndProc;
        window_class.hInstance = GetModuleHandleW(nullptr);
        window_class.lpszClassName = WindowClassName;

        if (RegisterClassExW(&window_class) != 0) {
            return true;
        }

        return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
    }

    bool run(SessionState &state)
    {
        if (!state.begin_run()) {
            return true;
        }

        if (!register_window_class()) {
            state.finish_run();
            return false;
        }

        HWND window = CreateWindowExW(
            0,
            WindowClassName,
            L"win32session",
            0,
            0,
            0,
            0,
            0,
            nullptr,
            nullptr,
            GetModuleHandleW(nullptr),
            &state);

        if (window == nullptr) {
            state.finish_run();
            return false;
        }

        if (state.publish_window(window)) {
            PostMessageW(window, WM_CLOSE, 0, 0);
        }

        BOOL result = 0;
        {
            py::gil_scoped_release release;
            MSG message{};

            while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
        }

        // gil_scoped_release has ended and the Python thread context is restored.
        state.finish_run();
        state.clear_callback();
        return result != -1;
    }

    void initialize_module(py::module_ &module)
    {
        py::capsule state(
            new SessionState(),
            StateCapsuleName,
            &destroy_state);

        // Each module instance owns one capsule. Function captures keep it alive
        // if a caller retains a bound function after deleting sys.modules entry.
        module.add_object("_state", state);

        module.def(
            "set",
            [state](py::function callback) {
                state_from(state).set_callback(std::move(callback));
            },
            "Set the session shutdown callback.",
            py::arg("callback"));

        module.def(
            "off",
            [state]() {
                SessionState &session = state_from(state);
                const bool stopped = session.request_stop();
                session.clear_callback();
                return stopped;
            },
            "Stop the session daemon and release its callback.");

        module.def(
            "run",
            [state]() {
                return run(state_from(state));
            },
            "Run the session daemon.");
    }
}

#if PYBIND11_VERSION_HEX >= 0x03000000
PYBIND11_MODULE(win32session, module, py::mod_gil_not_used())
#else
PYBIND11_MODULE(win32session, module)
#endif
{
    initialize_module(module);
}

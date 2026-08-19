# win32session

[![Deploy PyPI](https://github.com/LorenEteval/win32session/actions/workflows/deploy-pypi.yml/badge.svg?branch=main)](https://github.com/LorenEteval/win32session/actions/workflows/deploy-pypi.yml)

Python bindings for win32 session cleanup management. This is a Windows-only package.

It works exactly the same
as [sysproxy daemon](https://github.com/LorenEteval/sysproxy?tab=readme-ov-file#sysproxy-daemon).

## Install

```
pip install win32session
```

## API

```pycon
>>> import win32session
>>> help(win32session) 
Help on module win32session:                                                                                                                                                                                                                                                    

NAME
    win32session

FUNCTIONS
    off(...) method of builtins.PyCapsule instance
        off() -> bool

        Stop the session daemon and release its callback.

    run(...) method of builtins.PyCapsule instance
        run() -> bool

        Run the session daemon.

    set(...) method of builtins.PyCapsule instance
        set(callback: Callable) -> None

        Set the session shutdown callback.
```

`off()` should normally be called before application exit. As a safety net, the
callback is owned by per-module state and is also released during module teardown.
It is never stored in a process-static C++ object.

## Tested Platform

win32session supports CPython 3.6 through 3.14 on Windows.

The following builds are covered by [GitHub Actions](https://github.com/LorenEteval/win32session/actions).

| Platform / architecture | Python versions |
|-------------------------|-----------------|
| Windows Server 2022 / AMD64 | 3.6-3.14, including 3.13t and 3.14t |
| Windows 11 / ARM64 | 3.11-3.14, including 3.13t and 3.14t |

Python 3.6-3.8 use the final compatible pybind11 2.x toolchain. Python 3.9 and
newer use the current pybind11 and cibuildwheel toolchain.

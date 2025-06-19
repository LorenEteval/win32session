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

        Set session daemon off

    run(...) method of builtins.PyCapsule instance
        run() -> bool

        Run session daemon

    set(...) method of builtins.PyCapsule instance
        set(callback: Callable) -> None

        Set session callback
```

## Tested Platform

win32session works on all reasonable Windows platform with all Python version(Python 3).

Below are tested build in [github actions](https://github.com/LorenEteval/win32session/actions).

| Platform     | Python 3.6-Python 3.13 |
|--------------|:----------------------:|
| windows-2019 |   :heavy_check_mark:   |
| windows-2022 |   :heavy_check_mark:   |

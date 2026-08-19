from pathlib import Path

from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup


ROOT = Path(__file__).parent


setup(
    name="win32session",
    version="0.2.0",
    license="MIT",
    description="Python bindings for Windows session cleanup management.",
    long_description=(ROOT / "README.md").read_text(encoding="utf-8"),
    long_description_content_type="text/markdown",
    author="Loren Eteval",
    author_email="loren.eteval@proton.me",
    url="https://github.com/LorenEteval/win32session",
    project_urls={
        "Source": "https://github.com/LorenEteval/win32session",
        "Issues": "https://github.com/LorenEteval/win32session/issues",
    },
    python_requires=">=3.6",
    ext_modules=[
        Pybind11Extension(
            "win32session",
            ["win32session.cpp"],
            cxx_std=11,
            define_macros=[("UNICODE", None), ("_UNICODE", None)],
            libraries=["user32"],
        )
    ],
    cmdclass={"build_ext": build_ext},
    classifiers=[
        "Development Status :: 5 - Production/Stable",
        "Intended Audience :: Developers",
        "Programming Language :: C++",
        "Programming Language :: Python :: 3",
        "Programming Language :: Python :: 3 :: Only",
        "Programming Language :: Python :: 3.6",
        "Programming Language :: Python :: 3.7",
        "Programming Language :: Python :: 3.8",
        "Programming Language :: Python :: 3.9",
        "Programming Language :: Python :: 3.10",
        "Programming Language :: Python :: 3.11",
        "Programming Language :: Python :: 3.12",
        "Programming Language :: Python :: 3.13",
        "Programming Language :: Python :: 3.14",
        "Operating System :: Microsoft :: Windows",
        "Topic :: System :: Operating System",
    ],
    zip_safe=False,
)

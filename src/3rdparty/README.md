## 3rd party libraries

This folder contains or downloads versions of libraries and files LabPlot can use.


## KDMacTouchBar

KDAB's Qt Widget for the Mac Touch Bar ([link](https://github.com/KDAB/KDMacTouchBar))

## liborigin

A library for reading OriginLab project files (.OPJ) ([link](https://sourceforge.net/projects/liborigin))

    * adapted CMakeLists.txt

## QXlsx

QXlsx is an excel file(.xlsx) reader/writer MIT-licensed C++ (with Qt) library ([link](https://github.com/QtExcel/QXlsx))

    # git clone -b v1.4.6 https://github.com/QtExcel/QXlsx.git tmp
    # mv tmp/QXlsx .
    # mv tmp/{LICENSE,README.md} QXlsx
    # rm -rf tmp

## Qt Advanced Docking System

Qt Advanced Docking System lets you create customizable layouts using a full featured window docking system ([link](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System))

Changes:

src/CMakeLists.txt:
+ if(NOT MSVC_FOUND)
+    target_compile_options(${library_name} PRIVATE -Wno-switch-enum -Wno-shadow -Wno-deprecated-declarations -Wno-unused-but-set-variable)
+ endif()

## MCAP

MCAP provides classes for reading and writing the MCAP file format.

https://github.com/foxglove/mcap

copy cpp/mcap/{include,LICENSE} -> mcap/

## QStringTokenizer

QStringTokenizer is a universal, safe, zero-allocation string splitter and is part of KDToolBox ([link](https://github.com/KDABLabs/KDToolBox/tree/master)).

## ReadStat (not included)

ReadStat is a command-line tool and MIT-licensed C library for reading files from popular stats packages ([link](https://github.com/WizardMac/ReadStat))

## Vector BLF

A library to access Binary Log File (BLF) files from Vector Informatik ([link](https://github.com/Murmele/vector_blf)).
Depends on fast_float ([link](https://github.com/fastfloat/fast_float.git)) and C++ DBC Parser (see below).
Included Vector BLF commit SHA 5fe16eb200a476a38fbe3f1d4281432ea5e6b976

## C++ DBC Parser

A library for parsing CAN DBC files from [LinuxDevon](https://github.com/LinuxDevon/dbc_parser_cpp).
Used by Vector BLF for decoding CAN messages. Depends on FastFloat (see below).
Included commit SHA 2c8acdbd54a638e3c4ab8f0d3815f7db4dd59afc (latest master as of 2026-05-24, v0.5.0)

How to update:
```bash
cd src/3rdparty
rm -rf dbc_parser_cpp
git clone https://github.com/LinuxDevon/dbc_parser_cpp.git
cd dbc_parser_cpp
# Use latest master or checkout specific commit/tag
# Record the commit hash here for documentation
rm -rf .git .github .gitignore .gitmodules
# Convert to Unix line endings
find . -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" -o -name "CMakeLists.txt" \) -exec perl -pi -e 's/\r\n/\n/g' {} \;
cd ..
```

**Modified files:**
- `CMakeLists.txt`:
  - Changed `option(DBC_ENABLE_TESTS ...)` from `ON` to `OFF` (tests require Catch2 via FetchContent)
  - Replaced FetchContent for FastFloat with `add_subdirectory(../fast_float ...)` to use local copy

## FastFloat

A fast and exact float parser ([link](https://github.com/fastfloat/fast_float)).
Header-only library used by C++ DBC Parser.
Included commit SHA 1ea4f27 (v6.0.0)

How to update:
```bash
cd src/3rdparty
rm -rf fast_float
git clone https://github.com/fastfloat/fast_float.git
cd fast_float
git checkout <desired-commit-or-tag>
rm -rf .git
# Convert to Unix line endings
find . -type f \( -name "*.h" -o -name "*.cpp" -o -name "CMakeLists.txt" -o -name "*.md" \) -exec perl -pi -e 's/\r\n/\n/g' {} \;
cd ..
```

**No modifications needed** - used as-is from upstream.

## preview.sty

This file provides the LaTeX style 'preview' ([link](https://www.ctan.org/tex-archive/macros/latex/contrib/preview)).

The main purpose of the preview package is the extraction of selected
elements from a LaTeX source, like formulas or graphics, into separate
pages of a DVI file.  A flexible and convenient interface allows it to
specify what commands and constructs should be extracted.  This works
with DVI files postprocessed by either Dvips and Ghostscript or
dvipng, but it also works when you are using PDFTeX for generating PDF
files.

This package is used for the rendering of mathematical LaTeX expressions embedded in TextLabel.

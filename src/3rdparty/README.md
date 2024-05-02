## 3rd party libraries

This folder contains versions of libraries and files LabPlot depends on.


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

## QStringTokenizer

QStringTokenizer is a universal, safe, zero-allocation string splitter and is part of KDToolBox ([link](https://github.com/KDABLabs/KDToolBox/tree/master)).

## ReadStat (not included)

ReadStat is a command-line tool and MIT-licensed C library for reading files from popular stats packages ([link](https://github.com/WizardMac/ReadStat))

## Vector BLF (not included)

A library to access Binary Log File (BLF) files from Vector Informatik ([link](https://github.com/Murmele/vector_blf)).
Depends on fast_float ([link](https://github.com/fastfloat/fast_float.git)) and C++ DBC Parser ([link](https://github.com/Murmele/dbc_parser_cpp))

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

# Introduction

This is a library to access Binary Log File (BLF) files from Vector Informatik.

# Build on Linux (e.g. Debian Testing)

Building under Linux works as usual:

    mkdir build
    cd build
    cmake ..
    make
    make install DESTDIR=..
    make package

# Build on Windows (e.g. Windows 7 64-Bit)

Building under Windows contains the following steps:

* Use cmake-gui
* Set "Where is the source code:" to the root directory.
* Set "Where to build the binaries:" to folder "build" below the root directory.
* Configure and Generate
* Open the Visual Studio Solution (.sln) file in the build folder.
* Compile it in Release Configuration.

# Test

* Configure cmake option OPTION_BUILD_TESTS
* Use ctest to run unit tests

Further options allow to activate coverage and reports:

* OPTION_USE_GCOV to build with coverage flags
* OPTION_ADD_LCOV to add lcov targets to generate HTML coverage report

# Package

The package generation can be triggered using

    make package

# Repository Structure

The following files are part of the source code distribution:

* src/_project_/
* src/_project_/tests/

The following files are working directories for building and testing:

* build/_project_/

The following files are products of installation and building:

* bin/
* lib/
* share/doc/_project_/
* share/man/
* include/_project_/

# Wanted features

* File Append
* There is currently no transition between little/big endian. Current support is only for little endian machines.
* There should be setter/getter methods instead of direct member variable access. Also for bit settings. Use std::chrono for all times
* All pointers should be of type std::unique_ptr to make ownership clear.
* Plausibility checks for length fields. Check all length and version fields of compliance with original files.
* Propagate exceptions from threads to main library, to notify user application, e.g. when reading unfinished files.
* Provide an Unknown object type to be able to read just a blob for all yet unknown object types
* Rename constants in form of ABC_XYZ to AbcXyz to be conform with coding standard. Breaks compatibility.
* Provide getter/setter for AppText::reservedAppText1
* Provide getter/setter for EnvironmentVariable::objectType (ENV_INTEGER, ENV_DOUBLE, ENV_STRING, ENV_DATA)
* Provide getter/setter for J1708Message::objectType (J1708_MESSAGE, J1708_VIRTUAL_MSG)
* Replace CompressedFile with std::fstream. Replace UncompressedFile with std::iostream with custom std::streambuf. Drop AbstractFile and use std::iostream instead.
* Jenkinsfile

# Standards

* Implementation is compatible with binlog API version 7.1.0.

# Test sources

* events_from_converter/*.blf have been converted from asc files using original converter under Windows.
  binlog API version was 3.9.6.0.
* events_from_binlog/*.blf have been generated using the binlog library under Windows.
  binlog API version was 4.5.2.2.
* customer files

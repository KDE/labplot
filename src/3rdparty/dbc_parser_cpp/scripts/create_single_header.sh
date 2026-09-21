#!/bin/sh

set -e

cargo install cpp-amalgamate

rm -rf build/single_header/
mkdir -p build/single_header/libdbc

source_files=$(find src -name "*.cpp")
include_files=$(find include -name "*.hpp")

cpp-amalgamate -d include -d build/_deps/fastfloat-src/include -o build/single_header/libdbc/libdbc.hpp ${source_files} ${include_files}

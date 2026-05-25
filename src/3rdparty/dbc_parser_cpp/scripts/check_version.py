#!/usr/bin/env python3
# Used to check the input verion against the input
import re
import argparse


def get_cmake_version(cmake_file):
    with open(cmake_file, 'r') as f:
        contents = f.read()
        match = re.search(r'project\(.*VERSION (\d+)\.(\d+)\.(\d+)', contents)
        if match:
            major, minor, patch = map(int, match.groups())
            return major, minor, patch
    return None


def validate_semver(version):
    pattern = r'^v(\d+)\.(\d+)\.(\d+)$'
    match = re.match(pattern, version)
    if match:
        return tuple(map(int, match.groups()))
    else:
        return None


def compare_versions(input_version, cmake_version):
    if input_version > cmake_version:
        print("Input version is greater than CMake version.")
        exit(1)
    elif input_version < cmake_version:
        print("Input version is smaller than CMake version.")
        exit(1)
    else:
        print("Input version is equal to CMake version.")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Check input version against CMake project version")
    parser.add_argument("--version", type=str, help="Input version with a 'v' prefix", required=True)
    args = parser.parse_args()

    cmake_version = get_cmake_version("CMakeLists.txt")
    if cmake_version is None:
        print("Failed to retrieve version from CMakeLists.txt.")
        exit(1)
    else:
        input_version = validate_semver(args.version)
        if input_version is None:
            print("Invalid input version format. Please provide a version in the format 'vX.Y.Z'")
            exit(1)
        else:
            compare_versions(input_version, cmake_version)




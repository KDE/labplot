#!/bin/bash

# Variable that will hold the name of the clang-format command
FMT=""

FOLDERS=("src/backend" "src/commonfrontend" "src/kdefrontend" "src/tools" "tests")

# Some distros just call it clang-format. Others (e.g. Ubuntu) are insistent
# that the version number be part of the command. We prefer clang-format-13 if
# that's present, otherwise we check clang-format 
for clangfmt in clang-format{-19,}; do
    if which "$clangfmt" &>/dev/null; then
        echo "$($clangfmt --version)"
        FMT="$clangfmt"
        break
    fi
done

# Check if we found a working clang-format
if [ -z "$FMT" ]; then
    echo "failed to find clang-format. Please install clang-format version 19 or above"
    exit 1
fi

function format() {
	# ignore getRSS.h file
    for f in $(find $@ -name '*.h' ! -iname 'getRSS.h' -or -name '*.m' -or -name '*.mm' -or -name '*.c' -or -name '*.cpp'); do
        echo "format ${f}";
        ${FMT} -i ${f};
    done

    echo "~~~ $@ Done ~~~";
}

# Check all of the arguments first to make sure they're all directories
for dir in ${FOLDERS[@]}; do
    if [ ! -d "${dir}" ]; then
        echo "${dir} is not a directory";
    else
        format ${dir};
    fi
done

#!/bin/bash
# Used to format files to the given format using clang format.
# example runs:
# ./scripts/fmt.sh
# ./scripts/fmt.sh format # This will write changes to file
APPLY_FORMAT=${1:-""}


# Variable that will hold the name of the clang-format command
FMT=""

# Some distros just call it clang-format. Others (e.g. Ubuntu) are insistent
# that the version number be part of the command. We prefer clang-format if
# that's present, otherwise we check clang-format-16
for clangfmt in clang-format{,-16}; do
    if which "$clangfmt" &>/dev/null; then
        FMT="$clangfmt"
        break
    fi
done

# Check if we found a working clang-format
if [ -z "$FMT" ]; then
    echo "failed to find clang-format. Please install clang-format version 16 or above"
    exit 1
fi

files=$(find . \( -path ./build -prune -o -path ./.git -prune -o -path ./third_party -prune \) -o -type f -name "*[h|c]pp" -print)

if [[ ${APPLY_FORMAT} = "format" ]]; then
    ${FMT} -i ${files}
else
    ${FMT} --dry-run --Werror ${files}
fi


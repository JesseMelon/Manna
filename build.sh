#!/bin/bash

# Flag to determine if we should clean
CLEAN=false

# Check for -c argument
while getopts "c" opt; do
    case $opt in
        c) CLEAN=true;;
        ?) echo "Usage: $0 [-c]"; exit 1;;
    esac
done

echo "Generating build script"

# Only run make clean if -c was provided
if [ "$CLEAN" = true ]; then
    make clean
fi

./thirdparty/bin/premake5/linux/premake5 gmake --os=linux
make

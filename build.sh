#!/bin/bash

# Flag to determine if we should clean
CLEAN=false
PROFILE=false

# Check for -c argument
while getopts "cp" opt; do
    case $opt in
        c) CLEAN=true;;
        p) PROFILE=true;;
        ?) echo "Usage: $0 [-c] [-p]"; exit 1;;
    esac
done

echo "Generating build script"

# Only run make clean if -c was provided
if [ "$CLEAN" = true ]; then
    make clean
fi

./thirdparty/bin/premake5/linux/premake5 gmake --os=linux

if [ "$PROFILE" = true ]; then
    echo "building profile configuration"
    gmake config=profile
else
    echo "building debug configuration"
    gmake
fi

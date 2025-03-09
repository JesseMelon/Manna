#!/bin/bash

echo "Generating build script"
make clean
./thirdparty/bin/premake5/linux/premake5 gmake --os=linux
make

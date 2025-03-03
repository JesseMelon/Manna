#!/bin/bash

echo "Generating build script"
./thirdparty/bin/premake5/linux/premake5 gmake2 --os=linux

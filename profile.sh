#!/bin/bash

echo "profiling editor"
cd "$(dirname "0")/bin/profile-linux-x86_64/manna_editor"
CPUPROFILE=/tmp/manna.prof ./manna_editor

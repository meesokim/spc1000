#!/bin/bash
if [ -z "$CIRCLEHOME" ]; then
    CIRCLEHOME=~/circle
fi
if [ ! -d "$CIRCLEHOME/addon/fatfs" ]; then
    echo "Error: Cannot find $CIRCLEHOME/addon/fatfs"
    echo "Please set CIRCLEHOME environment variable."
    exit 1
fi
echo "Patching ffconf.h for UTF-8 Korean filename support..."
sed -i 's/#define FF_LFN_UNICODE\s\+[0-9]/#define FF_LFN_UNICODE 2/' "$CIRCLEHOME/addon/fatfs/ffconf.h"
echo "Rebuilding libfatfs.a..."
cd "$CIRCLEHOME/addon/fatfs" && make clean && make
echo "Done!"

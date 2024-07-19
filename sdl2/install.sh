#!/bin/bash
git clone https://github.com/emscripten-core/emsdk ~/emsdk
~/emsdk/emsdk install latest
~/emsdk/emsdk activate latest
source ~/emsdk/emsdk_env.sh
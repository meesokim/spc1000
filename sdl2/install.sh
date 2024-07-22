#!/bin/bash
sudo apt install libsdl2-dev libzip-dev libbz2-dev
git clone https://github.com/emscripten-core/emsdk ~/emsdk
~/emsdk/emsdk install latest
~/emsdk/emsdk activate latest
source ~/emsdk/emsdk_env.sh

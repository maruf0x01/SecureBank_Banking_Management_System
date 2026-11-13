#!/usr/bin/env bash

set -e

cd "$(dirname "$0")"

cmake -S . -B build/cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build/cmake -j"$(nproc)" --target securebank_gui

./build/cmake/securebank_gui

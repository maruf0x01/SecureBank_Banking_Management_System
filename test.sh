#!/usr/bin/env bash

set -e

cd "$(dirname "$0")"

mkdir -p build
g++ -std=c++17 -Wall -Wextra -pedantic -Isrc \
    tests/test_main.cpp src/Account.cpp src/FileManager.cpp \
    -o build/securebank_tests

./build/securebank_tests

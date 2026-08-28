#!/usr/bin/env bash

set -e

cd "$(dirname "$0")"

mkdir -p build
g++ -std=c++17 -Wall -Wextra -pedantic src/*.cpp -o build/securebank

./build/securebank

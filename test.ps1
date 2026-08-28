$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

New-Item -ItemType Directory -Force -Path build | Out-Null
g++ -std=c++17 -Wall -Wextra -pedantic -Isrc `
    tests/test_main.cpp src/Account.cpp src/FileManager.cpp `
    -o build/securebank_tests.exe

& .\build\securebank_tests.exe

$ErrorActionPreference = "Stop"

Set-Location $PSScriptRoot

New-Item -ItemType Directory -Force -Path build | Out-Null
g++ -std=c++17 -Wall -Wextra -pedantic (Get-ChildItem src -Filter *.cpp).FullName -o build/securebank.exe

& .\build\securebank.exe

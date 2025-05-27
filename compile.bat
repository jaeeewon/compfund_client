@echo off
setlocal enabledelayedexpansion

set FILES=
for /R src %%f in (*.cpp) do (
    set FILES=!FILES! %%f
)

g++ -std=c++20 -Isrc %FILES% -o client.exe -lws2_32

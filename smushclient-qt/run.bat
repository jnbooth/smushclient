cd /D "%~dp0"

cmake.exe -S . -B build\Release
cmake.exe --build build\Release --config Release --target SmushClient
build\Release\Release\SmushClient.exe

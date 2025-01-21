setlocal
set CMAKE_BUILD_TYPE=Release
set PATH=%PATH%;%VCPKG_ROOT%/packages/qtbase_x64-windows-static/tools/Qt6/bin
set CFLAGS=-MT
set CXXFLAGS=-MT -Zc:__cplusplus -permissive-
set RUSTFLAGS=-C target-feature=+crt-static

cd /D "%~dp0"

rd /s /q build\Release\Release

cmake.exe -S . -B build\Release ^
  -DCMAKE_TOOLCHAIN_FILE="%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake" ^
  && cmake.exe --build build\Release --config Release --target SmushClient

@echo on

set "AZURE_SDK_DISABLE_AUTO_VCPKG=ON"

cd sdk\identity\azure-identity

mkdir build
cd build
cmake %CMAKE_ARGS% ^
  -D CMAKE_BUILD_TYPE=Release ^
  -G Ninja ^
  -D BUILD_SHARED_LIBS=ON ^
  -D BUILD_TRANSPORT_WINHTTP=ON ^
  ..
if %ERRORLEVEL% neq 0 exit 1

cmake --build . --target install
if %ERRORLEVEL% neq 0 exit 1

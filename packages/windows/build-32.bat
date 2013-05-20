rmdir /S /Q build-32
mkdir build-32
cd build-32
cmake ..\source -G "Visual Studio 10" > config.log
cmake --build . --config RelWithDebInfo > build.log
cmake --build . --config RelWithDebInfo --target msi > msi.log
move *.msi ..\
cmake --build . --config RelWithDebInfo --target package > zip.log
move *.zip ..\
cd ..

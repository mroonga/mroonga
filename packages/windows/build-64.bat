rmdir /S /Q build-64
mkdir build-64
cd build-64
cmake ..\source -G "Visual Studio 10 win64" > config.log
cmake --build . --config RelWithDebInfo > build.log
cmake --build . --config RelWithDebInfo --target msi > msi.log
move *.msi ..\
cmake --build . --config RelWithDebInfo --target package > zip.log
move *.zip ..\
cd ..

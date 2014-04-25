rmdir /S /Q build-zip-32
mkdir build-zip-32
cd build-zip-32
cmake ..\source -G "Visual Studio 10" > config.log
cmake --build . --config RelWithDebInfo > build.log
cmake --build . --config RelWithDebInfo --target package > zip.log
move *.zip ..\
cd ..

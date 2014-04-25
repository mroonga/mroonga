rmdir /S /Q build-msi-32
mkdir build-msi-32
cd build-msi-32
cmake ..\source -G "Visual Studio 10" > config.log
cmake --build . --config RelWithDebInfo > build.log
cmake --build . --config RelWithDebInfo --target msi > msi.log
move *.msi ..\
cd ..

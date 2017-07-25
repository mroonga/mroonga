rmdir /S /Q build-vc2017-msi-32
mkdir build-vc2017-msi-32
cd build-vc2017-msi-32
cmake ..\source -G "Visual Studio 15 2017" > config.log
cmake --build . --config RelWithDebInfo > build.log
cmake --build . --config RelWithDebInfo --target msi > msi.log
move *.msi ..\
cd ..

mkdir build
cd build

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=R:\  ..
# cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=DebugFull -DCMAKE_INSTALL_PREFIX=R:\  ..

mingw32-make -j 3 install

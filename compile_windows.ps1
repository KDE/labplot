mkdir build
cd build

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=C:\KDE  ..

mingw32-make -j 4 install

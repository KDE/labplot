$BUILDDIR = "build"

mkdir $BUILDDIR
cd $BUILDDIR

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=C:\CraftRoot  ..
#cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=C:\CraftRoot  ..

mingw32-make -j 4 install

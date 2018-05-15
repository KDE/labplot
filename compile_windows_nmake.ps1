$BUILDDIR = "build_nmake"

mkdir $BUILDDIR
cd $BUILDDIR

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release  ..

nmake.exe

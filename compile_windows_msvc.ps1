$BUILDDIR = "build_msvc"

mkdir $BUILDDIR
cd $BUILDDIR

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..

jom.exe install

cd ..

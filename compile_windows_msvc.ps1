$BUILDDIR = "build_msvc"

mkdir $BUILDDIR
cd $BUILDDIR

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-DH5_BUILT_AS_DYNAMIC_LIB" ..

jom.exe

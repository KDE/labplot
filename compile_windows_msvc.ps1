$BUILDDIR = "build_msvc"

mkdir $BUILDDIR
cd $BUILDDIR

cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release ..
#cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug ..
#cmake -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Debug -DENABLE_TESTS=OFF ..

jom.exe install
jom.exe test

cd ..

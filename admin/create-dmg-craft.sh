# Build it on Mac howto in script form
# but be aware, some frameworks need patching to have this working

# reference: https://cgit.kde.org/kate.git/tree/mac/emerge-deploy.sh
# run in CraftRoot/..

# errors are fatal
set -e

NAME=labplot2
VERSION=2.10.1
CRAFTROOT=CraftRoot
PREFIX=$CRAFTROOT/Applications
INPREFIX=$PREFIX/$NAME.app/Contents
TMPDIR=LabPlot2
SIGNATURE="Stefan Gerlach"

GCP=/opt/local/libexec/gnubin/cp

# run after "craft labplot"
#########################################

mkdir -pv $INPREFIX/share/{appdata,applications}
mkdir -pv $INPREFIX/Resources/kxmlgui5/labplot2

echo "Running macdeployqt ..."
# -verbose=3
LIBPATH=/Users/user/$CRAFTROOT/lib
macdeployqt $PREFIX/$NAME.app -verbose=2 -libpath=$LIBPATH

#########################################

echo "Install files"
# splash
cp -v $CRAFTROOT/share/$NAME/splash.png $INPREFIX/Resources/
# themes
cp -vr $CRAFTROOT/share/$NAME/themes $INPREFIX/Resources/
# gsl_distros, fit_models, colorchooser
cp -vr $CRAFTROOT/share/$NAME/pics $INPREFIX/Resources/
# color schemes (needs patched kcolorschememanager.cpp)
cp -vr $CRAFTROOT/share/$NAME/color-schemes $INPREFIX/Resources/color-schemes
#datasets
cp -vr $CRAFTROOT/share/$NAME/datasets $INPREFIX/Resources/datasets
#color maps
cp -vr $CRAFTROOT/share/$NAME/colormaps $INPREFIX/Resources/colormaps
#examples
cp -vr $CRAFTROOT/share/$NAME/examples $INPREFIX/Resources/examples
# appdata
cp -v $CRAFTROOT/share/metainfo/org.kde.labplot2.appdata.xml $INPREFIX/share/appdata/
cp -v $CRAFTROOT/share/applications/org.kde.$NAME.desktop $INPREFIX/share/applications/

# cantor
cp -v $CRAFTROOT/Applications/cantor.app/Contents/MacOS/cantor $INPREFIX/MacOS
cp -v $CRAFTROOT/Applications/cantor_scripteditor.app/Contents/MacOS/cantor_scripteditor $INPREFIX/MacOS
cp -v $CRAFTROOT/Applications/cantor_pythonserver.app/Contents/MacOS/cantor_pythonserver $INPREFIX/MacOS
cp -vr $CRAFTROOT/plugins/cantor $INPREFIX/PlugIns
cp -v $CRAFTROOT/lib/libcantor_config.dylib $INPREFIX/Frameworks/
# libcantorlibs.XX.dylib pulled in by macdeployqt may be broken
#$GCP -Pv kde/lib/libcantorlibs* $INPREFIX/Frameworks/
cp -v $CRAFTROOT/share/kxmlgui5/cantor/*.rc $INPREFIX/Resources/kxmlgui5/labplot2/

# icons
cp -vf $CRAFTROOT/share/icontheme.rcc $INPREFIX/Resources/icontheme.rcc

# kcharselect data
mkdir -p $INPREFIX/Resources/kf5/kcharselect
cp -v $CRAFTROOT/share/kf5/kcharselect/kcharselect-data $INPREFIX/Resources/kf5/kcharselect/

# misc
cp -v Info.plist $INPREFIX

# translation (locale)
cd $CRAFTROOT/share
$GCP -vf --parents locale/*/LC_MESSAGES/labplot2.mo ../../$INPREFIX/Resources
$GCP -vf --parents locale/*/LC_MESSAGES/kconfigwidgets5.mo ../../$INPREFIX/Resources
$GCP -vf --parents locale/*/LC_MESSAGES/kxmlgui5.mo ../../$INPREFIX/Resources
cd ../..

### TODO
# package icon
# share/doc

##########################################

# fix for hdf5 lib
# install_name_tool -change /usr/local/Cellar/hdf5/1.8.17/lib/libhdf5.10.dylib /usr/local/opt/hdf5/1.8.17/lib/libhdf5.10.dylib /usr/local/opt/hdf5/1.8.17/lib/libhdf5_hl.10.dylib

###############################################
if [ -d ./$TMPDIR ]; then
	rm -rf ./$TMPDIR/*
else
	mkdir ./$TMPDIR
fi
mv $PREFIX/$NAME.app ./$TMPDIR 

# Add link for easy install
ln -s /Applications ./$TMPDIR/Applications

## remove stuff we don't need or like
#rm -rf $TMPDIR/$NAME.app/Contents/Plugins/bearer

###############################################

# create the final disk image
echo "Building package"
rm -f ./labplot-$VERSION.dmg
hdiutil create -srcfolder ./$TMPDIR -format UDBZ -fs HFS+ -imagekey zlib-level=9 ./labplot-$VERSION.dmg

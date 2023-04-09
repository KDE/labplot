# Build it on Mac howto in script form
# but be aware, some frameworks need patching to have this working

# reference: https://cgit.kde.org/kate.git/tree/mac/emerge-deploy.sh
# run in kde/..

# errors are fatal
set -e

NAME=labplot2
PNAME=LabPlot2
VERSION=2.8.0
PREFIX=kde/Applications
INPREFIX=$PREFIX/$PNAME.app/Contents
TMPDIR=LabPlot2
SIGNATURE="Stefan Gerlach"

GCP=/opt/local/libexec/gnubin/cp

#########################################

echo "CLEAN UP"
rm -rf $PREFIX/$PNAME.app

mkdir -pv $INPREFIX/{Frameworks,Resources,MacOS,PlugIns/iconengines,share/appdata,share/applications}

# application
cp -v labplot/build/src/$NAME.app/Contents/MacOS/$NAME $INPREFIX/MacOS

echo "Running macdeployqt ..."
# -verbose=3
macdeployqt $PREFIX/$PNAME.app -verbose=2

#########################################

echo "install files"
# splash
cp -v kde/share/$NAME/splash.png $INPREFIX/Resources/
# rc-file
# Standardlocation (QSP): ~/Library/Application\ Support/kxmlgui5/labplot2/labplot2ui.rc
# using hardcoded path:
cp -v kde/share/kxmlgui5/$NAME/${NAME}ui.rc $INPREFIX/Resources/
# themes
cp -vr kde/share/$NAME/themes $INPREFIX/Resources/
# gsl_distros, fit_models, colorchooser
cp -vr kde/share/$NAME/pics $INPREFIX/Resources/
# color schemes (needs patched kcolorschememanager.cpp)
cp -vr kde/share/$NAME/color-schemes $INPREFIX/Resources/color-schemes
# appdata
cp -v kde/share/metainfo/org.kde.labplot2.appdata.xml $INPREFIX/share/appdata/
cp -v kde/share/applications/org.kde.$NAME.desktop $INPREFIX/share/applications/

# cantor
cp -v kde/Applications/cantor.app/Contents/MacOS/cantor $INPREFIX/MacOS
cp -v kde/Applications/cantor_scripteditor.app/Contents/MacOS/cantor_scripteditor $INPREFIX/MacOS
cp -vr kde/plugins/cantor $INPREFIX/PlugIns
cp -v kde/lib/libcantor_config.dylib $INPREFIX/Frameworks/
cp -v kde/lib/libcantor_pythonbackend.dylib $INPREFIX/Frameworks/

# icons
cp -vf kde/share/icontheme.rcc $INPREFIX/Resources/icontheme.rcc

# kcharselect data
mkdir -p $INPREFIX/Resources/kf5/kcharselect
cp -v kde/share/kf5/kcharselect/kcharselect-data $INPREFIX/Resources/kf5/kcharselect/

# misc
cp -v labplot/admin/Info.plist $INPREFIX
cp -v /Applications/KDE/labplot2.app/Contents/Resources/{LABPLOT_ICONS.icns,LML_ICONS.icns} $INPREFIX/Resources

# translation (locale)
cd kde/share
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
mv $PREFIX/$PNAME.app ./$TMPDIR 

ln -s /Applications ./$TMPDIR/Applications

## remove stuff we don't need or like
#rm -rf $TMPDIR/$PNAME.app/Contents/Plugins/bearer

###############################################

# create the final disk image
echo "BUILDING PACKAGE"
rm -f ./labplot-$VERSION.dmg
hdiutil create -srcfolder ./$TMPDIR -format UDBZ -fs HFS+ -imagekey zlib-level=9 ./labplot-$VERSION.dmg

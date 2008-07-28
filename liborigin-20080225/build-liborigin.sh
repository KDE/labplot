#!/bin/bash
# build liborigin RPM package

LVERSION=20080225
SPEC=liborigin.spec

################################
SRC=liborigin-"$LVERSION".tar.gz

if [ ! -f "$SRC" ] ; then
	echo "$SRC not found!"
	exit
fi

if [ ! -f "$SPEC" ] ; then
	echo "$SPEC not found!"
	exit
fi

echo "%_topdir /tmp/build" > ~/.rpmmacros

rm -rf /tmp/build/
mkdir -p /tmp/build/{SOURCES,SPECS,RPMS,SRPMS,BUILD} 

cp $SRC /tmp/build/SOURCES
cp $SPEC /tmp/build/SPECS

cd /tmp/build/SPECS/

rpmbuild -ba $SPEC


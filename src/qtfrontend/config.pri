################################################################################
### This file contains user-servicable build configuration for SciDAVis.
################################################################################

#CONFIG += release
### use the next line to compile with debugging symbols instead of the line above
CONFIG += debug
### use the next line in order to see as many compiler warnings as possible
CONFIG += warn_on

### modules to be compiled in
### basic modules you most probably don't want to exclude
MODULES = spreadsheet worksheet #notes matrix
### support for Python scripting
#MODULES += python

#MODULES += graph3D

### what to install and where
INSTALLS += documentation
unix: INSTALLBASE = /usr
win32: INSTALLBASE = c:/scidavis
unix: target.path = $$INSTALLBASE/bin
unix: documentation.path = $$INSTALLBASE/share/doc/scidavis
win32: target.path = $$INSTALLBASE
win32: documentation.path = $$INSTALLBASE/doc

### where to put temporary files (relative to subdirs)
TEMP_DIR       = ../tmp
MOC_DIR        = $$TEMP_DIR
OBJECTS_DIR    = $$TEMP_DIR
RCC_DIR			= $$TEMP_DIR
UI_DIR			= $$TEMP_DIR
SIP_DIR        = $$TEMP_DIR

### suffix for library paths
linux-g++-64: libsuff = 64

### GSL and zlib are required by core
### Link dynamically on Linux and MacOS X.
unix:LIBS          += -L /usr/lib$${libsuff}
unix:LIBS          += -lgsl -lgslcblas -lz
### Link statically on Windows.
win32:INCLUDEPATH  += c:/gsl/include
win32:INCLUDEPATH  += c:/zlib/include
win32:LIBS         += c:/gsl/lib/libgsl.a
win32:LIBS         += c:/gsl/lib/libgslcblas.a

### Apparently this works around a qmake bug. This option doesn't seem to be
### documented, so it _might_ cause trouble and you _might_ want to build
### without it.
QMAKE_PROJECT_DEPTH = 0

################################################################################
### ORIGIN MODULE OPTIONS
################################################################################

contains(MODULES, origin) {

### Note to packagers: If you want to use systemwide installed liborigin
### instead of the one provided in "3rdparty", uncomment the following 2 lines:
# CONFIG += dynamic_liborigin
# LIBS += -lorigin
### Unfortunately, due to liborigin being in alpha stage, we cannot guarantee
### that SciDAVis works with any other version than the one in "3rdparty".

}

################################################################################
### GRAPH MODULE OPTIONS
################################################################################

contains(MODULES, graph) {

### Default for Linux / Mac OS X:
### Link statically againts Qwt (in order to make sure it's compiled against
### Qt4).
unix:INCLUDEPATH  += 3rdparty/qwt/src
unix:LIBS         += 3rdparty/qwt/lib/libqwt.a

### Link dynamically against system-wide installation of Qwt.
### WARNING: Make sure Qwt is compiled against Qt >= 4.2 if you use this.
### WARNING: Mixing Qt 4.2 and Qt >= 4.3 compiled stuff may also cause
### WARNING: problems.
#unix:INCLUDEPATH	+= /usr/include/qwt5
#unix:LIBS				+= -lqwt

### Paths to Qwt on Windows
win32:INCLUDEPATH  += c:/qwt-5.0.2/include
win32:LIBS         += c:/qwt-5.0.2/lib/libqwt.a

}

################################################################################
### GRAPH-3D MODULE OPTIONS
################################################################################

contains(MODULES, graph3D) {

### Default for Linux / Mac OS X:
### Link statically againts Qwtplot3D (in order to make sure it's compiled
### against Qt4).

#unix:INCLUDEPATH  += 3rdparty/qwtplot3d/include
#unix:LIBS         += 3rdparty/qwtplot3d/lib/libqwtplot3d.a
#unix:LIBS			 += -lmuparser # the dependency on muparser needs to be removed
#unix:LIBS			 += -lmuparserd # the dependency on muparser needs to be removed
unix:LIBS         += /usr/local/lib/libmuparser.a
unix:INCLUDEPATH  += /usr/local/include

### Link dynamically against system-wide installation of Qwtplot3D.
### WARNING: Make sure Qwtplot3D is compiled against Qt >= 4.2 if you use this.
### WARNING: Mixing Qt 4.2 and Qt >= 4.3 compiled stuff may also cause
### WARNING: problems.
#unix:LIBS		+= -lqwtplot3d

### use this on Debian
unix:INCLUDEPATH  += /usr/include/qwtplot3d-qt4
unix:LIBS		+= -lqwtplot3d-qt4


### Paths to Qwtplot3D on Windows (it seems to be impossible to link this
### statically).
win32:INCLUDEPATH  += c:/qwtplot3d/include
win32:LIBS         += c:/qwtplot3d/lib/qwtplot3d.dll

}

################################################################################
### MUPARSER MODULE OPTIONS
################################################################################

contains(MODULES, muparser) {

### You probably don't need to change this on Linux and Mac OS X.
#unix:LIBS			 += -lmuparser
unix:LIBS         += -L/usr/local/lib$${libsuff}
unix:LIBS         += /usr/local/lib/libmuparser.a
unix:INCLUDEPATH  += /usr/local/include

### Path to muParser on Windows.
win32:INCLUDEPATH  += c:/muparser/include
win32:LIBS         += c:/muparser/lib/libmuparser.a

}

################################################################################
### PYTHON MODULE OPTIONS
################################################################################

contains(MODULES, python) {

### paths of the Python installation are autodetected

### where to put scidavisrc.py
INSTALLS += python_config
unix: python_config.path = /etc
win32: python_config.path = $$INSTALLBASE

### where to put scidavisUtil.py
INSTALLS += python_shared
unix: python_shared.path = $$INSTALLBASE/share/scidavis
win32: python_shared.path = $$INSTALLBASE

}

################################################################################
DEFINES += ACTIVATE_SCIDAVIS_SPECIFIC_CODE
DEFINES += SUPPRESS_SCRIPTING_INIT



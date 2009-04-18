TEMPLATE = lib
CONFIG += plugin
DEPENDPATH += . 
INCLUDEPATH += . ../../backend ../../qtfrontend
TARGET = ../$$qtLibraryTarget(scidavis_helloworldplugin)

DEFINES += ACTIVATE_SCIDAVIS_SPECIFIC_CODE

RESOURCES += helloworld.qrc

HEADERS += \
	HelloWorldPlugin.h \
	
SOURCES += \
	HelloWorldPlugin.cpp \


include(../config.pri)
TEMPLATE = lib
CONFIG += plugin static
DEPENDPATH += . .. ../../backend ../../commonfrontend ../../commonfrontend/worksheet ../../backend/worksheet ../core ../../backend/core
INCLUDEPATH += .. ../../backend ../../commonfrontend
TARGET = ../$$qtLibraryTarget(scidavis_worksheet)
QT += xml

debug {
	CONFIG -= static
	DEFINES += QT_STATICPLUGIN
}

#FORMS += controltabs.ui \

HEADERS += \
	WorksheetModule.h \
	WorksheetView.h \
	WorksheetModel.h \
	WorksheetGraphicsScene.h \
	Worksheet.h \
	AbstractWorksheetElement.h \
	AbstractPlot.h \
	DecorationPlot.h \
	WorksheetElementContainer.h \
	WorksheetElementContainerPrivate.h \
	WorksheetElementGroup.h \
	WorksheetRectangleElement.h \
	AbstractCoordinateSystem.h \
	CartesianCoordinateSystem.h \
	Axis.h \
	AxisPrivate.h \
	LineSymbolCurve.h \
	AbstractCurveSymbol.h \
	interfaces.h \
	symbols/EllipseCurveSymbol.h \
	StandardCurveSymbolFactory.h \
	symbols/PathCurveSymbol.h \
	symbols/PathCurveSymbolPrivate.h \
	AbstractWorksheetDecorationElement.h \
	PlotArea.h \
	PlotAreaPrivate.h \
	ScalableTextLabel.h \
	
SOURCES += \
	WorksheetModule.cpp \
	WorksheetView.cpp \
	Worksheet.cpp \
	WorksheetModel.cpp \
	WorksheetGraphicsScene.cpp \
	AbstractWorksheetElement.cpp \
	AbstractPlot.cpp \
	DecorationPlot.cpp \
	WorksheetElementContainer.cpp \
	WorksheetElementGroup.cpp \
	WorksheetRectangleElement.cpp \
	AbstractCoordinateSystem.cpp \
	CartesianCoordinateSystem.cpp \
	Axis.cpp \
	LineSymbolCurve.cpp \
	AbstractCurveSymbol.cpp \
	symbols/EllipseCurveSymbol.cpp \
	StandardCurveSymbolFactory.cpp \
	symbols/PathCurveSymbol.cpp \
	AbstractWorksheetDecorationElement.cpp \ 
	PlotArea.cpp \
	ScalableTextLabel.cpp \


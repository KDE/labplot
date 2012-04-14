include(../config.pri)
TEMPLATE = lib
CONFIG += plugin static
DEPENDPATH += . .. ../../backend ../../commonfrontend ../../commonfrontend/worksheet ../../backend/worksheet ../core ../../backend/core
INCLUDEPATH += .. ../../backend ../../commonfrontend
TARGET = ../$$qtLibraryTarget(scidavis_worksheet)
QT += xml svg

debug {
	CONFIG -= static
	DEFINES += QT_STATICPLUGIN
}

#FORMS += controltabs.ui \

HEADERS += \
	interfaces.h \
	AbstractWorksheetElement.h \
#	AxisPrivate.h \
	AbstractWorksheetDecorationElement.h \
	AbstractCurveSymbol.h \
#	CartesianCoordinateSystem.h \
#	DecorationPlot.h \
#	PlotAreaPrivate.h \
	StandardCurveSymbolFactory.h \
	TextLabel.h \
	WorksheetModule.h \
	WorksheetView.h \
	WorksheetModel.h \
	WorksheetGraphicsScene.h \
	Worksheet.h \
	WorksheetElementContainer.h \
	WorksheetElementContainerPrivate.h \
	WorksheetElementGroup.h \
	WorksheetRectangleElement.h \
	plots/AbstractCoordinateSystem.h \
	plots/AbstractPlot.h \
	plots/cartesian/Axis.h \
	plots/cartesian/XYCurve.h \
	plots/cartesian/CartesianPlot.h \
	plots/PlotArea.h \
	symbols/EllipseCurveSymbol.h \
	symbols/PathCurveSymbol.h \
	symbols/PathCurveSymbolPrivate.h \
	../tools/TexRenderer.h
	
SOURCES += \
	AbstractCurveSymbol.cpp \
	AbstractWorksheetDecorationElement.cpp \ 
	AbstractWorksheetElement.cpp \
#	DecorationPlot.cpp \
	StandardCurveSymbolFactory.cpp \
	TextLabel.cpp \
	WorksheetModule.cpp \
	WorksheetView.cpp \
	Worksheet.cpp \
	WorksheetModel.cpp \
	WorksheetGraphicsScene.cpp \
	WorksheetElementContainer.cpp \
	WorksheetElementGroup.cpp \
	WorksheetRectangleElement.cpp \
	plots/AbstractCoordinateSystem.cpp \
	plots/AbstractPlot.cpp \
	plots/PlotArea.cpp \
	plots/cartesian/CartesianCoordinateSystem.cpp \
	plots/cartesian/Axis.cpp \
	plots/cartesian/XYCurve.cpp \
	plots/cartesian/CartesianPlot.cpp \
	symbols/EllipseCurveSymbol.cpp \
	symbols/PathCurveSymbol.cpp \
	../tools/TexRenderer.cpp


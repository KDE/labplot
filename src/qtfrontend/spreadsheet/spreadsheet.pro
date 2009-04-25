include(../config.pri)
TEMPLATE = lib
CONFIG += plugin static
DEPENDPATH += . .. ../../backend ../../backend/spreadsheet ../core ../../backend/core
INCLUDEPATH += .. ../../backend
TARGET = ../$$qtLibraryTarget(scidavis_spreadsheet)
QT += xml

debug {
	CONFIG -= static
	DEFINES += QT_STATICPLUGIN
}

FORMS += SpreadsheetControlTabs.ui \
	SpreadsheetConfigPage.ui \
	DimensionsDialog.ui \

HEADERS += \
	SpreadsheetModule.h \
	SpreadsheetView.h \
	SpreadsheetItemDelegate.h \
	SpreadsheetModel.h \
	Spreadsheet.h \
	SortDialog.h \
	SpreadsheetDoubleHeaderView.h \
	SpreadsheetCommentsHeaderModel.h  \
	AsciiSpreadsheetImportFilter.h \
	AbstractScriptingEngine.h \
	
	# TODO: port or remove the following files
	#ExportSpreadsheetDialog.h \
	#ImportSpreadsheetDialog.h \
	#SetColValuesDialog.h \
	#SpreadsheetDialog.h \

SOURCES += \
	SpreadsheetModule.cpp \
	SpreadsheetView.cpp \
	SpreadsheetItemDelegate.cpp \
	Spreadsheet.cpp \
	SpreadsheetModel.cpp \
	SortDialog.cpp \
	SpreadsheetDoubleHeaderView.cpp \
	SpreadsheetCommentsHeaderModel.cpp  \
	AsciiSpreadsheetImportFilter.cpp \
	AbstractScriptingEngine.cpp \

	# TODO: port or remove the following files
	#ExportSpreadsheetDialog.cpp \ -> superseded by core/ExportDialog (yet to be written)
	#ImportSpreadsheetDialog.cpp \ -> superseded by core/ImportDialog
	#SetColValuesDialog.cpp \
	#SpreadsheetDialog.cpp \


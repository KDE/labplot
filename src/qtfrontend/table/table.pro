include(../config.pri)
TEMPLATE = lib
CONFIG += plugin static
DEPENDPATH += . .. ../../backend ../../backend/table ../core ../../backend/core
INCLUDEPATH += .. ../../backend
TARGET = ../$$qtLibraryTarget(scidavis_table)
QT += xml

debug {
	CONFIG -= static
	DEFINES += QT_STATICPLUGIN
}

FORMS += controltabs.ui \
	TableConfigPage.ui \
	DimensionsDialog.ui \

HEADERS += \
	TableModule.h \
	TableView.h \
	TableItemDelegate.h \
	TableModel.h \
	Table.h \
	tablecommands.h \
	SortDialog.h \
	TableDoubleHeaderView.h \
	TableCommentsHeaderModel.h  \
	AsciiTableImportFilter.h \
	AbstractScriptingEngine.h \
	
	# TODO: port or remove the following files
	#ExportTableDialog.h \
	#ImportTableDialog.h \
	#SetColValuesDialog.h \
	#TableDialog.h \

SOURCES += \
	TableModule.cpp \
	TableView.cpp \
	TableItemDelegate.cpp \
	Table.cpp \
	tablecommands.cpp \
	TableModel.cpp \
	SortDialog.cpp \
	TableDoubleHeaderView.cpp \
	TableCommentsHeaderModel.cpp  \
	AsciiTableImportFilter.cpp \
	AbstractScriptingEngine.cpp \

	# TODO: port or remove the following files
	#ExportTableDialog.cpp \ -> superseded by core/ExportDialog (yet to be written)
	#ImportTableDialog.cpp \ -> superseded by core/ImportDialog
	#SetColValuesDialog.cpp \
	#TableDialog.cpp \


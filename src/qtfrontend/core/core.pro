include(../config.pri)
TEMPLATE = app
TARGET = ../scidavis
DEPENDPATH += . .. ../../backend ../../backend/core ../../backend/core/column  ../../backend/core/datatypes ../../backend/core/filters ../../backend/core/plugin
INCLUDEPATH += .. ../../backend
QT += xml network
CONFIG += assistant

# For debugging purposes, link modules dynamically and make sure they are found in the directory
# containing the executable. This allows testing changes to one module without re-linking the
# application.
debug:unix:LIBS += -Wl,-rpath,\'\$$ORIGIN\'

# link in modules
LIBS += -L..
for(mod, MODULES):LIBS += -lscidavis_$${mod}

# Changes to config.pri could mean MODULES changed, in which case staticplugins.cpp needs to be
# recompiled. Therefore, this file is not included in SOURCES but recieves special handling.
# Mostly this is supposed to mimic qmake's default handling of C++ source files, except for
# declaring config.pri as additional dependency and a preprocessor macro containing the code to
# register the modules with Qt's plugin system.
unix:staticplugins.target = $${OBJECTS_DIR}/staticplugins.o
win32:staticplugins.target = $${OBJECTS_DIR}/staticplugins.obj
staticplugins.depends = staticplugins.cpp ../config.pri
for(mod, MODULES):mods += Q_IMPORT_PLUGIN(scidavis_$${mod})
staticplugins.commands = $(CXX) -c $(CXXFLAGS) -DIMPORT_SCIDAVIS_MODULES='\'$${mods}\'' $(INCPATH) -o $$staticplugins.target staticplugins.cpp
QMAKE_EXTRA_TARGETS += staticplugins
OBJECTS += $$staticplugins.target

# ICONS
RESOURCES += \
	../appicons.qrc \
	../icons.qrc \

FORMS += \
	ProjectConfigPage.ui \

HEADERS += \
	globals.h \
	customevents.h \
	AbstractAspect.h \
	aspectcommands.h \
	AspectPrivate.h \
	AbstractColumn.h \
	AbstractColumnPrivate.h \
	abstractcolumncommands.h \
	Column.h \
	ColumnPrivate.h \
	columncommands.h \
	AbstractFilter.h \
	AbstractSimpleFilter.h \
	SimpleCopyThroughFilter.h \
	DateTime2DoubleFilter.h \
	DateTime2StringFilter.h \
	DayOfWeek2DoubleFilter.h \
	Double2DateTimeFilter.h \
	Double2DayOfWeekFilter.h \
	Double2MonthFilter.h \
	Double2StringFilter.h \
	Month2DoubleFilter.h \
	String2DateTimeFilter.h \
	String2DayOfWeekFilter.h \
	String2DoubleFilter.h \
	String2MonthFilter.h \
	interfaces.h \
	AbstractScriptingEngine.h \
	AbstractScript.h \
	ScriptingEngineManager.h \
	ScriptEdit.h \
	Project.h \
	Folder.h \
	ProjectWindow.h \
	AspectTreeModel.h \
	AbstractPart.h \
	PartMdiView.h \
	ProjectExplorer.h \
	AbstractImportFilter.h \
	AbstractExportFilter.h \
	ImportDialog.h \
	ProjectConfigPage.h \
	PluginManager.h \
	PluginLoader.h \
	datasources/AbstractDataSource.h \
	datasources/FileDataSource.h \
	datasources/filters/AbstractFileFilter.h \
	datasources/filters/AsciiFilter.h \
	datasources/filters/AsciiFilterPrivate.h \
	# TODO: port or delete the following files
	#PreferencesDialog.h \
	#CopyThroughFilter.h \
	#CurveRangeDialog.h \
	#DataSetDialog.h \
	#FilterDialog.h \
	#FindWindowDialog.h \
	#FitDialog.h \
	#OpenProjectDialog.h \
	#ReadOnlyTableModel.h \
	#RenameWindowDialog.h \
	#ScriptingLangDialog.h \

SOURCES += \
	main.cpp \
	Folder.cpp \
	AbstractAspect.cpp \
	AspectPrivate.cpp \
	AbstractColumn.cpp \
	AbstractColumnPrivate.cpp \
	abstractcolumncommands.cpp \
	globals.cpp \
	AbstractFilter.cpp \
	AbstractSimpleFilter.cpp \
	Column.cpp \
	ColumnPrivate.cpp \
	columncommands.cpp \
	AbstractScriptingEngine.cpp \
	AbstractScript.cpp \
	ScriptingEngineManager.cpp \
	ScriptEdit.cpp \
	Project.cpp \
	ProjectWindow.cpp \
	AspectTreeModel.cpp \
	AbstractPart.cpp \
	PartMdiView.cpp \
	ProjectExplorer.cpp \
	DateTime2StringFilter.cpp \
	String2DateTimeFilter.cpp \
	Double2StringFilter.cpp \
	AbstractImportFilter.cpp \
	AbstractExportFilter.cpp \
	ImportDialog.cpp \
	ProjectConfigPage.cpp \
	PluginManager.cpp \
	PluginLoader.cpp \
	datasources/AbstractDataSource.cpp \
	datasources/FileDataSource.cpp \
	datasources/filters/AsciiFilter.cpp \
	# TODO: port or delete the following files
	#PreferencesDialog.cpp \
	#CurveRangeDialog.cpp \
	#DataSetDialog.cpp \
	#FilterDialog.cpp \
	#FindWindowDialog.cpp \
	#FitDialog.cpp \
	#OpenProjectDialog.cpp \
	#ReadOnlyTableModel.cpp \
	#ScriptingLangDialog.cpp \
	#../3rdparty/minigzip/minigzip.c -> maybe we can do without this

SOURCES += \
	../lib/ColorBox.cpp \
	../lib/ColorButton.cpp \
	../lib/ExtensibleFileDialog.cpp \
	../lib/PatternBox.cpp \
	../lib/SymbolDialog.cpp \
	../lib/TextDialog.cpp \
	../lib/TextFormatButtons.cpp \
	../lib/ImageExportDialog.cpp \
    ../lib/ShortcutsDialogModel.cpp \
    ../lib/RecordShortcutDelegate.cpp \
    ../lib/ActionManager.cpp \
    ../lib/ShortcutsDialog.cpp \
    ../lib/ConfigPageWidget.cpp \
	../lib/XmlStreamReader.cpp \

HEADERS += \
	../lib/ColorBox.h \
	../lib/ColorButton.h \
	../lib/ExtensibleFileDialog.h \
	../lib/Interval.h \
	../lib/IntervalAttribute.h \
	../lib/PatternBox.h \
	../lib/SymbolDialog.h \
	../lib/TextDialog.h \
	../lib/TextFormatButtons.h \
	../lib/ImageExportDialog.h \
    ../lib/ShortcutsDialogModel.h \
    ../lib/RecordShortcutDelegate.h \
    ../lib/ActionManager.h \
    ../lib/ShortcutsDialog.h \
    ../lib/ConfigPageWidget.h \
	../lib/XmlStreamReader.h \


set(KDEFRONTEND_DIR kdefrontend)
set(BACKEND_DIR backend)
set(QTFRONTEND_DIR qtfrontend)
set(COMMONFRONTEND_DIR commonfrontend)
set(DATASOURCES_DIR ${KDEFRONTEND_DIR}/datasources)

ADD_SUBDIRECTORY(elements)
ADD_SUBDIRECTORY(filter)

set(GUI_SOURCES
	${KDEFRONTEND_DIR}/AxesDialog.cpp
	${KDEFRONTEND_DIR}/AxesWidget.cpp
	${KDEFRONTEND_DIR}/ColorMapPreview.cpp
	${KDEFRONTEND_DIR}/DataPlotDialog.cpp
	${KDEFRONTEND_DIR}/ExportDialog.cpp
	${KDEFRONTEND_DIR}/ExportWidget.cpp
	${KDEFRONTEND_DIR}/FunctionPlotDialog.cpp
  	${KDEFRONTEND_DIR}/FunctionWidget.cpp
  	${KDEFRONTEND_DIR}/GuiObserver.cpp
	${KDEFRONTEND_DIR}/ImportDialog.cpp
	${KDEFRONTEND_DIR}/ImportWidget.cpp
	${KDEFRONTEND_DIR}/LabelWidget.cpp
 	${KDEFRONTEND_DIR}/LabPlot.cpp
	${KDEFRONTEND_DIR}/LegendDialog.cpp
	${KDEFRONTEND_DIR}/LegendWidget.cpp
  	${KDEFRONTEND_DIR}/MainWin.cpp
  	${KDEFRONTEND_DIR}/PlotStyleWidget.cpp
 	${KDEFRONTEND_DIR}/PlotSurfaceStyleWidget.cpp
	${KDEFRONTEND_DIR}/ProjectDialog.cpp
	${KDEFRONTEND_DIR}/SettingsDialog.cpp
	${KDEFRONTEND_DIR}/SettingsGeneralPage.cpp
	${KDEFRONTEND_DIR}/SettingsPage.cpp
	${KDEFRONTEND_DIR}/SettingsPrintingPage.cpp
#   	${KDEFRONTEND_DIR}/SpreadsheetView.cpp
	${KDEFRONTEND_DIR}/TitleDialog.cpp
	${KDEFRONTEND_DIR}/ValuesWidget.cpp
	${KDEFRONTEND_DIR}/dockwidgets/AxisDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ColumnDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/SpreadsheetDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/LineSymbolCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/WorksheetDock.cpp
)

set(PLOTS_SOURCES
	plots/Plot.cpp
	plots/Plot2D.cpp
	plots/Plot2DSimple.cpp
)

set(UI_SOURCES
	${KDEFRONTEND_DIR}/ui/axeswidget.ui
	${KDEFRONTEND_DIR}/ui/columndialog.ui
	${KDEFRONTEND_DIR}/ui/exportwidget.ui
	${KDEFRONTEND_DIR}/ui/functionwidget.ui
	${KDEFRONTEND_DIR}/ui/importwidget.ui
	${KDEFRONTEND_DIR}/ui/labelwidget.ui
	${KDEFRONTEND_DIR}/ui/legendwidget.ui
	${KDEFRONTEND_DIR}/ui/plotstylewidget.ui
 	${KDEFRONTEND_DIR}/ui/plotsurfacestylewidget.ui
 	${KDEFRONTEND_DIR}/ui/projectdialog.ui
	${KDEFRONTEND_DIR}/ui/settingsgeneralpage.ui
	${KDEFRONTEND_DIR}/ui/settingsprintingpage.ui
	${KDEFRONTEND_DIR}/ui/valueswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/asciioptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/binaryoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importfilewidget.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/axisdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/columndock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/linesymbolcurvedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/spreadsheetdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/worksheetdock.ui
)

set(BACKEND_SOURCES
	${BACKEND_DIR}/core/Folder.cpp
	${BACKEND_DIR}/core/AbstractAspect.cpp
	${BACKEND_DIR}/core/AspectPrivate.cpp
	${BACKEND_DIR}/core/globals.cpp
	${BACKEND_DIR}/core/AbstractColumn.cpp
	${BACKEND_DIR}/core/AbstractColumnPrivate.cpp
	${BACKEND_DIR}/core/abstractcolumncommands.cpp
	${BACKEND_DIR}/core/AbstractFilter.cpp
	${BACKEND_DIR}/core/AbstractSimpleFilter.cpp
	${BACKEND_DIR}/core/column/Column.cpp
	${BACKEND_DIR}/core/column/ColumnPrivate.cpp
	${BACKEND_DIR}/core/column/columncommands.cpp
	${BACKEND_DIR}/core/AbstractScriptingEngine.cpp
	${BACKEND_DIR}/core/AbstractScript.cpp
	${BACKEND_DIR}/core/ScriptingEngineManager.cpp
	${BACKEND_DIR}/core/Project.cpp
	${BACKEND_DIR}/core/AbstractPart.cpp
	${BACKEND_DIR}/core/AspectTreeModel.cpp
	${BACKEND_DIR}/core/datatypes/DateTime2StringFilter.cpp
	${BACKEND_DIR}/core/datatypes/String2DateTimeFilter.cpp
	${BACKEND_DIR}/core/datatypes/Double2StringFilter.cpp
	${BACKEND_DIR}/core/plugin/PluginLoader.cpp
	${BACKEND_DIR}/core/plugin/PluginManager.cpp
	${BACKEND_DIR}/datasources/AbstractDataSource.cpp
	${BACKEND_DIR}/datasources/FileDataSource.cpp
	${BACKEND_DIR}/datasources/filters/AsciiFilter.cpp
	#${BACKEND_DIR}/datasources/filters/BinaryFilter.cpp
	${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
	${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
	${BACKEND_DIR}/lib/XmlStreamReader.cpp
	${BACKEND_DIR}/widgets/TreeViewComboBox.cpp
	${BACKEND_DIR}/worksheet/Worksheet.cpp
	${BACKEND_DIR}/worksheet/WorksheetModel.cpp
	${BACKEND_DIR}/worksheet/WorksheetGraphicsScene.cpp
	${BACKEND_DIR}/worksheet/DecorationPlot.cpp
	${BACKEND_DIR}/worksheet/WorksheetRectangleElement.cpp
	${BACKEND_DIR}/worksheet/AbstractWorksheetElement.cpp
	${BACKEND_DIR}/worksheet/AbstractPlot.cpp
	${BACKEND_DIR}/worksheet/LinearAxis.cpp
	${BACKEND_DIR}/worksheet/LogAxis.cpp
	${BACKEND_DIR}/worksheet/AbstractCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/CartesianCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementGroup.cpp
	${BACKEND_DIR}/worksheet/LineSymbolCurve.cpp
	${BACKEND_DIR}/worksheet/AbstractCurveSymbol.cpp
	${BACKEND_DIR}/worksheet/symbols/EllipseCurveSymbol.cpp
	${BACKEND_DIR}/worksheet/symbols/PathCurveSymbol.cpp
	${BACKEND_DIR}/worksheet/StandardCurveSymbolFactory.cpp
	${BACKEND_DIR}/worksheet/AbstractWorksheetDecorationElement.cpp
	${BACKEND_DIR}/worksheet/PlotArea.cpp
	${BACKEND_DIR}/worksheet/ScalableTextLabel.cpp
	${BACKEND_DIR}/worksheet/DropLineCurve.cpp
	${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
)

set(DATASOURCES_SOURCES
	${DATASOURCES_DIR}/AsciiOptionsWidget.cpp
	${DATASOURCES_DIR}/BinaryOptionsWidget.cpp
	${DATASOURCES_DIR}/ImportFileWidget.cpp
	${DATASOURCES_DIR}/ImportFileDialog.cpp
	${DATASOURCES_DIR}/FileInfoDialog.cpp
)

set(QTFRONTEND_SOURCES
	${QTFRONTEND_DIR}/core/ProjectExplorer.cpp
	${QTFRONTEND_DIR}/core/PartMdiView.cpp
	${QTFRONTEND_DIR}/lib/ActionManager.cpp
)

set(COMMONFRONTEND_SOURCES
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetCommentsHeaderModel.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetDoubleHeaderView.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetItemDelegate.cpp
# 	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetModule.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetView.cpp
	${COMMONFRONTEND_DIR}/worksheet/WorksheetView.cpp
)

set(QTMOC_HDRS
	${BACKEND_DIR}/core/datatypes/DateTime2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/DayOfWeek2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/Double2DateTimeFilter.h
	${BACKEND_DIR}/core/datatypes/Double2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/Double2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/Month2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/SimpleCopyThroughFilter.h
	${BACKEND_DIR}/core/datatypes/String2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/String2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/String2MonthFilter.h
)


qt4_wrap_cpp(QTMOC_HDRS ${QTMOC_HDRS})

add_custom_command (OUTPUT parser/parser.tab.c
                   COMMAND ${YACC_EXECUTABLE} ARGS -b parser parser.y
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/parser)
##############################################################################
INCLUDE_DIRECTORIES(${BACKEND_DIR} ${QTFRONTEND_DIR} ${COMMONFRONTEND_DIR} .)
set(LABPLOT_SRCS ${GUI_SOURCES} ${PLOTS_SOURCES} parser/parser.tab.c)

kde4_add_ui_files(LABPLOT_SRCS ${UI_SOURCES})
kde4_add_executable(labplot2 ${LABPLOT_SRCS} ${BACKEND_SOURCES} ${DATASOURCES_SOURCES} ${COMMONFRONTEND_SOURCES} ${QTFRONTEND_SOURCES} ${QTMOC_HDRS})
target_link_libraries(labplot2 labplot_elements labplot_filter ${KDE4_KDEUI_LIBS} ${KDE4_KIO_LIBS} 
${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${OPJ_LIBRARY})

############## installation ################################

install(TARGETS labplot2 DESTINATION ${BIN_INSTALL_DIR})
# install(FILES LabPlotui.rc DESTINATION  ${DATA_INSTALL_DIR}/LabPlot)

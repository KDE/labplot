find_package(SharedMimeInfo REQUIRED)
set(KDE_FRONTEND true)
set(KDEFRONTEND_DIR kdefrontend)
set(BACKEND_DIR backend)
set(COMMONFRONTEND_DIR commonfrontend)
set(TOOLS_DIR tools)
set(CMAKE_AUTOMOC ON)

set(GUI_SOURCES
  	${KDEFRONTEND_DIR}/GuiObserver.cpp
	${KDEFRONTEND_DIR}/GuiTools.cpp
	${KDEFRONTEND_DIR}/HistoryDialog.cpp
	${KDEFRONTEND_DIR}/LabPlot.cpp
  	${KDEFRONTEND_DIR}/MainWin.cpp
	${KDEFRONTEND_DIR}/SettingsDialog.cpp
	${KDEFRONTEND_DIR}/SettingsGeneralPage.cpp
	${KDEFRONTEND_DIR}/SettingsPage.h
	${KDEFRONTEND_DIR}/TemplateHandler.cpp
	${KDEFRONTEND_DIR}/datasources/AsciiOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/BinaryOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportFileWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportFileDialog.cpp
	${KDEFRONTEND_DIR}/datasources/FileInfoDialog.cpp
	${KDEFRONTEND_DIR}/dockwidgets/AxisDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotLegendDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ColumnDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ProjectDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/SpreadsheetDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFitCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYEquationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/WorksheetDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CantorWorksheetDock.cpp
	${KDEFRONTEND_DIR}/spreadsheet/EquidistantValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/ExportSpreadsheetDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/FunctionValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/RandomValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/SortDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/ExportWorksheetDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/GridDialog.cpp
	${KDEFRONTEND_DIR}/widgets/ConstantsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/ExpressionTextEdit.cpp
	${KDEFRONTEND_DIR}/widgets/FitOptionsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FitParametersWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FunctionsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/LabelWidget.cpp
)

set(UI_SOURCES
	${KDEFRONTEND_DIR}/ui/constantswidget.ui
	${KDEFRONTEND_DIR}/ui/functionswidget.ui
	${KDEFRONTEND_DIR}/ui/fitoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/fitparameterswidget.ui
	${KDEFRONTEND_DIR}/ui/labelwidget.ui
	${KDEFRONTEND_DIR}/ui/settingsgeneralpage.ui
	${KDEFRONTEND_DIR}/ui/settingsprintingpage.ui
	${KDEFRONTEND_DIR}/ui/datasources/asciioptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/binaryoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importfilewidget.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/axisdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotlegenddock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/columndock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/projectdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/spreadsheetdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycurvedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfitcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyequationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/worksheetdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cantorworksheetdock.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/equidistantvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/exportspreadsheetwidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/functionvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/randomvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/worksheet/exportworksheetwidget.ui
)

set(BACKEND_SOURCES
	${BACKEND_DIR}/core/Folder.cpp
	${BACKEND_DIR}/core/AbstractAspect.cpp
	${BACKEND_DIR}/core/AspectPrivate.cpp
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
	${BACKEND_DIR}/core/datatypes/DateTime2StringFilter.cpp
	${BACKEND_DIR}/core/datatypes/String2DateTimeFilter.cpp
	${BACKEND_DIR}/core/datatypes/Double2StringFilter.cpp
	${BACKEND_DIR}/core/plugin/PluginLoader.cpp
	${BACKEND_DIR}/core/plugin/PluginManager.cpp
	${BACKEND_DIR}/datasources/AbstractDataSource.cpp
	${BACKEND_DIR}/datasources/FileDataSource.cpp
	${BACKEND_DIR}/datasources/filters/AbstractFileFilter.cpp
	${BACKEND_DIR}/datasources/filters/AsciiFilter.cpp
	#${BACKEND_DIR}/datasources/filters/BinaryFilter.cpp
	${BACKEND_DIR}/gsl/ExpressionParser.cpp
 	${BACKEND_DIR}/gsl/parser.tab.c
	${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
	${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
	${BACKEND_DIR}/lib/XmlStreamReader.cpp
	${BACKEND_DIR}/worksheet/WorksheetElement.cpp
	${BACKEND_DIR}/worksheet/TextLabel.cpp
	${BACKEND_DIR}/worksheet/Worksheet.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementGroup.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFitCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
	${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
	${BACKEND_DIR}/cantorWorksheet/CantorWorksheet.cpp
)

set(COMMONFRONTEND_SOURCES
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetCommentsHeaderModel.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetDoubleHeaderView.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetItemDelegate.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetView.cpp
	${COMMONFRONTEND_DIR}/worksheet/WorksheetView.cpp
	${COMMONFRONTEND_DIR}/ProjectExplorer.cpp
	${COMMONFRONTEND_DIR}/core/PartMdiView.cpp
	${COMMONFRONTEND_DIR}/widgets/TreeViewComboBox.cpp
	${COMMONFRONTEND_DIR}/cantorWorksheet/CantorWorksheetView.cpp
)

set(CANTOR_SOURCES
	${CANTOR_DIR}/CantorWorksheetViewHolder.cpp
	${CANTOR_DIR}/CantorWorksheetScene.cpp
	${CANTOR_DIR}/epsrenderer.cpp
	${CANTOR_DIR}/latexrenderer.cpp
	${CANTOR_DIR}/worksheetcursor.cpp
	${CANTOR_DIR}/commandentry.cpp
	${CANTOR_DIR}/worksheetentry.cpp
	${CANTOR_DIR}/worksheettextitem.cpp
	${CANTOR_DIR}/resultitem.cpp
	${CANTOR_DIR}/loadedexpression.cpp
	${CANTOR_DIR}/textentry.cpp
	${CANTOR_DIR}/latexentry.cpp
	${CANTOR_DIR}/imageentry.cpp
	${CANTOR_DIR}/pagebreakentry.cpp
	${CANTOR_DIR}/imagesettingsdialog.cpp
	${CANTOR_DIR}/placeholderentry.cpp
	${CANTOR_DIR}/actionbar.cpp
	${CANTOR_DIR}/worksheettoolbutton.cpp
	${CANTOR_DIR}/textresultitem.cpp
	${CANTOR_DIR}/imageresultitem.cpp
	${CANTOR_DIR}/worksheetimageitem.cpp
	${CANTOR_DIR}/animationresultitem.cpp
)

set(TOOLS_SOURCES
	${TOOLS_DIR}/TeXRenderer.cpp
	${TOOLS_DIR}/EquationHighlighter.cpp
)

add_subdirectory( pics )
add_subdirectory( pics/gsl_distributions )

##############################################################################
# set(LABPLOT_SRCS ${GUI_SOURCES} ${PLOTS_SOURCES} parser/parser.tab.c)
set(LABPLOT_SRCS ${GUI_SOURCES} ${PLOTS_SOURCES})
INCLUDE_DIRECTORIES(. ${GSL_INCLUDE_DIR} ${GSL_INCLUDEDIR}/..)
qt5_wrap_ui(LABPLOT_SRCS ${UI_SOURCES})
add_executable(labplot2 ${LABPLOT_SRCS} ${BACKEND_SOURCES} ${DATASOURCES_SOURCES} ${COMMONFRONTEND_SOURCES} ${TOOLS_SOURCES} ${QTMOC_HDRS})
target_link_libraries(labplot2 KF5::KDELibs4Support KF5::Parts KF5::Archive Qt5::Svg KF5::WidgetsAddons KF5::KIOWidgets KF5::XmlGui ${Qt5XmlPatterns_LIBRARIES} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${CANTOR_LIBS})
# ${OPJ_LIBRARY}

############## installation ################################

install(TARGETS labplot2 DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES ${KDEFRONTEND_DIR}/LabPlot2ui.rc ${KDEFRONTEND_DIR}/splash.png DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/${PROJECT_NAME})
install(PROGRAMS LabPlot2.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES labplot2.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
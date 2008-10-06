 set(GUI_SOURCES
	gui/AxesDialog.cc
	gui/AxesWidget.cc
#  	gui/ColorMapPreview.cc
  	gui/ColumnDialog.cc
	gui/ExportDialog.cc
	gui/ExportWidget.cc
	gui/FunctionPlotDialog.cc
  	gui/FunctionPlotWidget.cc
	gui/ImportDialog.cc
	gui/ImportWidget.cc
	gui/LabelWidget.cc
	gui/LegendDialog.cc
	gui/LegendWidget.cc
  	gui/PlotStyleWidget.cc
 	gui/PlotSurfaceStyleWidget.cc
	gui/ProjectDialog.cc
	gui/SettingsDialog.cc
	gui/SettingsGeneralPage.cc
	gui/SettingsPage.cc
	gui/SettingsPrintingPage.cc
	gui/TitleDialog.cc
)

set(ELEMENTS_SOURCES
  elements/Axis.cc
  elements/Label.cc
  elements/Legend.cc
  elements/Point.cc
  elements/Point3D.cc
  elements/Range.cc
  elements/Set.cc
  elements/Style.cc
  elements/Symbol.cc
)

set(FILTER_SOURCES
  filter/FilterCDF.cc
  filter/FilterHDF5.cc
  filter/FilterNETCDF.cc
  filter/FilterOPJ.cc
)

set(PLOTS_SOURCES
  plots/Plot.cc
  plots/Plot2D.cc
  plots/Plot2DSimple.cc
)

set(LabPlot_SRCS ${GUI_SOURCES} ${PLOTS_SOURCES} ${ELEMENTS_SOURCES} ${FILTER_SOURCES}
  parser/parser.tab.c
  tools/TexRenderer.cc
  LabPlot.cc
  MainWin.cc
#  Project.cc
  Spreadsheet.cc
#  TableModel.cc
  Worksheet.cc
)

set(UI_SOURCES
	gui/ui/axeswidget.ui
	gui/ui/columndialog.ui
	gui/ui/exportwidget.ui
	gui/ui/functionplotwidget.ui
	gui/ui/importwidget.ui
	gui/ui/labelwidget.ui
	gui/ui/legendwidget.ui
	gui/ui/plotstylewidget.ui
 	gui/ui/plotsurfacestylewidget.ui
 	gui/ui/projectdialog.ui
	gui/ui/settingsgeneralpage.ui
	gui/ui/settingsprintingpage.ui
)

set(BACKEND_DIR backend)
set(QTFRONTEND_DIR qtfrontend)
INCLUDE_DIRECTORIES(${BACKEND_DIR} ${QTFRONTEND_DIR} .)

set(BACKEND_SOURCES
	${BACKEND_DIR}/core/Folder.cpp
	${BACKEND_DIR}/core/AbstractAspect.cpp
	${BACKEND_DIR}/core/AspectPrivate.cpp
	${BACKEND_DIR}/core/globals.cpp
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
	${BACKEND_DIR}/core/datatypes/DateTime2StringFilter.cpp
	${BACKEND_DIR}/core/datatypes/String2DateTimeFilter.cpp
	${BACKEND_DIR}/core/datatypes/Double2StringFilter.cpp
	${BACKEND_DIR}/table/Table.cpp
	${BACKEND_DIR}/table/tablecommands.cpp
	${BACKEND_DIR}/lib/XmlStreamReader.cpp
)

set(QTFRONTEND_SOURCES
	${QTFRONTEND_DIR}/core/ProjectExplorer.cpp
	${QTFRONTEND_DIR}/core/PartMdiView.cpp
	${QTFRONTEND_DIR}/core/AspectTreeModel.cpp
	${QTFRONTEND_DIR}/table/TableModel.cpp
)

set(QTMOC_HDRS
	${BACKEND_DIR}/core/AbstractColumn.h
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

add_definitions(-DSUPPRESS_SCRIPTING_INIT)

kde4_add_ui_files(LabPlot_SRCS ${UI_SOURCES})
kde4_add_executable(LabPlot ${LabPlot_SRCS} ${BACKEND_SOURCES} ${QTFRONTEND_SOURCES} ${QTMOC_HDRS})

INCLUDE_DIRECTORIES(../liborigin)
target_link_libraries(LabPlot ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${HDF5_LIBRARY} ${NETCDF_LIBRARY} ${CDF_LIBRARY} ${OPJ_LIBRARY})

add_custom_command (OUTPUT parser/parser.tab.c
                   COMMAND ${YACC_EXECUTABLE} ARGS -b parser parser.y
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/parser)

install(TARGETS LabPlot DESTINATION ${BIN_INSTALL_DIR})
# install(FILES LabPlotui.rc DESTINATION  ${DATA_INSTALL_DIR}/LabPlot)

 set(GUI_SOURCES
	gui/AxesDialog.cc
	gui/AxesWidget.cc
#  	gui/ColorMapPreview.cc
  	gui/ColumnDialog.cc
	gui/FunctionPlotDialog.cc
  	gui/FunctionPlotWidget.cc
	gui/ImportDialog.cc
	gui/ImportWidget.cc
	gui/LabelWidget.cc
	gui/LegendDialog.cc
	gui/LegendWidget.cc
  	gui/PlotStyleWidget.cc
 	gui/PlotSurfaceStyleWidget.cc
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

set(LabPlot_SRCS ${GUI_SOURCES} ${ELEMENTS_SOURCES} ${FILTER_SOURCES}
  parser/parser.tab.c
  Dialog.cc
  LabPlot.cc
  MainWin.cc
  plots/Plot.cc
  plots/Plot2D.cc
  plots/Plot2DSimple.cc
  Project.cc
  ProjectDialog.cc
  Spreadsheet.cc
  TableModel.cc
  Worksheet.cc
)

set(UI_SOURCES
	gui/ui/axeswidget.ui
	gui/ui/columndialog.ui
	gui/ui/functionplotwidget.ui
	gui/ui/importwidget.ui
	gui/ui/labelwidget.ui
	gui/ui/legendwidget.ui
	gui/ui/plotstylewidget.ui
 	gui/ui/plotsurfacestylewidget.ui
	gui/ui/settingsgeneralpage.ui
	gui/ui/settingsprintingpage.ui
)

kde4_add_ui_files(LabPlot_SRCS ${UI_SOURCES})
kde4_add_executable(LabPlot ${LabPlot_SRCS})

INCLUDE_DIRECTORIES(../liborigin)
target_link_libraries(LabPlot ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${HDF5_LIBRARY} ${NETCDF_LIBRARY} ${CDF_LIBRARY} ${OPJ_LIBRARY})

add_custom_command (OUTPUT parser/parser.tab.c
                   COMMAND ${YACC_EXECUTABLE} ARGS -b parser parser.y
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/parser)

install(TARGETS LabPlot DESTINATION ${BIN_INSTALL_DIR})
# install(FILES LabPlotui.rc DESTINATION  ${DATA_INSTALL_DIR}/LabPlot)

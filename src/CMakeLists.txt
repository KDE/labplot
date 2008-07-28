 set(GUI_SOURCES
	gui/AxesDialog.cc
	gui/AxesWidget.cc
#  	gui/ColorMapPreview.cc
	gui/FunctionPlotDialog.cc
  	gui/FunctionPlotWidget.cc
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

set(LabPlot_SRCS ${GUI_SOURCES} ${ELEMENTS_SOURCES}
  parser.tab.c
  ColumnDialog.cc
  Dialog.cc
  FileInfoDialog.cc
  FilterCDF.cc
  FilterHDF5.cc
  FilterNETCDF.cc
  FilterOPJ.cc
  ImportDialog.cc
  LabPlot.cc
  MainWin.cc
  plots/Plot.cc
  plots/Plot2D.cc
  plots/Plot2DSimple.cc
  PlotDialog.cc
  Project.cc
  ProjectDialog.cc
  Spreadsheet.cc
  TableModel.cc
  Worksheet.cc
  WorksheetDialog.cc
)

set(UI_SOURCES
	gui/ui/axeswidget.ui
	gui/ui/functionplotwidget.ui
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

add_custom_command (OUTPUT parser.tab.c
                   COMMAND ${YACC_EXECUTABLE} ARGS -b parser parser.y
                   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS LabPlot DESTINATION ${BIN_INSTALL_DIR})

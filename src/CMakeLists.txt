 set(GUI_SOURCES
	gui/AxesDialog.cc
	gui/AxesWidget.cc
	gui/LabelWidget.cc
	gui/SettingsDialog.cc
	gui/SettingsGeneralPage.cc
	gui/SettingsPage.cc
	gui/SettingsPrintingPage.cc
)

set(LabPlot_SRCS ${GUI_SOURCES}
  parser.tab.c
  Axis.cc
  ColumnDialog.cc
  Dialog.cc
  FileInfoDialog.cc
  FilterCDF.cc
  FilterHDF5.cc
  FilterNETCDF.cc
  FilterOPJ.cc
  FunctionDialog.cc
  ImportDialog.cc
  Label.cc
  LabPlot.cc
  LegendDialog.cc
  MainWin.cc
  Plot.cc
  Plot2D.cc
  Plot2DSimple.cc
  PlotDialog.cc
  Point.cc
  Project.cc
  ProjectDialog.cc
  Range.cc
  Set.cc
  Set2D.cc
  Spreadsheet.cc
  Style.cc
  Symbol.cc
  TableModel.cc
  TitleDialog.cc
  Worksheet.cc
  WorksheetDialog.cc
)

set(UI_SOURCES
	gui/ui/axeswidget.ui
	gui/ui/labelwidget.ui
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

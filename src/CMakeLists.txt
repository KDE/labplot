set(LabPlot_SRCS 
  parser.tab.c
  AxesDialog.cc
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
  LRange.cc
  MainWin.cc
  Plot.cc
  Plot2D.cc
  Plot2DSimple.cc
  PlotDialog.cc
  Point.cc
  Project.cc
  ProjectDialog.cc
  Set.cc
  Set2D.cc
  Spreadsheet.cc
  Style.cc
  Symbol.cc
  TitleDialog.cc
  Worksheet.cc
  WorksheetDialog.cc
)

kde4_add_executable(LabPlot ${LabPlot_SRCS})
 
INCLUDE_DIRECTORIES(../liborigin)
target_link_libraries(LabPlot ${KDE4_KDEUI_LIBS} ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${HDF5_LIBRARY} ${NETCDF_LIBRARY} ${CDF_LIBRARY} ${OPJ_LIBRARY})
 
install(TARGETS LabPlot DESTINATION ${BIN_INSTALL_DIR})

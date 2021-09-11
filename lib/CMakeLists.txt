cmake_policy(SET CMP0079 NEW)

set(KDEFRONTEND_DIR ../src/kdefrontend)
set(BACKEND_DIR ../src/backend)
set(COMMONFRONTEND_DIR ../src/commonfrontend)
set(TOOLS_DIR ../src/tools)
set(CMAKE_AUTOMOC ON)
set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
add_definitions (-DSDK)

include(GenerateExportHeader)

# do not process these files
set_property(SOURCE gsl_parser.h gsl_parser.c PROPERTY SKIP_AUTOMOC ON)

set(GUI_SOURCES
	${KDEFRONTEND_DIR}/GuiTools.cpp
	${KDEFRONTEND_DIR}/ThemeHandler.cpp)

set(BACKEND_SOURCES
	${BACKEND_DIR}/core/Folder.cpp
	${BACKEND_DIR}/core/AbstractAspect.cpp
	${BACKEND_DIR}/core/AbstractColumn.cpp
	${BACKEND_DIR}/core/AbstractColumnPrivate.cpp
	${BACKEND_DIR}/core/abstractcolumncommands.cpp
	${BACKEND_DIR}/core/AbstractFilter.cpp
	${BACKEND_DIR}/core/AbstractSimpleFilter.cpp
	${BACKEND_DIR}/core/column/Column.cpp
	${BACKEND_DIR}/core/column/ColumnPrivate.cpp
	${BACKEND_DIR}/core/column/ColumnStringIO.cpp
	${BACKEND_DIR}/core/column/columncommands.cpp
	${BACKEND_DIR}/core/Project.cpp
	${BACKEND_DIR}/core/AbstractPart.cpp
	${BACKEND_DIR}/core/AspectTreeModel.cpp
	${BACKEND_DIR}/core/datatypes/SimpleCopyThroughFilter.h
	${BACKEND_DIR}/core/datatypes/Double2DateTimeFilter.h
	${BACKEND_DIR}/core/datatypes/Double2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/Double2IntegerFilter.h
	${BACKEND_DIR}/core/datatypes/Double2BigIntFilter.h
	${BACKEND_DIR}/core/datatypes/Double2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/Double2StringFilter.cpp
	${BACKEND_DIR}/core/datatypes/Integer2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/Integer2BigIntFilter.h
	${BACKEND_DIR}/core/datatypes/Integer2DateTimeFilter.h
	${BACKEND_DIR}/core/datatypes/Integer2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/Integer2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/Integer2StringFilter.h
	${BACKEND_DIR}/core/datatypes/BigInt2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/BigInt2IntegerFilter.h
	${BACKEND_DIR}/core/datatypes/BigInt2StringFilter.h
	${BACKEND_DIR}/core/datatypes/BigInt2DateTimeFilter.h
	${BACKEND_DIR}/core/datatypes/BigInt2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/BigInt2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/String2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/String2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/String2IntegerFilter.h
	${BACKEND_DIR}/core/datatypes/String2BigIntFilter.h
	${BACKEND_DIR}/core/datatypes/String2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/String2DateTimeFilter.cpp
	${BACKEND_DIR}/core/datatypes/DateTime2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/DateTime2IntegerFilter.h
	${BACKEND_DIR}/core/datatypes/DateTime2BigIntFilter.h
	${BACKEND_DIR}/core/datatypes/DateTime2StringFilter.cpp
	${BACKEND_DIR}/core/datatypes/Month2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/Month2IntegerFilter.h
	${BACKEND_DIR}/core/datatypes/Month2BigIntFilter.h
	${BACKEND_DIR}/core/datatypes/DayOfWeek2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/DayOfWeek2IntegerFilter.h
	${BACKEND_DIR}/core/datatypes/DayOfWeek2BigIntFilter.h
	${BACKEND_DIR}/datasources/AbstractDataSource.cpp
	${BACKEND_DIR}/gsl/ExpressionParser.cpp
	${BACKEND_DIR}/lib/Range.cpp
	${BACKEND_DIR}/lib/XmlStreamReader.cpp
	${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
	${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
	${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
	${BACKEND_DIR}/worksheet/Image.cpp
	${BACKEND_DIR}/worksheet/ResizeItem.cpp
	${BACKEND_DIR}/worksheet/WorksheetElement.cpp
	${BACKEND_DIR}/worksheet/TextLabel.cpp
	${BACKEND_DIR}/worksheet/InfoElement.cpp
	${BACKEND_DIR}/worksheet/Worksheet.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
	${BACKEND_DIR}/worksheet/TreeModel.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/BoxPlot.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianScale.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Curve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Histogram.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceLine.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYAnalysisCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYDataReductionCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYDifferentiationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYIntegrationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYInterpolationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYSmoothCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFitCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierFilterCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierTransformCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYHilbertTransformCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYConvolutionCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYCorrelationCurve.cpp
)

set(NSL_SOURCES
	${BACKEND_DIR}/nsl/nsl_conv.c
	${BACKEND_DIR}/nsl/nsl_corr.c
	${BACKEND_DIR}/nsl/nsl_dft.c
	${BACKEND_DIR}/nsl/nsl_diff.c
	${BACKEND_DIR}/nsl/nsl_filter.c
	${BACKEND_DIR}/nsl/nsl_fit.c
	${BACKEND_DIR}/nsl/nsl_geom.c
	${BACKEND_DIR}/nsl/nsl_geom_linesim.c
	${BACKEND_DIR}/nsl/nsl_hilbert.c
	${BACKEND_DIR}/nsl/nsl_int.c
	${BACKEND_DIR}/nsl/nsl_interp.c
	${BACKEND_DIR}/nsl/nsl_kde.c
	${BACKEND_DIR}/nsl/nsl_math.c
	${BACKEND_DIR}/nsl/nsl_sf_basic.c
	${BACKEND_DIR}/nsl/nsl_sf_kernel.c
	${BACKEND_DIR}/nsl/nsl_sf_poly.c
	${BACKEND_DIR}/nsl/nsl_sf_stats.c
	${BACKEND_DIR}/nsl/nsl_sf_window.c
	${BACKEND_DIR}/nsl/nsl_smooth.c
	${BACKEND_DIR}/nsl/nsl_sort.c
	${BACKEND_DIR}/nsl/nsl_stats.c
)

set(TOOLS_SOURCES
	${TOOLS_DIR}/ImageTools.cpp
	${TOOLS_DIR}/TeXRenderer.cpp
)


IF (NOT MSVC_FOUND)
IF (NOT LIBCERF_FOUND)
	list(APPEND NSL_SOURCES
		${BACKEND_DIR}/nsl/Faddeeva.c
	)
ENDIF ()
ENDIF ()

set(COMMONFRONTEND_SOURCES
	${COMMONFRONTEND_DIR}/worksheet/WorksheetView.cpp
)

bison_target(GslParser
	${BACKEND_DIR}/gsl/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.c
)
set(GENERATED_SOURCES
	${BISON_GslParser_OUTPUTS}
)

##############################################################################
add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
INCLUDE_DIRECTORIES( ../src/ ${BACKEND_DIR}/gsl ${GSL_INCLUDE_DIR} )

# see also QT_MINIMUM_VERSION
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50500)

# dynamic library
add_library(labplot ${GUI_SOURCES} ${BACKEND_SOURCES} ${NSL_SOURCES} ${COMMONFRONTEND_SOURCES} ${TOOLS_SOURCES} ${GENERATED_SOURCES} ${QTMOC_HDRS})
generate_export_header(labplot)

target_link_libraries( labplot
	KF5::Archive
	KF5::ConfigCore
	KF5::ConfigWidgets
	KF5::I18n
	KF5::KIOCore
	Qt5::Svg
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
	Qt5::PrintSupport
	${GSL_LIBRARIES}
	${GSL_CBLAS_LIBRARIES}
	${FFTW3_LIBRARIES}
	${LIBCERF_LIBRARIES} )

IF (Discount_FOUND)
	target_link_libraries( labplot Discount::Lib )
ENDIF ()

IF (WIN32)
	target_link_libraries( labplot ${PSAPI} )
ENDIF ()

############## installation ################################
install( TARGETS labplot DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/labplot_export.h  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot COMPONENT Devel)

install( FILES labplot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/ COMPONENT Devel )
install( FILES ../src/backend/core/AbstractAspect.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel )
install( FILES ../src/backend/core/AbstractColumn.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel )
install( FILES ../src/backend/core/AbstractPart.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel )
# install( FILES ../src/backend/core/AbstractFilter.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel )
# install( FILES ../src/backend/core/AbstractSimpleFilter.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel )
install( FILES ../src/backend/core/column/Column.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/column/ COMPONENT Devel )
install( FILES ../src/backend/core/column/ColumnPrivate.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/column/ COMPONENT Devel )
install( FILES ../src/backend/gsl/parser.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel )
# install( FILES ../src/backend/lib/Interval.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel )
# install( FILES ../src/backend/lib/IntervalAttribute.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel )
install( FILES ../src/backend/lib/macros.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel )
install( FILES ../src/backend/lib/macrosXYCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel )
# install( FILES ../src/backend/lib/commandtemplates.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel )
install( FILES ../src/backend/lib/Range.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel )
install( FILES ../src/backend/nsl/nsl_math.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/Worksheet.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/WorksheetElement.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/WorksheetElementContainer.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/AbstractPlot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/PlotArea.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/cartesian/Axis.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/cartesian/CartesianPlot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/cartesian/CartesianPlotLegend.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/cartesian/Curve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )
install( FILES ../src/backend/worksheet/plots/cartesian/XYCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )
# install( FILES ../src/backend/worksheet/plots/cartesian/XYAnalysisCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )
# install( FILES ../src/backend/worksheet/plots/cartesian/XYFitCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel )

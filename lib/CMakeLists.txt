cmake_policy(SET CMP0079 NEW)

set(FRONTEND_DIR ../src/frontend)
set(BACKEND_DIR ../src/backend)
set(TOOLS_DIR ../src/tools)
set(CMAKE_AUTOMOC ON)
set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)
add_definitions(-DSDK)

include(GenerateExportHeader)

# do not process these files
set_property(SOURCE gsl_parser.hpp gsl_parser.cpp PROPERTY SKIP_AUTOMOC ON)

set(GUI_SOURCES
    ${FRONTEND_DIR}/GuiTools.cpp
    ${FRONTEND_DIR}/ThemeHandler.cpp
    ${FRONTEND_DIR}/matrix/MatrixView.cpp
    ${FRONTEND_DIR}/note/NoteView.cpp
    ${FRONTEND_DIR}/workbook/WorkbookView.cpp
    ${FRONTEND_DIR}/worksheet/WorksheetView.cpp
)

set(BACKEND_SOURCES
    ${BACKEND_DIR}/core/Time.cpp
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
    ${BACKEND_DIR}/core/Workbook.cpp
    ${BACKEND_DIR}/core/AspectTreeModel.cpp
    ${BACKEND_DIR}/core/Settings.cpp
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
    ${BACKEND_DIR}/datasources/filters/AbstractFileFilter.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceFilter.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceReader.cpp
    ${BACKEND_DIR}/datasources/AbstractDataSource.cpp
    ${BACKEND_DIR}/datasources/projects/ProjectParser.cpp
    ${BACKEND_DIR}/gsl/ExpressionParser.cpp
    ${BACKEND_DIR}/gsl/constants.cpp
    ${BACKEND_DIR}/gsl/functions.cpp
    ${BACKEND_DIR}/gsl/Parser.cpp
    ${BACKEND_DIR}/lib/Range.h
    ${BACKEND_DIR}/lib/Debug.cpp
    ${BACKEND_DIR}/lib/XmlStreamReader.cpp
    ${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
    ${BACKEND_DIR}/lib/hostprocess.cpp
    ${BACKEND_DIR}/matrix/Matrix.cpp
    ${BACKEND_DIR}/matrix/matrixcommands.cpp
    ${BACKEND_DIR}/matrix/MatrixModel.cpp
    ${BACKEND_DIR}/note/Note.cpp
    ${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
    ${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
    ${BACKEND_DIR}/spreadsheet/StatisticsSpreadsheet.cpp
    ${BACKEND_DIR}/worksheet/Background.cpp
    ${BACKEND_DIR}/worksheet/Image.cpp
    ${BACKEND_DIR}/worksheet/Line.cpp
    ${BACKEND_DIR}/worksheet/ResizeItem.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElement.cpp
    ${BACKEND_DIR}/worksheet/TextLabel.cpp
    ${BACKEND_DIR}/worksheet/InfoElement.cpp
    ${BACKEND_DIR}/worksheet/Worksheet.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElementGroup.cpp
    ${BACKEND_DIR}/worksheet/TreeModel.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
    ${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/BarPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/BoxPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianScale.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ErrorBar.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Histogram.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/KDEPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/LollipopPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceLine.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Plot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ProcessBehaviorChart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceRange.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/RunChart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/QQPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYAnalysisCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFunctionCurve.cpp
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
    ${BACKEND_DIR}/worksheet/plots/cartesian/Value.cpp
)

if(ENABLE_LIBORIGIN)
        list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/datasources/projects/OriginProjectParser.cpp)
endif()
if(Cantor_FOUND)
	list(APPEND BACKEND_SOURCES ${BACKEND_DIR}/notebook/Notebook.cpp ${BACKEND_DIR}/notebook/VariableParser.cpp)
endif()

set(NSL_SOURCES
    ${BACKEND_DIR}/nsl/nsl_baseline.cpp
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
    ${BACKEND_DIR}/nsl/nsl_pcm.c
    ${BACKEND_DIR}/nsl/nsl_peak.cpp
    ${BACKEND_DIR}/nsl/nsl_randist.c
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

if(NOT MSVC_FOUND)
if(NOT LIBCERF_FOUND)
    list(APPEND NSL_SOURCES
        ${BACKEND_DIR}/nsl/Faddeeva.c
   )
endif()
endif()

bison_target(GslParser
    ${BACKEND_DIR}/gsl/parser.ypp
    ${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.cpp
)
set(GENERATED_SOURCES
    ${BISON_GslParser_OUTPUTS}
)

##############################################################################
add_definitions (${QT_DEFINITIONS} ${QT_QTDBUS_DEFINITIONS})
include_directories(../src/ ${BACKEND_DIR}/gsl ${GSL_INCLUDE_DIR} ../src/3rdparty/liborigin ../src/3rdparty/Qt-Advanced-Docking-System/src/ ../src/3rdparty/QXlsx/header/)

# see also QT_MIN_VERSION
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50500)

# add system QADS include path
#if (qt${QT_VERSION_MAJOR}advanceddocking_FOUND)
#    get_target_property(QADS_INCLUDES ads::qt${QT_VERSION_MAJOR}advanceddocking INTERFACE_INCLUDE_DIRECTORIES)
#    get_target_property(QADS_LIBRARIES ads::qt${QT_VERSION_MAJOR}advanceddocking INTERFACE_LINK_LIBRARIES)
#    MESSAGE (STATUS "QADS Found LINK_LIBRARIES: ${QADS_LIBRARIES} INCLUDES: ${QADS_INCLUDES}")
#    target_include_directories(labplot PRIVATE ${QADS_INCLUDES})
#endif ()

# dynamic library
add_library(liblabplot ${GUI_SOURCES} ${BACKEND_SOURCES} ${NSL_SOURCES} ${TOOLS_SOURCES} ${GENERATED_SOURCES} ${QTMOC_HDRS})
# name it liblabplot, not libliblabplot
set_property(TARGET liblabplot PROPERTY OUTPUT_NAME labplot)
# generate labplot_export.h + LABPLOT_EXPORT
generate_export_header(liblabplot BASE_NAME labplot)

target_link_libraries(liblabplot
    KF${KF_MAJOR_VERSION}::Archive
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::XmlGui
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::PrintSupport
    Qt${QT_MAJOR_VERSION}::Sql
    ${GSL_LIBRARIES}
    ${GSL_CBLAS_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${Poppler_LIBRARIES}
    #    ads::qt${QT_VERSION_MAJOR}advanceddocking
    ${LIBCERF_LIBRARIES})

if(Discount_FOUND)
    target_link_libraries(liblabplot Discount::Lib)
endif()

if(ENABLE_LIBORIGIN)
    if(LIBORIGIN_FOUND)
        target_link_libraries(liblabplot ${LIBORIGIN_LIBRARIES})
    else()
        target_link_libraries(liblabplot liborigin-static)
    endif()
endif()

if(Cantor_FOUND)
    # KFX::Service, KFX::Parts is optional but probably required by Cantor
    target_link_libraries(liblabplot Cantor::cantorlibs KF${KF_MAJOR_VERSION}::Service KF${KF_MAJOR_VERSION}::Parts)
endif()

if(WIN32)
    target_link_libraries(liblabplot ${PSAPI})
endif()

############## installation ################################
install(TARGETS liblabplot DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


install(FILES ${CMAKE_CURRENT_BINARY_DIR}/labplot_export.h  DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot COMPONENT Devel)

install(FILES labplot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/ COMPONENT Devel)
install(FILES ../src/backend/core/AbstractAspect.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel)
install(FILES ../src/backend/core/AbstractColumn.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel)
install(FILES ../src/backend/core/AbstractPart.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel)
install(FILES ../src/backend/core/column/Column.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/column/ COMPONENT Devel)
install(FILES ../src/backend/core/column/ColumnPrivate.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/column/ COMPONENT Devel)
install(FILES ../src/backend/gsl/Parser.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/lib/Interval.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/IntervalAttribute.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/macros.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/macrosCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/commandtemplates.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/Range.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_fit.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_math.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/Worksheet.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/WorksheetElement.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/WorksheetElementContainer.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/AbstractPlot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/PlotArea.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/Axis.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/BoxPlot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/CartesianPlot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/CartesianPlotLegend.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/XYCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/Histogram.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/XYCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/XYAnalysisCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/XYEquationCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/XYFitCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)

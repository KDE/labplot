cmake_policy(SET CMP0079 NEW)

set(FRONTEND_DIR ../src/frontend)
set(BACKEND_DIR ../src/backend)
set(TOOLS_DIR ../src/tools)

set(CMAKE_AUTOMOC ON)
set_property(SOURCE gsl_parser.hpp gsl_parser.cpp PROPERTY SKIP_AUTOMOC ON)

set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(GenerateExportHeader)

set(FILTER_SOURCES
    ${BACKEND_DIR}/datasources/filters/FilterStatus.cpp
    ${BACKEND_DIR}/datasources/filters/AsciiFilter.cpp
    ${BACKEND_DIR}/datasources/filters/BinaryFilter.cpp
    ${BACKEND_DIR}/datasources/filters/CANFilter.cpp
    ${BACKEND_DIR}/datasources/filters/FITSFilter.cpp
    ${BACKEND_DIR}/datasources/filters/ImageFilter.cpp
    ${BACKEND_DIR}/datasources/filters/JsonFilter.cpp
    ${BACKEND_DIR}/datasources/filters/MatioFilter.cpp
    ${BACKEND_DIR}/datasources/filters/NetCDFFilter.cpp
    ${BACKEND_DIR}/datasources/filters/OdsFilter.cpp
    ${BACKEND_DIR}/datasources/filters/ROOTFilter.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceFilter.cpp
    ${BACKEND_DIR}/datasources/filters/VectorBLFFilter.cpp
    ${BACKEND_DIR}/datasources/filters/XLSXFilter.cpp
    ${BACKEND_DIR}/datasources/filters/McapFilter.cpp
    ${BACKEND_DIR}/datasources/filters/HDF5Filter.cpp
    ${BACKEND_DIR}/datasources/filters/ReadStatFilter.cpp
)

set(DATA_CONTAINER_SOURCES
    ${BACKEND_DIR}/matrix/Matrix.cpp
    ${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
)

set(ASPECT_CONTAINER_SOURCES
    ${BACKEND_DIR}/core/Folder.cpp
    ${BACKEND_DIR}/core/Project.cpp
)

set(COLUMN_SOURCES
    ${BACKEND_DIR}/core/column/Column.cpp
)

set(WORKSHEET_SOURCES
    ${BACKEND_DIR}/worksheet/Worksheet.cpp
)

set(WORKSHEET_ELEMENT_SOURCES
    ${BACKEND_DIR}/worksheet/TextLabel.cpp
    ${BACKEND_DIR}/worksheet/InfoElement.cpp
    ${BACKEND_DIR}/worksheet/Image.cpp
    ${BACKEND_DIR}/worksheet/ScriptWorksheetElement.cpp
)

set(WORKSHEET_ELEMENT_CONTAINER_SOURCES
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
)

set(PLOT_AREA_ELEMENT_SOURCES
    ${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/PlotAreaBackground.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceLine.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ReferenceRange.cpp
)

set(PLOT_SOURCES
    ${BACKEND_DIR}/worksheet/plots/cartesian/BarPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/BoxPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Histogram.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/KDEPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/LollipopPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ProcessBehaviorChart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/QQPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/RunChart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYBaselineCorrectionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYConvolutionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCorrelationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYLineSimplificationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYDifferentiationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFitCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierFilterCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierTransformCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYFunctionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYHilbertTransformCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYIntegrationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYInterpolationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYSmoothCurve.cpp
)

set(TIME_SERIES_ANALYSIS_SOURCES
    ${BACKEND_DIR}/timeseriesanalysis/SeasonalDecomposition.cpp
)

set(SDK_SOURCES
    ${FILTER_SOURCES}
    ${DATA_CONTAINER_SOURCES}
    ${ASPECT_CONTAINER_SOURCES}
    ${COLUMN_SOURCES}
    ${WORKSHEET_SOURCES}
    ${WORKSHEET_ELEMENT_SOURCES}
    ${WORKSHEET_ELEMENT_CONTAINER_SOURCES}
    ${PLOT_AREA_ELEMENT_SOURCES}
    ${PLOT_SOURCES}
    ${TIME_SERIES_ANALYSIS_SOURCES}
)

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

bison_target(GslParser
    ${BACKEND_DIR}/gsl/parser.ypp
    ${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.cpp
)

set(GSL_SOURCES
    ${BISON_GslParser_OUTPUTS}
    ${BACKEND_DIR}/gsl/ExpressionParser.cpp
)

set(SDK_DEPENDENCY_SOURCES
    ${BACKEND_DIR}/core/AbstractAspect.cpp
    ${BACKEND_DIR}/core/AbstractColumn.cpp
    ${BACKEND_DIR}/datasources/AbstractDataSource.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElement.cpp
    ${BACKEND_DIR}/core/AbstractPart.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Plot.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYAnalysisCurve.cpp
    ${BACKEND_DIR}/datasources/filters/AbstractFileFilter.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
    ${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
    ${BACKEND_DIR}/worksheet/Background.cpp
    ${BACKEND_DIR}/worksheet/Line.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ErrorBar.cpp
    ${BACKEND_DIR}/spreadsheet/StatisticsSpreadsheet.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Value.cpp
    ${BACKEND_DIR}/core/column/ColumnStringIO.cpp
    ${BACKEND_DIR}/lib/Range.h
)

set(SDK_INTERNAL_SOURCES
    ${FRONTEND_DIR}/worksheet/WorksheetView.cpp
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
    ${BACKEND_DIR}/core/datatypes/SimpleCopyThroughFilter.h
    ${BACKEND_DIR}/core/AbstractFilter.cpp
    ${BACKEND_DIR}/core/AbstractSimpleFilter.cpp
    ${BACKEND_DIR}/core/column/ColumnPrivate.cpp
    ${BACKEND_DIR}/core/AbstractColumnPrivate.cpp
    ${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
    ${BACKEND_DIR}/lib/Debug.cpp
    ${BACKEND_DIR}/datasources/filters/DBCParser.cpp
    ${BACKEND_DIR}/matrix/MatrixModel.cpp
    ${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
    ${BACKEND_DIR}/lib/hostprocess.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceReader.cpp
    ${BACKEND_DIR}/core/AspectTreeModel.cpp
    ${BACKEND_DIR}/matrix/matrixcommands.cpp
    ${BACKEND_DIR}/datasources/filters/QJsonModel.cpp
    ${BACKEND_DIR}/gsl/Parser.cpp
    ${BACKEND_DIR}/gsl/functions.cpp
    ${BACKEND_DIR}/gsl/constants.cpp
    ${BACKEND_DIR}/core/Settings.cpp
    ${FRONTEND_DIR}/widgets/TextPreview.cpp
    ${FRONTEND_DIR}/GuiTools.cpp
    ${FRONTEND_DIR}/ThemeHandler.cpp
    ${TOOLS_DIR}/ColorMapsManager.cpp
    ${TOOLS_DIR}/ImageTools.cpp
    ${TOOLS_DIR}/TeXRenderer.cpp
    ${BACKEND_DIR}/core/abstractcolumncommands.cpp
    ${BACKEND_DIR}/core/column/columncommands.cpp
    ${BACKEND_DIR}/worksheet/TreeModel.cpp
    ${BACKEND_DIR}/core/Time.cpp
    ${BACKEND_DIR}/note/Note.cpp
    ${BACKEND_DIR}/datasources/projects/ProjectParser.cpp
    ${BACKEND_DIR}/worksheet/ResizeItem.cpp
    ${BACKEND_DIR}/lib/XmlStreamReader.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianScale.cpp
)

if(ENABLE_LIBORIGIN)
    list(APPEND SDK_INTERNAL_SOURCES ${BACKEND_DIR}/datasources/projects/OriginProjectParser.cpp)
    list(APPEND SDK_INTERNAL_SOURCES ${BACKEND_DIR}/core/Workbook.cpp)
endif()

if(NOT MSVC_FOUND)
if(NOT LIBCERF_FOUND)
    list(APPEND NSL_SOURCES ${BACKEND_DIR}/nsl/Faddeeva.c)
endif()
endif()

add_library(liblabplot SHARED ${SDK_SOURCES} ${SDK_DEPENDENCY_SOURCES} ${SDK_INTERNAL_SOURCES} ${NSL_SOURCES} ${GSL_SOURCES} ${QTMOC_HDRS})
add_library(LabPlot::SDK ALIAS liblabplot)
add_dependencies(liblabplot labplotlib labplotbackendlib labplotnsllib)

set_target_properties(liblabplot PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
    EXPORT_NAME SDK
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN 1
    OUTPUT_NAME labplot
)

target_compile_definitions(liblabplot PRIVATE -DSDK ${LABPLOT_COMPILE_DEFINITIONS})

get_target_property(labplotlib_INCLUDE_DIRECTORIES labplotlib INCLUDE_DIRECTORIES)
get_target_property(labplotbackendlib_INCLUDE_DIRECTORIES labplotbackendlib INCLUDE_DIRECTORIES)
get_target_property(labplotnsllib_INCLUDE_DIRECTORIES labplotnsllib INCLUDE_DIRECTORIES)

target_include_directories(liblabplot
    PRIVATE
    ${labplotlib_INCLUDE_DIRECTORIES}
    ${labplotbackendlib_INCLUDE_DIRECTORIES}
    ${labplotnsllib_INCLUDE_DIRECTORIES}
    # any dependent building me directly
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../src>
    # any dependent not building me directly
    INTERFACE $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/labplot>
)

get_target_property(labplotlib_LINK_LIBRARIES labplotlib LINK_LIBRARIES)
get_target_property(labplotbackendlib_LINK_LIBRARIES labplotbackendlib LINK_LIBRARIES)
get_target_property(labplotnsllib_LINK_LIBRARIES labplotnsllib LINK_LIBRARIES)

target_link_libraries(liblabplot
    PRIVATE
    ${labplotlib_LINK_LIBRARIES}
    ${labplotbackendlib_LINK_LIBRARIES}
    ${labplotnsllib_LINK_LIBRARIES}
    PUBLIC
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(WIN32)
if(PSAPI)
    target_link_libraries(liblabplot PRIVATE ${PSAPI})
endif()
endif()

############## installation ################################
install(TARGETS liblabplot EXPORT LabPlotTargets DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} COMPONENT Devel)

# generate labplot_export.h + LABPLOT_EXPORT macro
generate_export_header(liblabplot BASE_NAME labplot)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/labplot_export.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot COMPONENT Devel)

install(FILES labplot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/ COMPONENT Devel)

# top-level and dependency includes
set(SDK_AND_DEPENDENCY_SOURCES ${SDK_SOURCES} ${SDK_DEPENDENCY_SOURCES})
foreach(SDK_SOURCE IN LISTS SDK_AND_DEPENDENCY_SOURCES)
    string(REPLACE .cpp .h SDK_SOURCE_HEADER ${SDK_SOURCE})
    if(EXISTS ${SDK_SOURCE_HEADER})
        string(REGEX REPLACE "^\.\./src/|/[^/]+$" "" SDK_SOURCE_HEADER_DIR ${SDK_SOURCE_HEADER})
        install(FILES ${SDK_SOURCE_HEADER} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/${SDK_SOURCE_HEADER_DIR}/ COMPONENT Devel)
    endif()
endforeach(SDK_SOURCE)

# included from the top-level includes
install(FILES ../src/backend/lib/macros.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/macrosWarningStyle.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/Debug.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/gsl/Parser.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/ParserDeclarations.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/constants.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/functions.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/parserFunctionTypes.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/datasources/filters/filters.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/plots.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/tools/TeXRenderer.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/tools/ COMPONENT Devel)

install(FILES ../src/backend/nsl/nsl_fit.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_math.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_sf_stats.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_conv.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_corr.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_geom_linesim.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_diff.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_filter.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_hilbert.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_int.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_interp.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_smooth.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_kde.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_sf_kernel.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_dft.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)
install(FILES ../src/backend/nsl/nsl_sf_window.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/nsl/ COMPONENT Devel)

install(EXPORT LabPlotTargets
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/labplot
    FILE LabPlotTargets.cmake
    NAMESPACE LabPlot::
    COMPONENT Devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/labplot-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfig.cmake
    @ONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/labplot-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfigVersion.cmake
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfigVersion.cmake
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/labplot
    COMPONENT Devel
)

if(ENABLE_PYTHON_SDK)
    message(STATUS "Python SDK build ENABLED")
    add_subdirectory(python)
endif()

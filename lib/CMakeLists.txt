cmake_policy(SET CMP0079 NEW)

set(FRONTEND_DIR ../src/frontend)
set(BACKEND_DIR ../src/backend)
set(TOOLS_DIR ../src/tools)

set(CMAKE_AUTOMOC ON)
set_property(SOURCE gsl_parser.hpp gsl_parser.cpp PROPERTY SKIP_AUTOMOC ON)

set(SRC_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR} PARENT_SCOPE)

include(GenerateExportHeader)

set(FILTER_SOURCES
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
)

if(ENABLE_XLSX)
if(QXLSX_FOUND OR BUILD_QXLSX)
    list(APPEND FILTER_SOURCES ${BACKEND_DIR}/datasources/filters/XLSXFilter.cpp)
endif()
endif()

if(ENABLE_MCAP)
    list(APPEND FILTER_SOURCES ${BACKEND_DIR}/datasources/filters/McapFilter.cpp)
endif()

if(ENABLE_HDF5)
if(HDF5_FOUND)
    list(APPEND FILTER_SOURCES ${BACKEND_DIR}/datasources/filters/HDF5Filter.cpp)
endif()
endif()

if(ENABLE_READSTAT)
if(READSTAT_FOUND OR BUILD_READSTAT)
    list(APPEND FILTER_SOURCES ${BACKEND_DIR}/datasources/filters/ReadStatFilter.cpp)
endif()
endif()

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
    ${FRONTEND_DIR}/worksheet/WorksheetView.cpp
)

set(WORKSHEET_ELEMENT_SOURCES
    ${BACKEND_DIR}/worksheet/TextLabel.cpp
    ${BACKEND_DIR}/worksheet/InfoElement.cpp
    ${BACKEND_DIR}/worksheet/Image.cpp
)

set(WORKSHEET_ELEMENT_CONTAINER_SOURCES
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
)

set(PLOT_AREA_ELEMENT_SOURCES
    ${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
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
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYConvolutionCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCorrelationCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/XYDataReductionCurve.cpp
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

set(GSL_SOURCES
    ${BACKEND_DIR}/gsl/ExpressionParser.cpp
)

# need to separate into internal classes and public classes
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
    ${BACKEND_DIR}/datasources/projects/ProjectParser.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
    ${BACKEND_DIR}/lib/Range.h
    ${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
    ${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
    ${BACKEND_DIR}/lib/XmlStreamReader.cpp
    ${BACKEND_DIR}/lib/Debug.cpp
    ${BACKEND_DIR}/datasources/filters/DBCParser.cpp
    ${BACKEND_DIR}/datasources/filters/QJsonModel.cpp
    ${BACKEND_DIR}/lib/hostprocess.cpp
    ${BACKEND_DIR}/datasources/filters/SpiceReader.cpp
    ${BACKEND_DIR}/matrix/matrixcommands.cpp
    ${BACKEND_DIR}/matrix/MatrixModel.cpp
    ${BACKEND_DIR}/spreadsheet/StatisticsSpreadsheet.cpp
    ${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
    ${BACKEND_DIR}/worksheet/ResizeItem.cpp
    ${BACKEND_DIR}/core/AspectTreeModel.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
    ${BACKEND_DIR}/core/abstractcolumncommands.cpp
    ${BACKEND_DIR}/core/AbstractColumnPrivate.cpp
    ${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
    ${BACKEND_DIR}/core/AbstractSimpleFilter.cpp
    ${BACKEND_DIR}/worksheet/Background.cpp
    ${BACKEND_DIR}/worksheet/Line.cpp
    ${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/ErrorBar.cpp
    ${BACKEND_DIR}/core/column/ColumnPrivate.cpp
    ${BACKEND_DIR}/core/AbstractFilter.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/Value.cpp
    ${BACKEND_DIR}/core/column/ColumnStringIO.cpp
    ${BACKEND_DIR}/core/column/columncommands.cpp
    ${BACKEND_DIR}/worksheet/TreeModel.cpp
    ${BACKEND_DIR}/worksheet/plots/cartesian/CartesianScale.cpp
    ${BACKEND_DIR}/core/Time.cpp
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
    ${BACKEND_DIR}/gsl/Parser.cpp
    ${BACKEND_DIR}/gsl/functions.cpp
    ${BACKEND_DIR}/gsl/constants.cpp
    ${BACKEND_DIR}/core/Settings.cpp
    ${FRONTEND_DIR}/GuiTools.cpp
    ${FRONTEND_DIR}/ThemeHandler.cpp
    ${TOOLS_DIR}/ImageTools.cpp
    ${TOOLS_DIR}/TeXRenderer.cpp
    ${BACKEND_DIR}/note/Note.cpp
)

if(ENABLE_LIBORIGIN)
    list(APPEND SDK_DEPENDENCY_SOURCES ${BACKEND_DIR}/datasources/projects/OriginProjectParser.cpp)
    list(APPEND SDK_DEPENDENCY_SOURCES ${BACKEND_DIR}/core/Workbook.cpp)
endif()

if(NOT MSVC_FOUND)
if(NOT LIBCERF_FOUND)
    list(APPEND NSL_SOURCES ${BACKEND_DIR}/nsl/Faddeeva.c)
endif()
endif()

bison_target(GslParser
    ${BACKEND_DIR}/gsl/parser.ypp
    ${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.cpp
)

list(APPEND GSL_SOURCES ${BISON_GslParser_OUTPUTS})

# dynamic library
add_library(liblabplot SHARED ${SDK_SOURCES} ${NSL_SOURCES} ${GSL_SOURCES} ${QTMOC_HDRS} ${SDK_DEPENDENCY_SOURCES})
add_library(LabPlot::SDK ALIAS liblabplot)

set_target_properties(liblabplot PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
    EXPORT_NAME SDK # the name that consumers will refer to this target (on namespace LabPlot)
)

target_include_directories(liblabplot
    PRIVATE ${BACKEND_DIR}/gsl/ ${GSL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/../src/3rdparty/liborigin
    # any dependent building me directly
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/>
    # any dependent not building me directly
    INTERFACE $<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR}/labplot>
)

if(ENABLE_XLSX)
if(BUILD_QXLSX)
    target_include_directories(liblabplot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/3rdparty/QXlsx/header")
endif()

if(QXLSX_FOUND)
    target_include_directories(liblabplot PRIVATE ${QXLSX_INCLUDE_DIR})
endif()
endif()

if(ENABLE_MCAP)
target_include_directories(liblabplot PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../src/3rdparty/mcap/include")
IF(LZ4_FOUND)
    target_include_directories(liblabplot PRIVATE ${LZ4_INCLUDE_DIR})
endif()

IF(ZSTD_FOUND)
    target_include_directories(liblabplot PRIVATE ${ZSTD_INCLUDE_DIR})
endif()
endif()

if(ENABLE_HDF5)
if(HDF5_FOUND)
    target_include_directories(liblabplot PRIVATE ${HDF5_INCLUDE_DIRS})
endif()
endif()

if(ENABLE_READSTAT)
if(READSTAT_FOUND OR BUILD_READSTAT)
    target_include_directories(liblabplot PRIVATE ${CMAKE_BINARY_DIR}/src/3rdparty/install/include)
    if(EXISTS ${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src)
    target_include_directories(liblabplot PRIVATE ${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src)
    endif()
endif()
endif()

target_compile_definitions(liblabplot PRIVATE -DSDK ${LABPLOT_COMPILE_DEFINITIONS})
# name it liblabplot, not libliblabplot
set_property(TARGET liblabplot PROPERTY OUTPUT_NAME labplot)
# generate labplot_export.h + LABPLOT_EXPORT
generate_export_header(liblabplot BASE_NAME labplot)

# trim linked libraries later
# how can we declare the shared libs needed by dependents with cmake?
target_link_libraries(liblabplot
    PRIVATE
    KF${KF_MAJOR_VERSION}::Archive
    KF${KF_MAJOR_VERSION}::ConfigCore
    KF${KF_MAJOR_VERSION}::ConfigWidgets
    KF${KF_MAJOR_VERSION}::I18n
    KF${KF_MAJOR_VERSION}::KIOCore
    KF${KF_MAJOR_VERSION}::XmlGui
    Qt${QT_MAJOR_VERSION}::PrintSupport
    Qt${QT_MAJOR_VERSION}::Sql
    ${GSL_LIBRARIES}
    ${GSL_CBLAS_LIBRARIES}
    ${FFTW3_LIBRARIES}
    ${Poppler_LIBRARIES}
    ${LIBCERF_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${LZ4_LIBRARIES}
    ${HDF5_LIBRARIES}
    ${netCDF_LIBRARIES}
    ${CFITSIO_LIBRARIES}
    ${MATIO_LIBRARIES}
    PUBLIC
    # ideally any project depending on us should link these qt libs, not because we rely on qt in our public api
    # but for cohesion and proper functioning of the library 
    Qt${QT_MAJOR_VERSION}::Core
    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Widgets
)

if(ENABLE_XLSX)
if(QXLSX_FOUND OR BUILD_QXLSX)
    target_link_libraries(liblabplot PRIVATE QXlsx)
endif()
endif()

if(ENABLE_MCAP)
IF(LZ4_FOUND)
    target_link_libraries(liblabplot PRIVATE ${LZ4_LIBRARIES})
endif()

if(ZSTD_FOUND)
    target_link_libraries(liblabplot PRIVATE ${ZSTD_LIBRARIES})
endif()
endif()

if(ENABLE_HDF5)
if(HDF5_FOUND)
    target_link_libraries(liblabplot PRIVATE ${HDF5_LIBRARIES})
endif()
endif()

if(ENABLE_READSTAT)
if(READSTAT_FOUND OR BUILD_READSTAT)
    if(READSTAT_FOUND AND ICONV_LIBRARIES)
        target_link_libraries (liblabplot PRIVATE ${ICONV_LIBRARIES})
    endif()
    target_link_libraries(liblabplot PRIVATE ${READSTAT_LIBRARIES} ${ZLIB_LIBRARIES})
endif()
endif()

if(Discount_FOUND)
    target_link_libraries(liblabplot PRIVATE Discount::Lib)
endif()

if(ENABLE_LIBORIGIN)
    target_include_directories(liblabplot PRIVATE ../src/3rdparty/liborigin)
    if(LIBORIGIN_FOUND)
        target_link_libraries(liblabplot PRIVATE ${LIBORIGIN_LIBRARIES})
    else()
        target_link_libraries(liblabplot PRIVATE liborigin-static)
    endif()
endif()

if(Cantor_FOUND)
    # KFX::Service, KFX::Parts is optional but probably required by Cantor
    target_link_libraries(liblabplot PRIVATE Cantor::cantorlibs KF${KF_MAJOR_VERSION}::Service KF${KF_MAJOR_VERSION}::Parts)
endif()

# required by worksheetview
if(ENABLE_QTSVG)
if(Qt${QT_MAJOR_VERSION}Svg_FOUND)
    target_link_libraries(liblabplot PRIVATE Qt${QT_MAJOR_VERSION}::Svg)
endif()
endif()

if(WIN32)
if(PSAPI)
    target_link_libraries(liblabplot PRIVATE ${PSAPI})
endif()
endif()

# commented for now, will be moved to sdk tests
# if(ENABLE_SDK_EXAMPLES)
#     add_subdirectory(example)
# endif()

############## installation ################################
install(TARGETS liblabplot EXPORT LabPlotTargets DESTINATION ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} COMPONENT Devel)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/labplot_export.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot COMPONENT Devel)

configure_file(labplot.h.in ${CMAKE_CURRENT_BINARY_DIR}/labplot.h @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/labplot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/ COMPONENT Devel)

# top-level includes
foreach(SDK_SOURCE IN LISTS SDK_SOURCES)
    string(REPLACE .cpp .h SDK_SOURCE_HEADER ${SDK_SOURCE})
    if(EXISTS ${SDK_SOURCE_HEADER})
        string(REGEX REPLACE "^\.\./src/|/[^/]+$" "" SDK_SOURCE_HEADER_DIR ${SDK_SOURCE_HEADER})
        install(FILES ${SDK_SOURCE_HEADER} DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/${SDK_SOURCE_HEADER_DIR}/ COMPONENT Devel)
    endif()
endforeach(SDK_SOURCE)

# included from the top-level includes
install(FILES ../src/backend/datasources/filters/AbstractFileFilter.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
install(FILES ../src/backend/core/AbstractColumn.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel)
install(FILES ../src/backend/core/AbstractAspect.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel)
install(FILES ../src/backend/datasources/AbstractDataSource.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/ COMPONENT Devel)
install(FILES ../src/backend/core/AbstractPart.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/ COMPONENT Devel)
install(FILES ../src/backend/lib/macros.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/macrosWarningStyle.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/lib/Debug.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/core/column/ColumnStringIO.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/core/column/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/WorksheetElement.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel)
install(FILES ../src/tools/TeXRenderer.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/tools/ COMPONENT Devel)
install(FILES ../src/backend/lib/Range.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/lib/ COMPONENT Devel)
install(FILES ../src/backend/gsl/Parser.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/ParserDeclarations.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/constants.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/functions.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/gsl/parserFunctionTypes.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/gsl/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/CartesianScale.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/AbstractCoordinateSystem.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/AbstractPlot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/WorksheetElementContainer.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/Plot.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/XYAnalysisCurve.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
install(FILES ../src/backend/worksheet/plots/cartesian/ErrorBar.h DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/worksheet/plots/cartesian/ COMPONENT Devel)
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

# will be removed as soon as xlsxfilter.h and readstatfilter.h are reworked
if(ENABLE_XLSX)
if(BUILD_QXLSX)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../src/3rdparty/QXlsx/header/xlsxcellrange.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../src/3rdparty/QXlsx/header/xlsxglobal.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../src/3rdparty/QXlsx/header/xlsxcellreference.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
endif()

if(QXLSX_FOUND)
    install(FILES "${QXLSX_INCLUDE_DIR}/xlsxcellrange.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
    install(FILES "${QXLSX_INCLUDE_DIR}/xlsxglobal.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
    install(FILES "${QXLSX_INCLUDE_DIR}/xlsxcellreference.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
endif()
endif()

if(ENABLE_READSTAT)
if(READSTAT_FOUND OR BUILD_READSTAT)
    install(FILES "${CMAKE_BINARY_DIR}/src/3rdparty/install/include/readstat.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
    if(EXISTS ${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src)
    install(FILES "${CMAKE_BINARY_DIR}/src/3rdparty/readstat/src/readstat/src/readstat.h" DESTINATION ${KDE_INSTALL_INCLUDEDIR}/labplot/backend/datasources/filters/ COMPONENT Devel)
    endif()
endif()
endif()

install(EXPORT LabPlotTargets
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/labplot
    FILE LabPlotTargets.cmake
    NAMESPACE LabPlot:: # namespace all targets in this export
    COMPONENT Devel
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/labplot-config.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfig.cmake
    @ONLY
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/labplot-config-version.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfigVersion.cmake
    @ONLY
)

install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfig.cmake ${CMAKE_CURRENT_BINARY_DIR}/LabPlotConfigVersion.cmake
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/labplot
    COMPONENT Devel
)
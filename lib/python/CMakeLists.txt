find_package(Python3 COMPONENTS Development Development.Embed Interpreter REQUIRED)
find_package(PySide6 REQUIRED)
find_package(Shiboken6 REQUIRED)

include(KDPySide6ModuleBuild REQUIRED)

set(PyLabPlot_sources
    # abstract classes
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractaspect_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractaspect_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractcolumn_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractcolumn_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractcoordinatesystem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractcoordinatesystem_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractdatasource_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractdatasource_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractfilefilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractfilefilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractpart_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractpart_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/abstractplot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/plot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/plot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelement_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelement_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelementcontainer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelementcontainer_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyanalysiscurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyanalysiscurve_wrapper.h
    
    # aspect containers
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/folder_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/folder_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/project_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/project_wrapper.h

    # column
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/column_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/column_wrapper.h

    # data containers
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/spreadsheet_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/spreadsheet_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/matrix_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/matrix_wrapper.h

    # filters
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/asciifilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/asciifilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/asciifilter_properties_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/asciifilter_properties_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/binaryfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/binaryfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/canfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/canfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/fitsfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/fitsfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/hdf5filter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/hdf5filter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/imagefilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/imagefilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/jsonfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/jsonfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/matiofilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/matiofilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/mcapfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/mcapfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/netcdffilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/netcdffilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/odsfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/odsfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/readstatfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/readstatfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/rootfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/rootfilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/spicefilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/spicefilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/vectorblffilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/vectorblffilter_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xlsxfilter_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xlsxfilter_wrapper.h

    # helper classes
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/background_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/background_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesiancoordinatesystem_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesiancoordinatesystem_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianscale_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianscale_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/columnstringio_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/columnstringio_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/errorbar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/errorbar_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/line_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/line_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/plotarea_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/plotarea_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/statisticsspreadsheet_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/statisticsspreadsheet_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/symbol_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/symbol_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/value_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/value_wrapper.h

    # plot area elements
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/axis_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/axis_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplotlegend_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplotlegend_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/custompoint_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/custompoint_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/referenceline_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/referenceline_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/referencerange_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/referencerange_wrapper.h

    # plot
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/barplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/barplot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/boxplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/boxplot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/histogram_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/histogram_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/kdeplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/kdeplot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/lollipopplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/lollipopplot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/processbehaviorchart_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/processbehaviorchart_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/qqplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/qqplot_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/runchart_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/runchart_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xycurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xycurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyequationcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyequationcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyequationcurve_equationdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyequationcurve_equationdata_wrapper.h

    # analysis plot
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyconvolutioncurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyconvolutioncurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyconvolutioncurve_convolutiondata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyconvolutioncurve_convolutiondata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xycorrelationcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xycorrelationcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xycorrelationcurve_correlationdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xycorrelationcurve_correlationdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydatareductioncurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydatareductioncurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydatareductioncurve_datareductiondata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydatareductioncurve_datareductiondata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_differentiationdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_differentiationdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_fitdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_fitdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_filterdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_filterdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfouriertransformcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfouriertransformcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfouriertransformcurve_transformdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfouriertransformcurve_transformdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfunctioncurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfunctioncurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfunctioncurve_functiondata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfunctioncurve_functiondata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyhilberttransformcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyhilberttransformcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyhilberttransformcurve_transformdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyhilberttransformcurve_transformdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyintegrationcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyintegrationcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyintegrationcurve_integrationdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyintegrationcurve_integrationdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyinterpolationcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyinterpolationcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyinterpolationcurve_interpolationdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyinterpolationcurve_interpolationdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xysmoothcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xysmoothcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xysmoothcurve_smoothdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xysmoothcurve_smoothdata_wrapper.h

    # worksheet
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheet_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheet_wrapper.h

    # worksheet element containers
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplot_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplot_wrapper.h

    # worksheet elements
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/image_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/image_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/infoelement_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/infoelement_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/textlabel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/textlabel_wrapper.h

    # global module wrapper
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/pylabplot_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/pylabplot_python.h
)

set(PyLabPlot_depends
    # abstract classes
    ${CMAKE_SOURCE_DIR}/src/backend/core/AbstractAspect.h
    ${CMAKE_SOURCE_DIR}/src/backend/core/AbstractColumn.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/AbstractCoordinateSystem.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/AbstractDataSource.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/AbstractFileFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/core/AbstractPart.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/AbstractPlot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/Plot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/WorksheetElement.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/WorksheetElementContainer.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYAnalysisCurve.h
    
    # aspect containers
    ${CMAKE_SOURCE_DIR}/src/backend/core/Folder.h
    ${CMAKE_SOURCE_DIR}/src/backend/core/Project.h
    
    # column
    ${CMAKE_SOURCE_DIR}/src/backend/core/column/Column.h
    
    # data containers
    ${CMAKE_SOURCE_DIR}/src/backend/spreadsheet/Spreadsheet.h
    ${CMAKE_SOURCE_DIR}/src/backend/matrix/Matrix.h
    
    # filters
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/AsciiFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/BinaryFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/CANFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/FITSFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/HDF5Filter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/ImageFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/JsonFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/MatioFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/McapFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/NetCDFFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/OdsFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/ReadStatFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/ROOTFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/SpiceFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/VectorBLFFilter.h
    ${CMAKE_SOURCE_DIR}/src/backend/datasources/filters/XLSXFilter.h
    
    # helper classes
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/Background.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/CartesianCoordinateSystem.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/CartesianScale.h
    ${CMAKE_SOURCE_DIR}/src/backend/core/column/ColumnStringIO.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/ErrorBar.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/Line.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/PlotArea.h
    
    ${CMAKE_SOURCE_DIR}/src/backend/spreadsheet/StatisticsSpreadsheet.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/Symbol.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/Value.h
    
    # plot area elements
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/Axis.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/CartesianPlotLegend.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/CustomPoint.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/ReferenceLine.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/ReferenceRange.h
    
    # plots
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/BarPlot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/BoxPlot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/Histogram.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/KDEPlot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/LollipopPlot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/ProcessBehaviorChart.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/QQPlot.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/RunChart.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYEquationCurve.h
    
    # analysis plots
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYConvolutionCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYCorrelationCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYDataReductionCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYDifferentiationCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYFitCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYFourierFilterCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYFourierTransformCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYFunctionCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYHilbertTransformCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYIntegrationCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYInterpolationCurve.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/XYSmoothCurve.h
    
    # worksheet
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/Worksheet.h
    
    # worksheet element containers
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/plots/cartesian/CartesianPlot.h
    
    # worksheet elements
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/Image.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/InfoElement.h
    ${CMAKE_SOURCE_DIR}/src/backend/worksheet/TextLabel.h
)

get_target_property(liblabplot_INCLUDE_DIRECTORIES liblabplot INCLUDE_DIRECTORIES)

set(PyLabPlot_include_paths
    ${PYSIDE_INCLUDE_DIR}
    ${PYSIDE_INCLUDE_DIR}/QtCore
    ${PYSIDE_INCLUDE_DIR}/QtGui
    ${PYSIDE_INCLUDE_DIR}/QtWidgets
    ${SHIBOKEN_INCLUDE_DIR}
    ${Python3_INCLUDE_DIRS}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Gui_INCLUDE_DIRS}
    ${Qt6Widgets_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}/../
    ${liblabplot_INCLUDE_DIRECTORIES}
)

get_target_property(liblabplot_LINK_LIBRARIES liblabplot LINK_LIBRARIES)

set(PyLabPlot_link_libraries
    ${SHIBOKEN_LIBRARY}
    ${PYSIDE_LIBRARY}
    ${Python3_LIBRARIES}
    ${liblabplot_LINK_LIBRARIES}
    liblabplot
)

set(PyLabPlot_typesystem_paths
    ${PYSIDE_TYPESYSTEMS}
)

set(ENV{CLANG_INSTALL_DIR} /usr/lib64/clang/19)

CREATE_PYTHON_BINDINGS(
    "pylabplot"
    "${PyLabPlot_typesystem_paths}"
    "${PyLabPlot_include_paths}"
    "${PyLabPlot_sources}"
    "${PyLabPlot_include_paths}"
    "${PyLabPlot_link_libraries}"
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bindings.xml
    "${PyLabPlot_depends}"
    ${CMAKE_CURRENT_BINARY_DIR}
)

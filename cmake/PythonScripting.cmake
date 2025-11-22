include(GenerateShibokenSources)

set(scripting_library "pylabplot")
set(scripting_wrapped_header ${CMAKE_CURRENT_SOURCE_DIR}/../lib/python/bindings.h)
set(scripting_typesystem_file ${CMAKE_CURRENT_SOURCE_DIR}/../lib/python/bindings.xml)
set(shiboken_scripting_generated_sources
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
#    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelement_positionwrapper_wrapper.cpp
#    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelement_positionwrapper_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelementcontainer_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/worksheetelementcontainer_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyanalysiscurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyanalysiscurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyanalysiscurve_result_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyanalysiscurve_result_wrapper.h
    
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
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/errorbar_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/errorbar_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/line_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/line_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/plotarea_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/plotarea_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/doublerange_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/doublerange_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/intrange_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/intrange_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/ranget_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/ranget_wrapper.h
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
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydatareductioncurve_datareductionresult_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydatareductioncurve_datareductionresult_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_differentiationdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xydifferentiationcurve_differentiationdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_fitdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_fitdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_fitresult_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfitcurve_fitresult_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_filterdata_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_filterdata_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_filterresult_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyfourierfiltercurve_filterresult_wrapper.h
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
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyintegrationcurve_integrationresult_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/xyintegrationcurve_integrationresult_wrapper.h
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
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplot_rangebreak_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplot_rangebreak_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplot_rangebreaks_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/cartesianplot_rangebreaks_wrapper.h

    # worksheet elements
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/image_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/image_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/infoelement_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/infoelement_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/infoelement_markerpoints_t_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/infoelement_markerpoints_t_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/textlabel_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/textlabel_wrapper.h
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/textlabel_textwrapper_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/textlabel_textwrapper_wrapper.h

    # scripting
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/pythonlogger_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/pythonlogger_wrapper.h

    # global module wrapper
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/pylabplot_module_wrapper.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/pylabplot/pylabplot_python.h
)
set(python_scripting_backend_sources
    ${BACKEND_DIR}/script/python/PythonScriptRuntime.cpp
    ${BACKEND_DIR}/script/python/PythonLogger.cpp
)
get_target_property(PySide6_INCLUDE_DIRECTORIES PySide6::pyside6 INTERFACE_INCLUDE_DIRECTORIES)
set(python_scripting_includes
    ${PYSIDE_PYTHONPATH}/include
    ${PySide6_INCLUDE_DIRECTORIES}
    ${SHIBOKEN_PYTHON_INCLUDE_DIRS}
    ${Shiboken6_INCLUDE_DIRECTORIES}
    ${PySide6_PYTHONPATH}/include/QtWidgets
    ${PySide6_PYTHONPATH}/include/QtGui
    ${PySide6_PYTHONPATH}/include/QtCore
)
get_target_property(Shiboken6_LIBRARIES Shiboken6::libshiboken IMPORTED_LOCATION)
set(python_scripting_link_libraries
    PySide6::pyside6
    Shiboken6::libshiboken
    ${Python3_LIBRARIES}
    ${PySide6_ABI3_LIBRARY}
    ${Shiboken6_LIBRARIES}
)

# Hide noisy warnings in shiboken scripting generated sources
# 1. Wno-missing-include-dirs : pyside6 declares an include directory in the INTERFACE_INCLUDE_DIRECTORIES which doesn't actually exist and thus the compiler complains
# 2. Wno-cast-function-type: the shiboken6 generated files contain weird but correct function casts which the compiler complains about
set_property(SOURCE ${shiboken_scripting_generated_sources} ${python_scripting_backend_sources} APPEND PROPERTY COMPILE_OPTIONS -Wno-cast-function-type -Wno-missing-include-dirs)

# Previously, we were adding the python_scripting_includes to the liblabplotbackendlib target, but we can actually restrict things further and add these includes to only the
# shiboken generated sources since they are the ones who require the includes
set_property(SOURCE ${shiboken_scripting_generated_sources} ${python_scripting_backend_sources} APPEND PROPERTY INCLUDE_DIRECTORIES ${python_scripting_includes})

# Shiboken internally defines the Py_LIMITED_API to 3.8 and we also define the Py_LIMITED_API but to 3.9 so the compiler warns about a macro redefinition. Now we apply our
# definition to only our source files
set_property(SOURCE ${python_scripting_backend_sources} APPEND PROPERTY COMPILE_DEFINITIONS -DPy_LIMITED_API=0x03090000)

# PYTHON3_EXECUTABLE is the python executable path and is needed when initializing the python scripting interpreter
set_property(SOURCE ${BACKEND_DIR}/script/python/PythonScriptRuntime.cpp APPEND PROPERTY COMPILE_DEFINITIONS -DPYTHON3_EXECUTABLE=${Python3_EXECUTABLE})

# shiboken generates sources using deprecated code so we remove these deprecation macros to enable the shiboken generated files to compile
get_property(_defs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_BEFORE=]])
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})
get_property(_defs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_BEFORE=]])
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})

get_property(_defs DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_UP_TO=]])
set_property(DIRECTORY ${CMAKE_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})
get_property(_defs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS)
list(FILTER _defs EXCLUDE REGEX [[^QT_DISABLE_DEPRECATED_UP_TO=]])
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY COMPILE_DEFINITIONS ${_defs})

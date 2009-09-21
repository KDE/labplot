
set(FILTER_SOURCES
	FilterCDF.cpp
	FilterHDF5.cpp
	FilterNETCDF.cpp
	FilterOPJ.cpp
)

INCLUDE_DIRECTORIES(../${BACKEND_DIR} ../${QTFRONTEND_DIR} ..)
# type can be STATIC, SHARED or MODULE (not linked to executable!)
add_library(labplot_filter ${FILTER_SOURCES})
set_target_properties(labplot_filter PROPERTIES VERSION 0.0.1 SOVERSION 0)
target_link_libraries(labplot_filter ${KDE4_KDEUI_LIBS} ${HDF5_LIBRARY} ${NETCDF_LIBRARY} ${CDF_LIBRARY})

install(TARGETS labplot_filter DESTINATION ${LIB_INSTALL_DIR})

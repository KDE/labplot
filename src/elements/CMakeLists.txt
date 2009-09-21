set(ELEMENTS_SOURCES
	Axis.cpp
	Label.cpp
	Legend.cpp
	Point.cpp
	Point3D.cpp
	Range.cpp
	Set.cpp
	Style.cpp
	Symbol.cpp
	../tools/TexRenderer.cpp
	../tools/ColorMapRenderer.cpp
)

# type can be STATIC, SHARED or MODULE (not linked to executable!)
add_library(labplot_elements ${ELEMENTS_SOURCES})
set_target_properties(labplot_elements PROPERTIES VERSION 0.0.1 SOVERSION 0)
target_link_libraries(labplot_elements ${KDE4_KDEUI_LIBS})

install(TARGETS labplot_elements DESTINATION ${LIB_INSTALL_DIR})

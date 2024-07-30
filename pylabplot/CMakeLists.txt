include(PyLabPlot)

set(PyLabPlot_target_link_libraries
    labplot2backendlib
    labplot2lib
    labplot2nsllib
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    ${PyLabPlot_libraries}
)

set(libraryName "pylabplot")

set(TARGET_NAME "Py${libraryName}")
set(MODULE_NAME "${libraryName}")
add_library(${TARGET_NAME} MODULE ${PyLabPlot_SRC})

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES PREFIX ""
                OUTPUT_NAME ${MODULE_NAME}
                LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
if(APPLE)
    set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "@loader_path")
elseif(NOT WIN32) #ie. linux
    set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH "$ORIGIN")
endif()

if(WIN32)
    set_target_properties(${TARGET_NAME} PROPERTIES SUFFIX ".pyd")
endif()

target_include_directories(${TARGET_NAME} PUBLIC ${PyLabPlot_include_paths})

target_link_libraries(${TARGET_NAME} ${PyLabPlot_target_link_libraries})
target_compile_definitions(${TARGET_NAME} PRIVATE Py_LIMITED_API=0x03050000)
if(APPLE)
    set_property(
        TARGET ${TARGET_NAME}
        APPEND
        PROPERTY LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()
install(TARGETS ${TARGET_NAME} LIBRARY DESTINATION ${${PROJECT_NAME}_PYTHON_BINDINGS_INSTALL_PREFIX})
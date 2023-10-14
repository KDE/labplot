# TODO: fix test for Qt6 on Windows
if (QT_MAJOR_VERSION LESS 6 OR NOT WIN32)
add_subdirectory(widgets)
endif ()

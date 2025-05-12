# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

include_directories(${PROJECT_SOURCE_DIR}/src)

if(OPTION_BUILD_EXAMPLES)
    add_definitions(
        -DCMAKE_CURRENT_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}"
        -DCMAKE_CURRENT_BINARY_DIR="${CMAKE_CURRENT_BINARY_DIR}")

    set(CMAKE_STATIC_LIBRARY_PREFIX "${CMAKE_CURRENT_BINARY_DIR}")
    set(CMAKE_SHARED_LIBRARY_PREFIX "${CMAKE_CURRENT_BINARY_DIR}")

#    if(WIN32)
#        add_custom_target(Vector_BLF-Copy ALL
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                "${PROJECT_BINARY_DIR}/src/Vector/BLF/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.dll"
#                "${CMAKE_CURRENT_BINARY_DIR}"
#            COMMENT "Copy ${PROJECT_BINARY_DIR}/src/Vector/BLF/${CMAKE_BUILD_TYPE}/${PROJECT_NAME}.dll to ${CMAKE_CURRENT_BINARY_DIR}")
#    endif()

    add_executable(vector-blf-parser "")
    target_sources(vector-blf-parser PRIVATE Parser.cpp)
    target_link_libraries(vector-blf-parser PRIVATE ${PROJECT_NAME})

    add_executable(vector-blf-write-example "")
    target_sources(vector-blf-write-example PRIVATE Write-Example.cpp)
    target_link_libraries(vector-blf-write-example PRIVATE ${PROJECT_NAME})

    install(
        TARGETS vector-blf-parser vector-blf-write-example
        DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(
    FILES Parser.cpp Write-Example.cpp
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/examples)

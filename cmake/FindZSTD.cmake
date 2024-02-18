find_path(ZSTD_INCLUDE_DIR
    NAMES zstd.h
    PATHS
        ${CONAN_INCLUDE_DIRS_RELEASE}
        ${CONAN_INCLUDE_DIRS_DEBUG}
)

set(ZSTD_NAMES zstd zstd_static)
set(ZSTD_NAMES_DEBUG zstdd zstd_staticd)

find_library(ZSTD_LIBRARY_RELEASE
    NAMES ${ZSTD_NAMES}
    PATHS ${CONAN_LIB_DIRS_RELEASE}
)
find_library(ZSTD_LIBRARY_DEBUG
    NAMES
        ${ZSTD_NAMES_DEBUG}
        ${ZSTD_NAMES}
    PATHS ${CONAN_LIB_DIRS_DEBUG}
)

include(SelectLibraryConfigurations)
select_library_configurations(ZSTD)

if(ZSTD_INCLUDE_DIR AND EXISTS "${ZSTD_INCLUDE_DIR}/zstd.h")
    file(STRINGS "${ZSTD_INCLUDE_DIR}/zstd.h" ZSTD_VERSION_MAJOR_LINE REGEX "^#define ZSTD_VERSION_MAJOR.*$")
    file(STRINGS "${ZSTD_INCLUDE_DIR}/zstd.h" ZSTD_VERSION_MINOR_LINE REGEX "^#define ZSTD_VERSION_MINOR.*$")
    file(STRINGS "${ZSTD_INCLUDE_DIR}/zstd.h" ZSTD_VERSION_RELEASE_LINE REGEX "^#define ZSTD_VERSION_RELEASE.*$")
    
    string(REGEX REPLACE "^.*ZSTD_VERSION_MAJOR *([0-9]+)$" "\\1" ZSTD_VERSION_MAJOR "${ZSTD_VERSION_MAJOR_LINE}")
    string(REGEX REPLACE "^.*ZSTD_VERSION_MINOR *([0-9]+)$" "\\1" ZSTD_VERSION_MINOR "${ZSTD_VERSION_MINOR_LINE}")
    string(REGEX REPLACE "^.*ZSTD_VERSION_RELEASE *([0-9]+)$" "\\1" ZSTD_VERSION_RELEASE "${ZSTD_VERSION_RELEASE_LINE}")
    
    set(ZSTD_VERSION_STRING "${ZSTD_VERSION_MAJOR}.${ZSTD_VERSION_MINOR}.${ZSTD_VERSION_RELEASE}")
endif()

# handle the QUIETLY and REQUIRED arguments and set ZLIB_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(zstd REQUIRED_VARS ZSTD_LIBRARY ZSTD_INCLUDE_DIR
                                       VERSION_VAR ZSTD_VERSION_STRING)


if (ZSTD_FOUND)
    set(ZSTD_INCLUDE_DIRS ${ZSTD_INCLUDE_DIR})

    if (NOT ZSTD_LIBRARIES)
        set(ZSTD_LIBRARIES ${ZSTD_LIBRARY})
    endif()

    if (NOT TARGET zstd::zstd)
        add_library(zstd::zstd UNKNOWN IMPORTED)
        set_target_properties(zstd::zstd PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_INCLUDE_DIRS}")

        if(ZSTD_LIBRARY_RELEASE)
            set_property(TARGET zstd::zstd APPEND PROPERTY
                IMPORTED_CONFIGURATIONS RELEASE)
            set_target_properties(zstd::zstd PROPERTIES
                IMPORTED_LOCATION_RELEASE "${ZSTD_LIBRARY_RELEASE}")
        endif()

        if(ZSTD_LIBRARY_DEBUG)
            set_property(TARGET zstd::zstd APPEND PROPERTY
                IMPORTED_CONFIGURATIONS DEBUG)
            set_target_properties(zstd::zstd PROPERTIES
                IMPORTED_LOCATION_DEBUG "${ZSTD_LIBRARY_DEBUG}")
        endif()

        if(NOT ZSTD_LIBRARY_RELEASE AND NOT ZSTD_LIBRARY_DEBUG)
            set_property(TARGET zstd::zstd APPEND PROPERTY
                IMPORTED_LOCATION "${ZSTD_LIBRARY}")
        endif()
    endif()
endif()
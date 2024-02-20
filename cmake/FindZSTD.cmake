find_package(PkgConfig QUIET)
pkg_check_modules(PC_ZSTD zstd QUIET)

find_library(ZSTD_LIBRARIES
    NAMES zstd
    HINTS ${PC_ZSTD_LIBRARY_DIRS}
)

find_path(ZSTD_INCLUDE_DIR
    NAMES zstd.h
    HINTS ${PC_ZSTD_INCLUDE_DIRS}
)

set(ZSTD_VERSION "${PC_ZSTD_VERSION}")


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ZSTD
    REQUIRED_VARS
    ZSTD_LIBRARIES
    ZSTD_INCLUDE_DIR
    VERSION_VAR
    ZSTD_VERSION
)

if(ZSTD_FOUND AND NOT TARGET zstd::zstd)
    add_library(zstd::zstd UNKNOWN IMPORTED)
    set_target_properties(zstd::zstd PROPERTIES
	IMPORTED_LOCATION "$ZSTD_LIBRARIES}"
        INTERFACE_COMPILE_OPTIONS "${PC_ZSTD_CFLAGS}"
        INTERFACE_INCLUDE_DIRECTORIES "${ZSTD_INCLUDE_DIR}"
    )
else()
	set(ZSTD_LIBRARIES "")
endif()

mark_as_advanced(ZSTD_LIBRARIES ZSTD_INCLUDE_DIR ZSTD_VERSION)

include(FeatureSummary)
set_package_properties(ZSTD PROPERTIES
    DESCRIPTION "Zstandard - Fast real-time compression algorithm "
    URL "https://github.com/facebook/zstd"
)
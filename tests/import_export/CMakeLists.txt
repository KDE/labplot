option(ENABLE_TEST_IMPORTEXPORT "Enable Import/Export Tests" ON)

if (ENABLE_TEST_IMPORTEXPORT)

    add_subdirectory(ASCII)
    add_subdirectory(Binary)
    add_subdirectory(JSON)
    add_subdirectory(MQTT)
    add_subdirectory(Project)
    add_subdirectory(Spice)
    add_subdirectory(CAN)
    add_subdirectory(SQL)

    option(ENABLE_TEST_IMPORTEXPORT_ROOT "Enable Root Tests" ON)
    IF (ZLIB_FOUND AND LZ4_FOUND AND ENABLE_TEST_IMPORTEXPORT_ROOT)
            add_subdirectory(ROOT)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_READSTAT "Enable Readstat Tests" ON)
    IF ((READSTAT_FOUND OR BUILD_READSTAT) AND ENABLE_TEST_IMPORTEXPORT_READSTAT)
            add_subdirectory(ReadStat)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_MATIO "Enable Matio Tests" ON)
    IF (MATIO_FOUND AND ENABLE_TEST_IMPORTEXPORT_MATIO)
            add_subdirectory(Matio)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_HDF5 "Enable HDF5 Tests" ON)
    IF (HDF5_FOUND AND ENABLE_TEST_IMPORTEXPORT_HDF5)
            add_subdirectory(HDF5)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_CFITSIO "Enable CFitsio Tests" ON)
    IF (CFITSIO_FOUND AND ENABLE_TEST_IMPORTEXPORT_CFITSIO)
            add_subdirectory(FITS)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_NETCDF "Enable Netcdf Tests" ON)
    IF (NETCDF_FOUND AND ENABLE_TEST_IMPORTEXPORT_NETCDF)
            add_subdirectory(NetCDF)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_EXCEL "Enable Excel Tests" ON)
    IF ((QXLSX_FOUND OR BUILD_EXCEL) AND ENABLE_TEST_IMPORTEXPORT_EXCEL)
            add_subdirectory(XLSX)
    ENDIF ()

    option(ENABLE_TEST_IMPORTEXPORT_ORCUS "Enable Orcus Tests" ON)
    IF (ORCUS_FOUND AND ENABLE_TEST_IMPORTEXPORT_ORCUS)
            add_subdirectory(Ods)
    ENDIF ()

    add_subdirectory(Datasets)

endif()

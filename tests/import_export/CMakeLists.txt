option(ENABLE_TEST_IMPORTEXPORT "Enable Import/Export Tests" ON)

if(ENABLE_TEST_IMPORTEXPORT)
    option(ENABLE_TEST_IMPORTEXPORT_ASCII "Enable Ascii Tests" ON)
    if(ENABLE_TEST_IMPORTEXPORT_ASCII)
        add_subdirectory(ASCII)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_BINARY "Enable Binary Tests" ON)
    if(ENABLE_TEST_IMPORTEXPORT_BINARY)
        add_subdirectory(Binary)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_CAN "Enable CAN Bus related Tests" ON)
    if(ENABLE_TEST_IMPORTEXPORT_CAN)
        add_subdirectory(CAN)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_CFITSIO "Enable CFitsio Tests" ON)
    if(CFITSIO_FOUND AND ENABLE_TEST_IMPORTEXPORT_CFITSIO)
        add_subdirectory(FITS)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_DATASET "Enable Dataset Tests" ON)
    if(ENABLE_TEST_IMPORTEXPORT_DATASET)
        add_subdirectory(Datasets)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_HDF5 "Enable HDF5 Tests" ON)
    if(HDF5_FOUND AND ENABLE_TEST_IMPORTEXPORT_HDF5)
        add_subdirectory(HDF5)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_JSON "Enable Json Tests" ON)
    if(ENABLE_TEST_IMPORTEXPORT_JSON)
        add_subdirectory(JSON)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_MATIO "Enable Matio Tests" ON)
    if(MATIO_FOUND AND ENABLE_TEST_IMPORTEXPORT_MATIO)
        add_subdirectory(Matio)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_MCAP "Enable MCAP Tests" ON)
    if(ENABLE_MCAP AND ENABLE_TEST_IMPORTEXPORT_MCAP)
        add_subdirectory(MCAP)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_MQTT "Enable MQTT Tests" ON)
    if(Qt${QT_VERSION_MAJOR}Mqtt_FOUND AND ENABLE_TEST_IMPORTEXPORT_MQTT)
    add_subdirectory(MQTT)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_NETCDF "Enable Netcdf Tests" ON)
    if(NETCDF_FOUND AND ENABLE_TEST_IMPORTEXPORT_NETCDF)
        add_subdirectory(NetCDF)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_ODS "Enable ODS Tests" ON)
    if(ORCUS_FOUND AND ENABLE_TEST_IMPORTEXPORT_ODS)
        add_subdirectory(Ods)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_PROJECT "Enable Project import Tests" ON)
    if (ENABLE_TEST_IMPORTEXPORT_PROJECT)
        add_subdirectory(Project)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_SPICE "Enable Spice Tests" ON)
    if (ENABLE_TEST_IMPORTEXPORT_SPICE)
        add_subdirectory(Spice)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_SQL_DATABASE "Enable SQL Database Tests" ON)
    if (ENABLE_TEST_IMPORTEXPORT_SQL_DATABASE)
        add_subdirectory(SQL)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_READSTAT "Enable Readstat Tests" ON)
    if((READSTAT_FOUND OR BUILD_READSTAT) AND ENABLE_TEST_IMPORTEXPORT_READSTAT)
        add_subdirectory(ReadStat)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_ROOT "Enable Root Tests" ON)
    if(ZLIB_FOUND AND LZ4_FOUND AND ENABLE_TEST_IMPORTEXPORT_ROOT)
        add_subdirectory(ROOT)
    endif()

    option(ENABLE_TEST_IMPORTEXPORT_XLSX "Enable XLSX Tests" ON)
    if((QXLSX_FOUND OR BUILD_QXLSX) AND ENABLE_TEST_IMPORTEXPORT_XLSX)
        add_subdirectory(XLSX)
    endif()

    option(ENABLE_TEST_SERIAL_PORT "Enable Serial Port Tests" ON)
    if(ENABLE_QTSERIALPORT AND ENABLE_TEST_SERIAL_PORT)
        add_subdirectory(SerialPort)
    endif()
endif()

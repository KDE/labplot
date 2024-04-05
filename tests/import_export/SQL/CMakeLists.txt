option(ENABLE_TEST_IMPORTEXPORT_SQL_DATABASE "Enable SQL Database Tests" ON)

if (ENABLE_TEST_IMPORTEXPORT_SQL_DATABASE)

add_executable (ImportSqlDatabaseTest ImportSqlDatabaseTest.cpp)

target_link_libraries(ImportSqlDatabaseTest labplot2backendlib labplot2lib labplot2nsllib labplot2test)

add_test(NAME ImportSqlDatabaseTest COMMAND ImportSqlDatabaseTest)

endif()

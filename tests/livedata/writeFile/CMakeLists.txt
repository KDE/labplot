add_executable (LiveDataWriteFileTest LiveDataWriteFileTest.cpp)

target_link_libraries(LiveDataWriteFileTest labplot2lib Qt5::Test labplot2test)

add_test(NAME LiveDataWriteFileTest COMMAND LiveDataWriteFileTest)

set(EXPORT_FILE "${CMAKE_CURRENT_BINARY_DIR}/exportFile.csv")
set(SLEEP_TIME_MS 100)

add_executable (LiveDataWriteFile LiveDataWriteFile.cpp)
set_property(TARGET LiveDataWriteFile PROPERTY AUTOMOC ON)
target_link_libraries(LiveDataWriteFile Qt5::Core Qt5::SerialPort)
target_compile_definitions(LiveDataWriteFile PRIVATE EXPORT_FILE="${EXPORT_FILE}" SLEEP_TIME_MS=${SLEEP_TIME_MS})

target_compile_definitions(LiveDataWriteFileTest PRIVATE
    LIVEDATAWRITEFILE_EXEC="${CMAKE_CURRENT_BINARY_DIR}/LiveDataWriteFile"
    SLEEP_TIME_MS=${SLEEP_TIME_MS}
    EXPORT_FILE="${EXPORT_FILE}"
)

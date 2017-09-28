add_executable (asciifiltertest AsciiFilterTest.cpp
)

target_link_libraries(asciifiltertest ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTGUI_LIBRARY})
add_test(NAME asciifiltertest COMMAND asciifiltertest)

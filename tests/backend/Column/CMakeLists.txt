option(ENABLE_TEST_BACKEND_COLUMN "Enable Column Tests" ON)

if(ENABLE_TEST_BACKEND_COLUMN)
	add_executable (ColumnTest ColumnTest.cpp)

	target_link_libraries(ColumnTest labplotmetatest)

	add_test(NAME ColumnTest COMMAND ColumnTest)
endif()

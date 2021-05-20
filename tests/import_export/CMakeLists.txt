add_subdirectory(ASCII)
add_subdirectory(JSON)
add_subdirectory(MQTT)
add_subdirectory(project)

IF (READSTAT_FOUND OR NOT WIN32)
	add_subdirectory(ReadStat)
ENDIF ()
IF (MATIO_FOUND)
	add_subdirectory(Matio)
ENDIF ()

# add_subdirectory(DATASETS)

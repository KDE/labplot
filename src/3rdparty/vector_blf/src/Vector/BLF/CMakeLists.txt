# SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
#
# SPDX-License-Identifier: GPL-3.0-or-later

# dependencies
include(GenerateExportHeader)

# targets
add_library(${PROJECT_NAME} STATIC "")

# search paths
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/src
    ${ZLIB_INCLUDE_DIRS})

# sources/headers
target_sources(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/A429BusStatistic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/A429Error.h
        ${CMAKE_CURRENT_SOURCE_DIR}/A429Message.h
        ${CMAKE_CURRENT_SOURCE_DIR}/A429Status.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AbstractFile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxBusStatistic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxErrorEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxFrame.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxLineStatus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxStatistic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxStatus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AppText.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AppTrigger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/AttributeEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverErrorExt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverHwSync.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverStatistic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanErrorFrameExt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanErrorFrame.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdErrorFrame64.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdExtFrameData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdMessage64.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdMessage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanMessage2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanMessage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanOverloadFrame.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CanSettingChanged.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CompactSerialEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/CompressedFile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DataLostBegin.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DataLostEnd.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DiagRequestInterpretation.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DistributedObjectMember.h
        ${CMAKE_CURRENT_SOURCE_DIR}/DriverOverrun.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EnvironmentVariable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetErrorEx.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetErrorForwarded.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetFrameEx.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetFrameForwarded.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetFrame.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetRxError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetStatistic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetStatus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/EventComment.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Exceptions.h
        ${CMAKE_CURRENT_SOURCE_DIR}/File.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FileStatistics.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayData.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayStatusEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRaySync.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayV6Message.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayV6StartCycleEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrReceiveMsgEx.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrReceiveMsg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrStartCycle.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrStatus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/FunctionBus.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GeneralSerialEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GlobalMarker.h
        ${CMAKE_CURRENT_SOURCE_DIR}/GpsEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/J1708Message.h
        ${CMAKE_CURRENT_SOURCE_DIR}/KLineStatusEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinBaudrateEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinBusEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinChecksumInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinCrcError2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinCrcError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinDatabyteTimestampEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinDisturbanceEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinDlcInfo.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinLongDomSignalEvent2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinLongDomSignalEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinMessage2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinMessageDescriptor.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinMessage.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinReceiveError2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinReceiveError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSchedulerModeChange.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSendError2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSendError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinShortOrSlowResponse2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinShortOrSlowResponse.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSlaveTimeout.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSleepModeEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSpikeEvent2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSpikeEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinStatisticEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSyncError2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSyncError.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSynchFieldEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinUnexpectedWakeup.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinWakeupEvent2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LinWakeupEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/LogContainer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150AllocTab.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150MessageFragment.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150Message.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150PktFragment.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150Pkt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most50Message.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Most50Pkt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostAllocTab.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostCtrl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostDataLost.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostEcl.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostEthernetPktFragment.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostEthernetPkt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostGenReg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostHwMode.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostLightLock.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostNetState.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostPkt2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostPkt.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostReg.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostSpy.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostStatisticEx.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostStatistic.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostStress.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostSystemEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostTrigger.h
        ${CMAKE_CURRENT_SOURCE_DIR}/MostTxLight.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectHeader2.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectHeaderBase.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectQueue.h
        ${CMAKE_CURRENT_SOURCE_DIR}/platform.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RealtimeClock.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RestorePoint.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RestorePointContainer.h
        ${CMAKE_CURRENT_SOURCE_DIR}/RestorePoints.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SerialEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SingleByteSerialEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/SystemVariable.h
        ${CMAKE_CURRENT_SOURCE_DIR}/TestStructure.h
        ${CMAKE_CURRENT_SOURCE_DIR}/TriggerCondition.h
        ${CMAKE_CURRENT_SOURCE_DIR}/UncompressedFile.h
        ${CMAKE_CURRENT_SOURCE_DIR}/VarObjectHeader.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WaterMarkEvent.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WlanFrame.h
        ${CMAKE_CURRENT_SOURCE_DIR}/WlanStatistic.h
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/A429BusStatistic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/A429Error.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/A429Message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/A429Status.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AbstractFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxBusStatistic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxErrorEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxLineStatus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxStatistic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AfdxStatus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AppText.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AppTrigger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/AttributeEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverErrorExt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverHwSync.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanDriverStatistic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanErrorFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanErrorFrameExt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdErrorFrame64.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdExtFrameData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdMessage64.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanFdMessage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanMessage2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanMessage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanOverloadFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CanSettingChanged.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CompactSerialEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/CompressedFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DataLostBegin.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DataLostEnd.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DiagRequestInterpretation.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DistributedObjectMember.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/DriverOverrun.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EnvironmentVariable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetErrorEx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetErrorForwarded.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetFrameEx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetFrameForwarded.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetRxError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetStatistic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EthernetStatus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/EventComment.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/File.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FileStatistics.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayData.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayStatusEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRaySync.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayV6Message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayV6StartCycleEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrReceiveMsg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrReceiveMsgEx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrStartCycle.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FlexRayVFrStatus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/FunctionBus.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GeneralSerialEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GlobalMarker.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GpsEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/J1708Message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/KLineStatusEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinBaudrateEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinBusEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinChecksumInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinCrcError2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinCrcError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinDatabyteTimestampEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinDisturbanceEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinDlcInfo.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinLongDomSignalEvent2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinLongDomSignalEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinMessage2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinMessage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinMessageDescriptor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinReceiveError2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinReceiveError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSchedulerModeChange.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSendError2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSendError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinShortOrSlowResponse2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinShortOrSlowResponse.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSlaveTimeout.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSleepModeEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSpikeEvent2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSpikeEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinStatisticEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSyncError2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSyncError.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinSynchFieldEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinUnexpectedWakeup.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinWakeupEvent2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LinWakeupEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/LogContainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150AllocTab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150Message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150MessageFragment.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150Pkt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most150PktFragment.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most50Message.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Most50Pkt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostAllocTab.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostCtrl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostDataLost.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostEcl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostEthernetPkt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostEthernetPktFragment.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostGenReg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostHwMode.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostLightLock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostNetState.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostPkt2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostPkt.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostReg.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostSpy.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostStatistic.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostStatisticEx.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostStress.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostSystemEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostTrigger.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MostTxLight.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectHeader2.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectHeaderBase.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectHeader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/ObjectQueue.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RealtimeClock.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RestorePoint.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RestorePointContainer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/RestorePoints.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SerialEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SingleByteSerialEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/SystemVariable.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TestStructure.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/TriggerCondition.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/UncompressedFile.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/VarObjectHeader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WaterMarkEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WlanFrame.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/WlanStatistic.cpp)

# generated files
configure_file(config.h.in config.h)
configure_file(${PROJECT_NAME}.pc.in ${PROJECT_NAME}.pc @ONLY)
generate_export_header(${PROJECT_NAME})

# compiler/linker settings
set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
    CXX_VISIBILITY_PRESET "hidden"
    SOVERSION ${PROJECT_VERSION_MAJOR}
    VERSION ${PROJECT_VERSION}
    VISIBILITY_INLINES_HIDDEN 1)
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-pedantic)
    add_definitions(-D_FORTIFY_SOURCE=2)
    #add_definitions(-fPIE -pie)
    add_definitions(-Wl,-z,relro,-z,now)
    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
        add_definitions(-fstack-protector-strong)
    endif()
    if(OPTION_USE_GCOV)
        add_definitions(-g -O0 --coverage)
    endif()
    if(OPTION_USE_GPROF)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
    endif()
endif()
target_link_libraries(${PROJECT_NAME}
    Threads::Threads
    ${ZLIB_LIBRARIES})
if(OPTION_USE_GCOV)
    target_link_libraries(${PROJECT_NAME} gcov)
endif()

# install
install(
    TARGETS ${PROJECT_NAME})

if (NOT FETCH_CONTENT_INCLUSION)
    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
    install(
        FILES
            ${CMAKE_CURRENT_BINARY_DIR}/vector_blf_export.h
            ${CMAKE_CURRENT_BINARY_DIR}/config.h
            $<TARGET_PROPERTY:${PROJECT_NAME},INTERFACE_SOURCES>
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/Vector/BLF)
endif()

# sub directories
if (OPTION_RUN_DOXYGEN)
    add_subdirectory(docs)
endif()

if (OPTION_BUILD_TESTS)
    add_subdirectory(tests)
endif()

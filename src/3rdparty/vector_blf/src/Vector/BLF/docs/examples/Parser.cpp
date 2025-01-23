// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <array>
#include <codecvt>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <locale>
#include <map>

#include <Vector/BLF.h>

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

void printData(const uint8_t * data, size_t size) {
    if ((data == nullptr) || (size == 0))
        return;

    std::cout << "[";
    for (size_t i = 0; i < size; ++i) {
        uint16_t value = data[i];
        if (i > 0)
            std::cout << " ";
        std::cout << std::setfill('0') << std::setw(2) << std::hex << static_cast<uint16_t>(value);
    }
    std::cout << "]";
}

void printString(char * data, size_t size) {
    if ((data == nullptr) || (size == 0))
        return;

    for (size_t i = 0; i < size; ++i)
        std::cout << static_cast<char>(data[i]);
}

void show(Vector::BLF::FileStatistics * obj) {
    const std::map<uint8_t, std::string> applicationIdStr = {
        { 0, "Unknown" },
        { 1, "CANalyzer" },
        { 2, "CANoe" },
        { 3, "CANstress" },
        { 4, "CANlog" },
        { 5, "CANape" },
        { 6, "CANcaseXL Log" },
        { 7, "Vector Logger Configurator" },
        { 200, "Porsche Logger" },
        { 201, "CAETEC Logger" },
        { 202, "Vector Network Simulator" },
        { 203, "IPETRONIK Logger" },
        { 204, "RT PK" },
        { 205, "PikeTec" },
        { 206, "Sparks" }
    };
    const std::array<std::string, 7> dayOfWeekStr = {
        {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" }
    };
    std::cout << "FileStatistics:" << std::endl;
    std::cout << "  statisticsSize: "
              << "0x" << std::hex << obj->statisticsSize << std::endl;
    uint32_t apiNumber = obj->apiNumber;
    uint8_t apiPatch = apiNumber % 100;
    apiNumber /= 100;
    uint8_t apiBuild = apiNumber % 100;
    apiNumber /= 100;
    uint8_t apiMinor = apiNumber % 100;
    apiNumber /= 100;
    uint8_t apiMajor = apiNumber;
    std::cout << "  apiNumber: "
              << std::dec << static_cast<uint16_t>(apiMajor) << "."
              << std::dec << static_cast<uint16_t>(apiMinor) << "."
              << std::dec << static_cast<uint16_t>(apiBuild) << "."
              << std::dec << static_cast<uint16_t>(apiPatch) << std::endl;
    if (applicationIdStr.count(obj->applicationId)) {
        std::cout << "  applicationId: "
                  << std::dec << static_cast<uint16_t>(obj->applicationId)
                  << " (" << applicationIdStr.at(obj->applicationId) << ")" << std::endl;
    } else {
        std::cout << "  applicationId: "
                  << std::dec << static_cast<uint16_t>(obj->applicationId) << std::endl;
    }
    std::cout << "  applicationVersion: "
              << std::dec << static_cast<uint16_t>(obj->applicationMajor) << "."
              << std::dec << static_cast<uint16_t>(obj->applicationMinor) << "."
              << std::dec << obj->applicationBuild << std::endl;
    std::cout << "  fileSize: "
              << std::dec << obj->fileSize
              << " (0x" << std::hex << obj->fileSize << ")"
              << std::endl;
    std::cout << "  uncompressedFileSize: " << std::dec << obj->uncompressedFileSize
              << " (0x" << std::hex << obj->uncompressedFileSize << ")"
              << std::endl;
    std::cout << "  objectCount: "
              << std::dec << obj->objectCount << std::endl;
    std::cout << "  compressionLevel: "
              << std::dec << static_cast<uint16_t>(obj->compressionLevel) << std::endl;
    std::cout << "  measurementStartTime: "
              << std::dec << std::setfill('0') << std::setw(4) << obj->measurementStartTime.year << "-"
              << std::dec << std::setfill('0') << std::setw(2) << obj->measurementStartTime.month << "-"
              << std::dec << std::setfill('0') << std::setw(2) << obj->measurementStartTime.day << " "
              << dayOfWeekStr[obj->measurementStartTime.dayOfWeek % 7] << " "
              << std::dec << std::setfill('0') << std::setw(2) << obj->measurementStartTime.hour << ":"
              << std::dec << std::setfill('0') << std::setw(2) << obj->measurementStartTime.minute << ":"
              << std::dec << std::setfill('0') << std::setw(2) << obj->measurementStartTime.second << "."
              << std::dec << std::setfill('0') << std::setw(3) << obj->measurementStartTime.milliseconds
              << std::endl;
    std::cout << "  lastObjectTime: "
              << std::dec << std::setfill('0') << std::setw(4) << obj->lastObjectTime.year << "-"
              << std::dec << std::setfill('0') << std::setw(2) << obj->lastObjectTime.month << "-"
              << std::dec << std::setfill('0') << std::setw(2) << obj->lastObjectTime.day << " "
              << dayOfWeekStr[obj->lastObjectTime.dayOfWeek % 7] << " "
              << std::dec << std::setfill('0') << std::setw(2) << obj->lastObjectTime.hour << ":"
              << std::dec << std::setfill('0') << std::setw(2) << obj->lastObjectTime.minute << ":"
              << std::dec << std::setfill('0') << std::setw(2) << obj->lastObjectTime.second << "."
              << std::dec << std::setfill('0') << std::setw(3) << obj->lastObjectTime.milliseconds
              << std::endl;
    std::cout << "  restorePointsOffset: "
              << "0x" << std::hex << obj->restorePointsOffset
              << std::endl;
    std::cout << std::endl;
}

// UNKNOWN = 0

// CAN_MESSAGE = 1
void show(Vector::BLF::CanMessage * obj) {
    std::cout << "CanMessage:"
              << " channel=" << obj->channel
              << " flags=" << static_cast<uint16_t>(obj->flags)
              << " dlc=" << static_cast<uint16_t>(obj->dlc)
              << " id=0x" << std::hex << obj->id
              << " data=";
    printData(obj->data.data(), min(obj->data.size(), obj->dlc));
    std::cout << std::endl;
}

// CAN_ERROR = 2
void show(Vector::BLF::CanErrorFrame * obj) {
    std::cout << "CanErrorFrame:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " length=" << std::dec << obj->length;
    std::cout << " reserved=0x" << std::hex << obj->reservedCanErrorFrame;
    std::cout << std::endl;
}

// CAN_OVERLOAD = 3
void show(Vector::BLF::CanOverloadFrame * obj) {
    std::cout << "CanOverloadFrame:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << std::endl;
}

// CAN_STATISTIC = 4
void show(Vector::BLF::CanDriverStatistic * obj) {
    std::cout << "CanDriverStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " busLoad=" << std::dec << obj->busLoad;
    std::cout << " standardDataFrames=" << std::dec << obj->standardDataFrames;
    std::cout << " extendedDataFrames=" << std::dec << obj->extendedDataFrames;
    std::cout << " standardRemoteFrames=" << std::dec << obj->standardRemoteFrames;
    std::cout << " extendedRemoteFrames=" << std::dec << obj->extendedRemoteFrames;
    std::cout << " errorFrames=" << std::dec << obj->errorFrames;
    std::cout << " overloadFrames=" << std::dec << obj->overloadFrames;
    std::cout << std::endl;
}

// APP_TRIGGER = 5
void show(Vector::BLF::AppTrigger * obj) {
    std::cout << "AppTrigger:";
    std::cout << " preTriggerTime=" << std::dec << obj->preTriggerTime;
    std::cout << " postTriggerTime=" << std::dec << obj->postTriggerTime;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " appSpecific2=" << std::dec << obj->appSpecific2;
    std::cout << std::endl;
}

// ENV_INTEGER = 6
// ENV_DOUBLE = 7
// ENV_STRING = 8
// ENV_DATA = 9
void show(Vector::BLF::EnvironmentVariable * obj) {
    std::cout << "EnvironmentVariable:";
    std::cout << " name=" << obj->name;
    switch (obj->objectType) {
    case Vector::BLF::ObjectType::ENV_INTEGER:
        std::cout << " type=Integer";
        std::cout << " value=" << std::dec << static_cast<int>(*obj->data.data());
        break;
    case Vector::BLF::ObjectType::ENV_DOUBLE:
        std::cout << " type=Double";
        std::cout << " value=" << std::fixed << static_cast<double>(*obj->data.data());
        break;
    case Vector::BLF::ObjectType::ENV_STRING:
        std::cout << " type=String";
        std::cout << " value=";
        printString(reinterpret_cast<char *>(obj->data.data()), obj->dataLength);
        break;
    case Vector::BLF::ObjectType::ENV_DATA:
        std::cout << " type=Data";
        std::cout << " value=";
        printData(obj->data.data(), min(obj->data.size(), obj->dataLength));
        break;
    default:
        std::cout << " type=Unknown";
        break;
    }
    std::cout << std::endl;
}

// LOG_CONTAINER = 10
void show(Vector::BLF::LogContainer * obj) {
    std::cout << "LogContainer:";
    std::cout << " compressionMethod=" << std::dec << obj->compressionMethod;
    std::cout << " uncompressedFileSize=" << std::dec << obj->uncompressedFileSize;
    std::cout << std::endl;
}

// LIN_MESSAGE = 11
void show(Vector::BLF::LinMessage * obj) {
    std::cout << "LinMessage:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " data=";
    printData(obj->data.data(), min(obj->data.size(), obj->dlc));
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " headerTime=" << std::dec << static_cast<uint16_t>(obj->headerTime);
    std::cout << " fullTime=" << std::dec << static_cast<uint16_t>(obj->fullTime);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << std::endl;
}

// LIN_CRC_ERROR = 12
void show(Vector::BLF::LinCrcError * obj) {
    std::cout << "LinCrcError:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " data=";
    printData(obj->data.data(), min(obj->data.size(), obj->dlc));
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " headerTime=" << std::dec << static_cast<uint16_t>(obj->headerTime);
    std::cout << " fullTime=" << std::dec << static_cast<uint16_t>(obj->fullTime);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << std::endl;
}

// LIN_DLC_INFO = 13
void show(Vector::BLF::LinDlcInfo * obj) {
    std::cout << "LinDlcInfo:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << std::endl;
}

// LIN_RCV_ERROR =  14
void show(Vector::BLF::LinReceiveError * obj) {
    std::cout << "LinReceiveError:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " headerTime=" << std::dec << static_cast<uint16_t>(obj->headerTime);
    std::cout << " fullTime=" << std::dec << static_cast<uint16_t>(obj->fullTime);
    std::cout << " stateReason=" << std::dec << static_cast<uint16_t>(obj->stateReason);
    std::cout << " offendingByte=" << std::dec << static_cast<uint16_t>(obj->offendingByte);
    std::cout << " shortError=" << std::dec << static_cast<uint16_t>(obj->shortError);
    std::cout << " timeoutDuringDlcDetection=" << std::dec << static_cast<uint16_t>(obj->timeoutDuringDlcDetection);
    std::cout << std::endl;
}

// LIN_SND_ERROR = 15
void show(Vector::BLF::LinSendError * obj) {
    std::cout << "LinSendError:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " headerTime=" << std::dec << static_cast<uint16_t>(obj->headerTime);
    std::cout << " fullTime=" << std::dec << static_cast<uint16_t>(obj->fullTime);
    std::cout << std::endl;
}

// LIN_SLV_TIMEOUT = 16
void show(Vector::BLF::LinSlaveTimeout * obj) {
    std::cout << "LinSlaveTimeout:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " slaveId=" << std::dec << static_cast<uint16_t>(obj->slaveId);
    std::cout << " stateId=" << std::dec << static_cast<uint16_t>(obj->stateId);
    std::cout << " followStateId=" << std::dec << obj->followStateId;
    std::cout << std::endl;
}

// LIN_SCHED_MODCH = 17
void show(Vector::BLF::LinSchedulerModeChange * obj) {
    std::cout << "LinSchedulerModeChange:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " oldMode=" << std::dec << static_cast<uint16_t>(obj->oldMode);
    std::cout << " newMode=" << std::dec << static_cast<uint16_t>(obj->newMode);
    std::cout << std::endl;
}

// LIN_SYN_ERROR = 18
void show(Vector::BLF::LinSyncError * obj) {
    std::cout << "LinSyncError:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " timeDiff[0]=" << std::dec << obj->timeDiff[0];
    std::cout << " timeDiff[1]=" << std::dec << obj->timeDiff[1];
    std::cout << " timeDiff[2]=" << std::dec << obj->timeDiff[2];
    std::cout << " timeDiff[3]=" << std::dec << obj->timeDiff[3];
    std::cout << std::endl;
}

// LIN_BAUDRATE = 19
void show(Vector::BLF::LinBaudrateEvent * obj) {
    std::cout << "LinBaudrateEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " baudrate=" << std::dec << obj->baudrate;
    std::cout << std::endl;
}

// LIN_SLEEP = 20
void show(Vector::BLF::LinSleepModeEvent * obj) {
    std::cout << "LinSleepModeEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " reason=" << std::dec << static_cast<uint16_t>(obj->reason);
    std::cout << " flags=0x" << std::hex << static_cast<uint16_t>(obj->flags);
    std::cout << std::endl;
}

// LIN_WAKEUP = 21
void show(Vector::BLF::LinWakeupEvent * obj) {
    std::cout << "LinWakeupEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " signal=" << std::dec << static_cast<uint16_t>(obj->signal);
    std::cout << " external=" << std::dec << static_cast<uint16_t>(obj->external);
    std::cout << std::endl;
}

// MOST_SPY = 22
void show(Vector::BLF::MostSpy * obj) {
    std::cout << "MostSpy:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " msg=";
    printData(obj->msg.data(), obj->msg.size());
    std::cout << " rTyp=0x" << std::hex << obj->rTyp;
    std::cout << " rTypAdr=0x" << std::hex << static_cast<uint16_t>(obj->rTypAdr);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << std::endl;
}

// MOST_CTRL = 23
void show(Vector::BLF::MostCtrl * obj) {
    std::cout << "MostCtrl:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " msg=";
    printData(obj->msg.data(), obj->msg.size());
    std::cout << " rTyp=0x" << std::hex << obj->rTyp;
    std::cout << " rTypAdr=0x" << std::hex << static_cast<uint16_t>(obj->rTypAdr);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << std::endl;
}

// MOST_LIGHTLOCK = 24
void show(Vector::BLF::MostLightLock * obj) {
    std::cout << "MostLightLock:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " state=0x" << std::hex << obj->state;
    std::cout << std::endl;
}

// MOST_STATISTIC = 25
void show(Vector::BLF::MostStatistic * obj) {
    std::cout << "MostStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " pktCnt=" << std::dec << obj->pktCnt;
    std::cout << " frmCnt=" << std::dec << obj->frmCnt;
    std::cout << " lightCnt=" << std::dec << obj->lightCnt;
    std::cout << " bufferLevel=" << std::dec << obj->bufferLevel;
    std::cout << std::endl;
}

// Reserved26 = 26
// Reserved27 = 27
// Reserved28 = 28

// FLEXRAY_DATA = 29
void show(Vector::BLF::FlexRayData * obj) {
    std::cout << "FlexRayData:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " mux=" << std::dec << static_cast<uint16_t>(obj->mux);
    std::cout << " len=" << std::dec << static_cast<uint16_t>(obj->len);
    std::cout << " messageId=" << std::dec << obj->messageId;
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), min(obj->dataBytes.size(), obj->len));
    std::cout << std::endl;
}

// FLEXRAY_SYNC = 30
void show(Vector::BLF::FlexRaySync * obj) {
    std::cout << "FlexRaySync:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " mux=" << std::dec << static_cast<uint16_t>(obj->mux);
    std::cout << " len=" << std::dec << static_cast<uint16_t>(obj->len);
    std::cout << " messageId=" << std::dec << obj->messageId;
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), min(obj->dataBytes.size(), obj->len));
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << std::endl;
}

// CAN_DRIVER_ERROR = 31
void show(Vector::BLF::CanDriverError * obj) {
    std::cout << "CanDriverError:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " txErrors=" << std::dec << static_cast<uint16_t>(obj->txErrors);
    std::cout << " rxErrors=" << std::dec << static_cast<uint16_t>(obj->rxErrors);
    std::cout << " errorCode=0x" << std::hex << obj->errorCode;
    std::cout << std::endl;
}

// MOST_PKT = 32
void show(Vector::BLF::MostPkt * obj) {
    std::cout << "MostPkt:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " arbitration=" << std::dec << static_cast<uint16_t>(obj->arbitration);
    std::cout << " timeRes=" << std::dec << static_cast<uint16_t>(obj->timeRes);
    std::cout << " quadsToFollow=" << std::dec << static_cast<uint16_t>(obj->quadsToFollow);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " pktDataLength=" << std::dec << obj->pktDataLength;
    std::cout << " pktData=";
    printData(obj->pktData.data(), min(obj->pktData.size(), obj->pktDataLength));
    std::cout << std::endl;
}

// MOST_PKT2 = 33
void show(Vector::BLF::MostPkt2 * obj) {
    std::cout << "MostPkt2:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " arbitration=" << std::dec << static_cast<uint16_t>(obj->arbitration);
    std::cout << " timeRes=" << std::dec << static_cast<uint16_t>(obj->timeRes);
    std::cout << " quadsToFollow=" << std::dec << static_cast<uint16_t>(obj->quadsToFollow);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " pktDataLength=" << std::dec << obj->pktDataLength;
    std::cout << " pktData=";
    printData(obj->pktData.data(), min(obj->pktData.size(), obj->pktDataLength));
    std::cout << std::endl;
}

// MOST_HWMODE = 34
void show(Vector::BLF::MostHwMode * obj) {
    std::cout << "MostHwMode:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " hwMode=0x" << std::hex << obj->hwMode;
    std::cout << " hwModeMask=0x" << std::hex << obj->hwModeMask;
    std::cout << std::endl;
}

// MOST_REG = 35
void show(Vector::BLF::MostReg * obj) {
    std::cout << "MostReg:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " subType=" << std::dec << static_cast<uint16_t>(obj->subType);
    std::cout << " handle=" << std::dec << obj->handle;
    std::cout << " offset=" << std::dec << obj->offset;
    std::cout << " chip=" << std::dec << obj->chip;
    std::cout << " regDataLen=" << std::dec << obj->regDataLen;
    std::cout << " regData=";
    printData(obj->regData.data(), min(obj->regData.size(), obj->regDataLen));
    std::cout << std::endl;
}

// MOST_GENREG = 36
void show(Vector::BLF::MostGenReg * obj) {
    std::cout << "MostGenReg:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " subType=" << std::dec << static_cast<uint16_t>(obj->subType);
    std::cout << " handle=" << std::dec << obj->handle;
    std::cout << " regId=0x" << std::hex << obj->regId;
    std::cout << " regVal=0x" << std::hex << obj->regValue;
    std::cout << std::endl;
}

// MOST_NETSTATE = 37
void show(Vector::BLF::MostNetState * obj) {
    std::cout << "MostNetState:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " stateNew=" << std::dec << obj->stateNew;
    std::cout << " stateOld=" << std::dec << obj->stateOld;
    std::cout << std::endl;
}

// MOST_DATALOST = 38
void show(Vector::BLF::MostDataLost * obj) {
    std::cout << "MostDataLost:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " info=0x" << std::hex << obj->info;
    std::cout << " lostMsgsCtrl=" << std::dec << obj->lostMsgsCtrl;
    std::cout << " lostMsgsAsync=" << std::dec << obj->lostMsgsAsync;
    std::cout << " lastGoodTimeStampNs=" << std::dec << obj->lastGoodTimeStampNs;
    std::cout << " nextGoodTimeStampNs=" << std::dec << obj->nextGoodTimeStampNs;
    std::cout << std::endl;
}

// MOST_TRIGGER = 39
void show(Vector::BLF::MostTrigger * obj) {
    std::cout << "MostTrigger:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " mode=" << std::dec << obj->mode;
    std::cout << " hw=" << std::dec << obj->hw;
    std::cout << " previousTriggerValue=" << std::dec << obj->previousTriggerValue;
    std::cout << " currentTriggerValue=" << std::dec << obj->currentTriggerValue;
    std::cout << std::endl;
}

// FLEXRAY_CYCLE = 40
void show(Vector::BLF::FlexRayV6StartCycleEvent * obj) {
    std::cout << "FlexRayV6StartCycleEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " lowTime=" << std::dec << static_cast<uint16_t>(obj->lowTime);
    std::cout << " fpgaTick=" << std::dec << obj->fpgaTick;
    std::cout << " fpgaTickOverflow=" << std::dec << obj->fpgaTickOverflow;
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayV6StartCycleEvent;
    std::cout << " clusterTime=" << std::dec << obj->clusterTime;
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), obj->dataBytes.size());
    std::cout << std::endl;
}

// FLEXRAY_MESSAGE = 41
void show(Vector::BLF::FlexRayV6Message * obj) {
    std::cout << "FlexRayV6Message:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " lowTime=" << std::dec << static_cast<uint16_t>(obj->lowTime);
    std::cout << " fpgaTick=" << std::dec << obj->fpgaTick;
    std::cout << " fpgaTickOverflow=" << std::dec << obj->fpgaTickOverflow;
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayV6Message;
    std::cout << " clusterTime=" << std::dec << obj->clusterTime;
    std::cout << " frameId=" << std::dec << obj->frameId;
    std::cout << " headerCrc=0x" << std::hex << obj->headerCrc;
    std::cout << " frameState=0x" << std::hex << obj->frameState;
    std::cout << " length=" << std::dec << static_cast<uint16_t>(obj->length);
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->cycle);
    std::cout << " headerBitMask=" << std::dec << static_cast<uint16_t>(obj->headerBitMask);
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), obj->dataBytes.size());
    std::cout << std::endl;
}

// LIN_CHECKSUM_INFO = 42
void show(Vector::BLF::LinChecksumInfo * obj) {
    std::cout << "LinChecksumInfo:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " checksumModel=" << std::dec << static_cast<uint16_t>(obj->checksumModel);
    std::cout << std::endl;
}

// LIN_SPIKE_EVENT = 43
void show(Vector::BLF::LinSpikeEvent * obj) {
    std::cout << "LinSpikeEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " width=" << std::dec << obj->width;
    std::cout << std::endl;
}

// CAN_DRIVER_SYNC = 44
void show(Vector::BLF::CanDriverHwSync * obj) {
    std::cout << "CanDriverHwSync:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << static_cast<uint16_t>(obj->flags);
    std::cout << std::endl;
}

// FLEXRAY_STATUS = 45
void show(Vector::BLF::FlexRayStatusEvent * obj) {
    std::cout << "FlexRayStatusEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " version=" << std::dec << obj->version;
    std::cout << " statusType=" << std::dec << obj->statusType;
    std::cout << " infoMask1=0x" << std::hex << obj->infoMask1;
    std::cout << " infoMask2=0x" << std::hex << obj->infoMask2;
    std::cout << " infoMask3=0x" << std::hex << obj->infoMask3;
    std::cout << std::endl;
}

// GPS_EVENT = 46
void show(Vector::BLF::GpsEvent * obj) {
    std::cout << "GpsEvent:";
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " latitude=" << std::fixed << obj->latitude;
    std::cout << " longitude=" << std::fixed << obj->longitude;
    std::cout << " altitude=" << std::fixed << obj->altitude;
    std::cout << " speed=" << std::fixed << obj->speed;
    std::cout << " course=" << std::fixed << obj->course;
    std::cout << std::endl;
}

// FR_ERROR = 47
void show(Vector::BLF::FlexRayVFrError * obj) {
    std::cout << "FlexRayVFrError:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " version=" << std::dec << obj->version;
    std::cout << " channelMask=0x" << std::hex << obj->channelMask;
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->cycle);
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayVFrError;
    std::cout << " clusterNo=" << std::dec << obj->clusterNo;
    std::cout << " tag=" << std::dec << obj->tag;
    std::cout << " data[0]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[0];
    std::cout << " data[1]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[1];
    std::cout << " data[2]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[2];
    std::cout << " data[3]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[3];
    std::cout << std::endl;
}

// FR_STATUS = 48
void show(Vector::BLF::FlexRayVFrStatus * obj) {
    std::cout << "FlexRayVFrStatus:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " version=" << std::dec << obj->version;
    std::cout << " channelMask=0x" << std::hex << obj->channelMask;
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->cycle);
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayVFrStatus;
    std::cout << " clusterNo=" << std::dec << obj->clusterNo;
    std::cout << " wus=" << std::dec << obj->wus;
    std::cout << " ccSyncState=" << std::dec << obj->ccSyncState;
    std::cout << " tag=" << std::dec << obj->tag;
    std::cout << " data[0]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[0];
    std::cout << " data[1]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[1];
    std::cout << " reserved[0]=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->reservedFlexRayVFrStatus2[0];
    std::cout << std::endl;
}

// FR_STARTCYCLE = 49
void show(Vector::BLF::FlexRayVFrStartCycle * obj) {
    std::cout << "FlexRayVFrStartCycle:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " version=" << std::dec << obj->version;
    std::cout << " channelMask=0x" << std::hex << obj->channelMask;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->cycle);
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayVFrStartCycle;
    std::cout << " clusterNo=" << std::dec << obj->clusterNo;
    std::cout << " nmSize=" << std::dec << obj->nmSize;
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), min(obj->dataBytes.size(), obj->nmSize));
    std::cout << " tag=" << std::dec << obj->tag;
    std::cout << " data[0]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[0];
    std::cout << " data[1]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[1];
    std::cout << " data[2]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[2];
    std::cout << " data[3]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[3];
    std::cout << " data[4]=0x" << std::setfill('0') << std::setw(8) << std::hex << obj->data[4];
    std::cout << std::endl;
}

// FR_RCVMESSAGE = 50
void show(Vector::BLF::FlexRayVFrReceiveMsg * obj) {
    std::cout << "FlexRayVFrReceiveMsg:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " version=" << std::dec << obj->version;
    std::cout << " channelMask=0x" << std::hex << obj->channelMask;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayVFrReceiveMsg;
    std::cout << " clusterNo=" << std::dec << obj->clusterNo;
    std::cout << " frameId=" << std::dec << obj->frameId;
    std::cout << " headerCrc1=0x" << std::hex << obj->headerCrc1;
    std::cout << " headerCrc2=0x" << std::hex << obj->headerCrc2;
    std::cout << " byteCount=" << std::dec << obj->byteCount;
    std::cout << " dataCount=" << std::dec << obj->dataCount;
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->cycle);
    std::cout << " tag=" << std::dec << obj->tag;
    std::cout << " data=0x" << std::hex << obj->data;
    std::cout << " frameFlags=0x" << std::hex << obj->frameFlags;
    std::cout << " appParameter=" << std::dec << obj->appParameter;
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), min(obj->dataBytes.size(), obj->dataCount));
    std::cout << std::endl;
}

// REALTIMECLOCK = 51
void show(Vector::BLF::RealtimeClock * obj) {
    std::cout << "RealtimeClock:";
    std::cout << " time=" << std::dec << obj->time;
    std::cout << " loggingOffset=" << std::dec << obj->loggingOffset;
    std::cout << std::endl;
}

// Reserved52 = 52
// Reserved52 = 53

// LIN_STATISTIC = 54
void show(Vector::BLF::LinStatisticEvent * obj) {
    std::cout << "LinStatisticEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " busload=" << std::fixed << obj->busLoad;
    std::cout << " burstsTotal=" << std::dec << obj->burstsTotal;
    std::cout << " burstsOverrun=" << std::dec << obj->burstsOverrun;
    std::cout << " framesSent=" << std::dec << obj->framesSent;
    std::cout << " framesReceived=" << std::dec << obj->framesReceived;
    std::cout << " framesUnanswered=" << std::dec << obj->framesUnanswered;
    std::cout << std::endl;
}

// J1708_MESSAGE = 55
// J1708_VIRTUAL_MSG = 56
void show(Vector::BLF::J1708Message * obj) {
    switch (obj->objectType) {
    case Vector::BLF::ObjectType::J1708_MESSAGE:
        std::cout << "J1708Message:";
        break;
    case Vector::BLF::ObjectType::J1708_VIRTUAL_MSG:
        std::cout << "J1708VirtualMessage:";
        break;
    default:
        std::cout << "Unknown:";
        break;
    }
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " error=" << std::dec << obj->error;
    std::cout << " size=" << std::dec << static_cast<uint16_t>(obj->size);
    std::cout << " data=";
    printData(obj->data.data(), min(obj->data.size(), obj->size));
    std::cout << std::endl;
}

// LIN_MESSAGE2 = 57
void show(Vector::BLF::LinMessage2 * obj) {
    std::cout << "LinMessage2:";
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " simulated=" << std::dec << static_cast<uint16_t>(obj->simulated);
    std::cout << " isEtf=" << std::dec << static_cast<uint16_t>(obj->isEtf);
    std::cout << " etfAssocIndex=" << std::dec << static_cast<uint16_t>(obj->etfAssocIndex);
    std::cout << " etfAssocEtfId=" << std::dec << static_cast<uint16_t>(obj->etfAssocEtfId);
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " respBaudrate=" << std::dec << obj->respBaudrate;
    std::cout << " exactHeaderBaudrate=" << std::fixed << obj->exactHeaderBaudrate;
    std::cout << " earlyStopbitOffset=" << std::dec << obj->earlyStopbitOffset;
    std::cout << " earlyStopbitOffsetResponse=" << std::dec << obj->earlyStopbitOffsetResponse;
    std::cout << std::endl;
}

// LIN_SND_ERROR2 = 58
void show(Vector::BLF::LinSendError2 * obj) {
    std::cout << "LinSendError2:";
    std::cout << " eoh=" << std::dec << obj->eoh;
    std::cout << " isEtf=" << std::dec << static_cast<uint16_t>(obj->isEtf);
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " exactHeaderBaudrate=" << std::fixed << obj->exactHeaderBaudrate;
    std::cout << " earlyStopbitOffset=" << std::dec << obj->earlyStopbitOffset;
    std::cout << std::endl;
}

// LIN_SYN_ERROR2 = 59
void show(Vector::BLF::LinSyncError2 * obj) {
    std::cout << "LinSendError2:";
    std::cout << " timeDiff[0]=" << std::dec << obj->timeDiff[0];
    std::cout << " timeDiff[1]=" << std::dec << obj->timeDiff[1];
    std::cout << " timeDiff[2]=" << std::dec << obj->timeDiff[2];
    std::cout << " timeDiff[3]=" << std::dec << obj->timeDiff[3];
    std::cout << std::endl;
}

// LIN_CRC_ERROR2 = 60
void show(Vector::BLF::LinCrcError2 * obj) {
    std::cout << "LinCrcError2:";
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " simulated=" << std::dec << static_cast<uint16_t>(obj->simulated);
    std::cout << " respBaudrate=" << std::dec << obj->respBaudrate;
    std::cout << " exactHeaderBaudrate=" << std::fixed << obj->exactHeaderBaudrate;
    std::cout << " earlyStopbitOffset=" << std::dec << obj->earlyStopbitOffset;
    std::cout << " earlyStopbitOffsetResponse=" << std::dec << obj->earlyStopbitOffsetResponse;
    std::cout << std::endl;
}

// LIN_RCV_ERROR2 = 61
void show(Vector::BLF::LinReceiveError2 * obj) {
    std::cout << "LinReceiveError2:";
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    std::cout << " fsmId=" << std::dec << static_cast<uint16_t>(obj->fsmId);
    std::cout << " fsmState=" << std::dec << static_cast<uint16_t>(obj->fsmState);
    std::cout << " stateReason=" << std::dec << static_cast<uint16_t>(obj->stateReason);
    std::cout << " offendingByte=" << std::dec << static_cast<uint16_t>(obj->offendingByte);
    std::cout << " shortError=" << std::dec << static_cast<uint16_t>(obj->shortError);
    std::cout << " timeoutDuringDlcDetection=" << std::dec << static_cast<uint16_t>(obj->timeoutDuringDlcDetection);
    std::cout << " isEtf=" << std::dec << static_cast<uint16_t>(obj->isEtf);
    std::cout << " hasDatabytes=" << std::dec << static_cast<uint16_t>(obj->hasDatabytes);
    std::cout << " respBaudrate=" << std::dec << obj->respBaudrate;
    std::cout << " exactHeaderBaudrate=" << std::fixed << obj->exactHeaderBaudrate;
    std::cout << " earlyStopbitOffset=" << std::dec << obj->earlyStopbitOffset;
    std::cout << " earlyStopbitOffsetResponse=" << std::dec << obj->earlyStopbitOffsetResponse;
    std::cout << std::endl;
}

// LIN_WAKEUP2 = 62
void show(Vector::BLF::LinWakeupEvent2 * obj) {
    std::cout << "LinWakeupEvent2:";
    std::cout << " lengthInfo=" << std::dec << static_cast<uint16_t>(obj->lengthInfo);
    std::cout << " signal=" << std::dec << static_cast<uint16_t>(obj->signal);
    std::cout << " external=" << std::dec << static_cast<uint16_t>(obj->external);
    std::cout << std::endl;
}

// LIN_SPIKE_EVENT2 = 63
void show(Vector::BLF::LinSpikeEvent2 * obj) {
    std::cout << "LinSpikeEvent2:";
    std::cout << " width=" << std::dec << obj->width;
    std::cout << " internal=" << std::dec << static_cast<uint16_t>(obj->internal);
    std::cout << std::endl;
}

// LIN_LONG_DOM_SIG = 64
void show(Vector::BLF::LinLongDomSignalEvent * obj) {
    std::cout << "LinLongDomSignalEvent:";
    std::cout << " type=" << std::dec << static_cast<uint16_t>(obj->type);
    std::cout << std::endl;
}

// APP_TEXT = 65
void show(Vector::BLF::AppText * obj) {
    std::cout << "AppText:";
    std::cout << " source=0x" << std::hex << obj->source;
    std::cout << " textLength=" << std::dec << obj->textLength;
    std::cout << " text=" << obj->text;
    std::cout << std::endl;
}

// FR_RCVMESSAGE_EX = 66
void show(Vector::BLF::FlexRayVFrReceiveMsgEx * obj) {
    std::cout << "FlexRayVFrReceiveMsgEx:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " version=" << std::dec << obj->version;
    std::cout << " channelMask=0x" << std::hex << obj->channelMask;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " clientIndex=" << std::dec << obj->clientIndexFlexRayVFrReceiveMsgEx;
    std::cout << " clusterNo=" << std::dec << obj->clusterNo;
    std::cout << " frameId=" << std::dec << obj->frameId;
    std::cout << " headerCrc1=0x" << std::hex << obj->headerCrc1;
    std::cout << " headerCrc2=0x" << std::hex << obj->headerCrc2;
    std::cout << " byteCount=" << std::dec << obj->byteCount;
    std::cout << " dataCount=" << std::dec << obj->dataCount;
    std::cout << " cycle=" << std::dec << static_cast<uint16_t>(obj->cycle);
    std::cout << " tag=" << std::dec << obj->tag;
    std::cout << " data=0x" << std::hex << obj->data;
    std::cout << " frameFlags=0x" << std::hex << obj->frameFlags;
    std::cout << " appParameter=" << std::dec << obj->appParameter;
    std::cout << " frameCrc=0x" << std::hex << obj->frameCrc;
    std::cout << " frameLengthNs=" << std::dec << obj->frameLengthNs;
    std::cout << " frameId1=" << std::dec << obj->frameId1;
    std::cout << " pduOffset=" << std::dec << obj->pduOffset;
    std::cout << " blfLogMask=" << std::dec << obj->blfLogMask;
    std::cout << " dataBytes=";
    printData(obj->dataBytes.data(), min(obj->dataBytes.size(), obj->dataCount));
    std::cout << std::endl;
}

// MOST_STATISTICEX = 67
void show(Vector::BLF::MostStatisticEx * obj) {
    std::cout << "MostStatisticEx:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " codingErrors=" << std::dec << obj->codingErrors;
    std::cout << " frameCounter=" << std::dec << obj->frameCounter;
    std::cout << std::endl;
}

// MOST_TXLIGHT = 68
void show(Vector::BLF::MostTxLight * obj) {
    std::cout << "MostTxLight:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " state=" << std::dec << obj->state;
    std::cout << std::endl;
}

// MOST_ALLOCTAB = 69
void show(Vector::BLF::MostAllocTab * obj) {
    std::cout << "MostAllocTab:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " length=0x" << std::hex << obj->length;
    std::cout << " allocTab=";
    printData(obj->tableData.data(), obj->tableData.size());
    std::cout << std::endl;
}

// MOST_STRESS = 70
void show(Vector::BLF::MostStress * obj) {
    std::cout << "MostStress:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " state=" << std::dec << obj->state;
    std::cout << " mode=" << std::dec << obj->mode;
    std::cout << std::endl;
}

// ETHERNET_FRAME = 71
void show(Vector::BLF::EthernetFrame * obj) {
    std::cout << "EthernetFrame:";
    std::cout << " sourceAddress=";
    printData(obj->sourceAddress.data(), obj->sourceAddress.size());
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " destinationAddress=";
    printData(obj->destinationAddress.data(), obj->destinationAddress.size());
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " type=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->type;
    std::cout << " tpid=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->tpid;
    std::cout << " tci=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->tci;
    std::cout << " payLoadLength=" << std::dec << obj->payLoadLength;
    std::cout << " payLoad=";
    printData(obj->payLoad.data(), obj->payLoad.size());
    std::cout << std::endl;
}

// SYS_VARIABLE = 72
void show(Vector::BLF::SystemVariable * obj) {
    std::cout << "SystemVariable:";
    switch (obj->type) {
    case Vector::BLF::SystemVariable::Type::Double:
        std::cout << " type=Double";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=" << std::fixed << static_cast<double>(*obj->data.data());
        break;
    case Vector::BLF::SystemVariable::Type::Long:
        std::cout << " type=Long";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=" << std::dec << static_cast<long>(*obj->data.data());
        break;
    case Vector::BLF::SystemVariable::Type::String:
        std::cout << " type=String";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=";
        printString(reinterpret_cast<char *>(obj->data.data()), obj->data.size());
        break;
    case Vector::BLF::SystemVariable::Type::DoubleArray:
        std::cout << " type=DoubleArray";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=[";
        for (uint32_t i = 0; i < min(obj->data.size(), obj->dataLength) / 8; ++i) {
            if (i > 0)
                std::cout << ",";
            std::cout << std::fixed << static_cast<double>(obj->data[i * 8]);
        }
        std::cout << "]";
        break;
    case Vector::BLF::SystemVariable::Type::LongArray:
        std::cout << " type=LongArray";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=[";
        for (uint32_t i = 0; i < min(obj->data.size(), obj->dataLength) / 4; ++i) {
            if (i > 0)
                std::cout << ",";
            std::cout << std::dec << static_cast<int32_t>(obj->data[i * 4]);
        }
        std::cout << "]";
        break;
    case Vector::BLF::SystemVariable::Type::LongLong:
        std::cout << " type=LongLong";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=" << std::dec << static_cast<uint64_t>(*obj->data.data());
        break;
    case Vector::BLF::SystemVariable::Type::ByteArray:
        std::cout << " type=ByteArray";
        std::cout << " representation=" << std::dec << obj->representation;
        std::cout << " nameLength=" << std::dec << obj->nameLength;
        std::cout << " dataLength=" << std::dec << obj->dataLength;
        std::cout << " name=" << obj->name;
        std::cout << " data=[";
        for (uint32_t i = 0; i < min(obj->data.size(), obj->dataLength); ++i) {
            if (i > 0)
                std::cout << ",";
            std::cout << std::dec << static_cast<uint16_t>(obj->data[i]);
        }
        std::cout << "]";
        break;
    }
    std::cout << std::endl;
}

// CAN_ERROR_EXT = 73
void show(Vector::BLF::CanErrorFrameExt * obj) {
    std::cout << "CanErrorFrameExt:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " length=" << std::dec << obj->length;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " ecc=0x" << std::hex << static_cast<uint16_t>(obj->ecc);
    std::cout << " position=" << std::dec << static_cast<uint16_t>(obj->position);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " frameLengthInNs=" << std::dec << obj->frameLengthInNs;
    std::cout << " id=0x" << std::hex << obj->id;
    std::cout << " flagsExt=0x" << std::hex << obj->flagsExt;
    std::cout << " data=" << std::hex;
    printData(obj->data.data(), min(obj->data.size(), obj->dlc));
    std::cout << std::endl;
}

// CAN_DRIVER_ERROR_EXT = 74
void show(Vector::BLF::CanDriverErrorExt * obj) {
    std::cout << "CanErrorFrameExt:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " txErrors=" << std::dec << static_cast<uint16_t>(obj->txErrors);
    std::cout << " rxErrors=" << std::dec << static_cast<uint16_t>(obj->rxErrors);
    std::cout << " errorCode=" << std::dec << obj->errorCode;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " state=" << std::dec << static_cast<uint16_t>(obj->state);
    std::cout << std::endl;
}

// LIN_LONG_DOM_SIG2 = 75
void show(Vector::BLF::LinLongDomSignalEvent2 * obj) {
    std::cout << "LinLongDomSignalEvent2:";
    std::cout << " type=" << std::dec << static_cast<uint16_t>(obj->type);
    std::cout << " length=" << std::dec << obj->length;
    std::cout << std::endl;
}

// MOST_150_MESSAGE = 76
void show(Vector::BLF::Most150Message * obj) {
    std::cout << "Most150Message:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " pAck=" << std::dec << static_cast<uint16_t>(obj->pAck);
    std::cout << " cAck=" << std::dec << static_cast<uint16_t>(obj->cAck);
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " pIndex=" << std::dec << static_cast<uint16_t>(obj->pIndex);
    std::cout << " msgLen=" << std::dec << obj->msgLen;
    std::cout << " msg=";
    printData(obj->msg.data(), min(obj->msg.size(), obj->msgLen));
    std::cout << std::endl;
}

// MOST_150_PKT = 77
void show(Vector::BLF::Most150Pkt * obj) {
    std::cout << "Most150Pkt:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " pAck=" << std::dec << static_cast<uint16_t>(obj->pAck);
    std::cout << " cAck=" << std::dec << static_cast<uint16_t>(obj->cAck);
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " pIndex=" << std::dec << static_cast<uint16_t>(obj->pIndex);
    std::cout << " pktDataLength=" << std::dec << obj->pktDataLength;
    std::cout << " pktData=";
    printData(obj->pktData.data(), min(obj->pktData.size(), obj->pktDataLength));
    std::cout << std::endl;
}

// MOST_ETHERNET_PKT = 78
void show(Vector::BLF::MostEthernetPkt * obj) {
    std::cout << "MostEthernetPkt:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceMacAdr=" << std::dec << obj->sourceMacAdr;
    std::cout << " destMacAdr=" << std::dec << obj->destMacAdr;
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " pAck=" << std::dec << static_cast<uint16_t>(obj->pAck);
    std::cout << " cAck=" << std::dec << static_cast<uint16_t>(obj->cAck);
    std::cout << " pktDataLength=" << std::dec << obj->pktDataLength;
    std::cout << " pktData=";
    printData(obj->pktData.data(), min(obj->pktData.size(), obj->pktDataLength));
    std::cout << std::endl;
}

// MOST_150_MESSAGE_FRAGMENT = 79
void show(Vector::BLF::Most150MessageFragment * obj) {
    std::cout << "Most150MessageFragment:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " validMask=0x" << std::hex << obj->validMask;
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " pAck=" << std::dec << static_cast<uint16_t>(obj->pAck);
    std::cout << " cAck=" << std::dec << static_cast<uint16_t>(obj->cAck);
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " pIndex=" << std::dec << static_cast<uint16_t>(obj->pIndex);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dataLen=" << std::dec << obj->dataLen;
    std::cout << " dataLenAnnounced=" << std::dec << obj->dataLenAnnounced;
    std::cout << " firstDataLen=" << std::dec << obj->firstDataLen;
    std::cout << " firstData=";
    printData(obj->firstData.data(), min(obj->firstData.size(), obj->firstDataLen));
    std::cout << std::endl;
}

// MOST_150_PKT_FRAGMENT = 80
void show(Vector::BLF::Most150PktFragment * obj) {
    std::cout << "Most150PktFragment:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " validMask=0x" << std::hex << obj->validMask;
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " pAck=" << std::dec << static_cast<uint16_t>(obj->pAck);
    std::cout << " cAck=" << std::dec << static_cast<uint16_t>(obj->cAck);
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " pIndex=" << std::dec << static_cast<uint16_t>(obj->pIndex);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dataLen=" << std::dec << obj->dataLen;
    std::cout << " dataLenAnnounced=" << std::dec << obj->dataLenAnnounced;
    std::cout << " firstDataLen=" << std::dec << obj->firstDataLen;
    std::cout << " firstData=";
    printData(obj->firstData.data(), min(obj->firstData.size(), obj->firstDataLen));
    std::cout << std::endl;
}

// MOST_ETHERNET_PKT_FRAGMENT = 81
void show(Vector::BLF::MostEthernetPktFragment * obj) {
    std::cout << "MostEthernetPktFragment:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " validMask=0x" << std::hex << obj->validMask;
    std::cout << " sourceMacAdr=" << std::dec << obj->sourceMacAdr;
    std::cout << " destMacAdr=" << std::dec << obj->destMacAdr;
    std::cout << " pAck=" << std::dec << static_cast<uint16_t>(obj->pAck);
    std::cout << " cAck=" << std::dec << static_cast<uint16_t>(obj->cAck);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " dataLen=" << std::dec << obj->dataLen;
    std::cout << " dataLenAnnounced=" << std::dec << obj->dataLenAnnounced;
    std::cout << " firstDataLen=" << std::dec << obj->firstDataLen;
    std::cout << " firstData=";
    printData(obj->firstData.data(), min(obj->firstData.size(), obj->firstDataLen));
    std::cout << std::endl;
}

// MOST_SYSTEM_EVENT = 82
void show(Vector::BLF::MostSystemEvent * obj) {
    std::cout << "MostSystemEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << obj->id;
    std::cout << " value=" << std::dec << obj->value;
    std::cout << " valueOld=" << std::dec << obj->valueOld;
    std::cout << std::endl;
}

// MOST_150_ALLOCTAB = 83
void show(Vector::BLF::Most150AllocTab * obj) {
    std::cout << "Most150AllocTab:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " eventModeFlags=0x" << std::hex << obj->eventModeFlags;
    std::cout << " freeBytes=" << std::dec << obj->freeBytes;
    std::cout << " length=" << std::dec << obj->length;
    std::cout << " tableData=";
    printData(obj->tableData.data(), min(obj->tableData.size(), obj->length * 4));
    std::cout << std::endl;
}

// MOST_50_MESSAGE = 84
void show(Vector::BLF::Most50Message * obj) {
    std::cout << "Most50Message:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " msgLen=" << std::dec << obj->msgLen;
    std::cout << " msg=";
    printData(obj->msg.data(), min(obj->msg.size(), obj->msgLen));
    std::cout << std::endl;
}

// MOST_50_PKT = 85
void show(Vector::BLF::Most50Pkt * obj) {
    std::cout << "Most50Pkt:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " sourceAdr=" << std::dec << obj->sourceAdr;
    std::cout << " destAdr=" << std::dec << obj->destAdr;
    std::cout << " transferType=" << std::dec << static_cast<uint16_t>(obj->transferType);
    std::cout << " state=0x" << std::hex << static_cast<uint16_t>(obj->state);
    std::cout << " ackNack=0x" << std::hex << static_cast<uint16_t>(obj->ackNack);
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " priority=" << std::dec << static_cast<uint16_t>(obj->priority);
    std::cout << " pktDataLength=" << std::dec << obj->pktDataLength;
    std::cout << " pktData=";
    printData(obj->pktData.data(), min(obj->pktData.size(), obj->pktDataLength));
    std::cout << std::endl;
}

// CAN_MESSAGE2 = 86
void show(Vector::BLF::CanMessage2 * obj) {
    std::cout << "CanMessage2:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << static_cast<uint16_t>(obj->flags);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " id=0x" << std::hex << obj->id;
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " bitCount=" << std::dec << static_cast<uint16_t>(obj->bitCount);
    std::cout << std::endl;
}

// LIN_UNEXPECTED_WAKEUP = 87
void show(Vector::BLF::LinUnexpectedWakeup * obj) {
    std::cout << "LinUnexpectedWakeup:";
    std::cout << " width=" << std::dec << obj->width;
    std::cout << " signal=" << std::dec << static_cast<uint16_t>(obj->signal);
    std::cout << std::endl;
}

// LIN_SHORT_OR_SLOW_RESPONSE = 88
void show(Vector::BLF::LinShortOrSlowResponse * obj) {
    std::cout << "LinShortOrSlowResponse:";
    std::cout << " numberOfRespBytes=" << std::dec << obj->numberOfRespBytes;
    std::cout << " respBytes=";
    printData(obj->respBytes.data(), min(obj->respBytes.size(), obj->numberOfRespBytes));
    std::cout << " slowResponse=" << std::dec << static_cast<uint16_t>(obj->slowResponse);
    std::cout << " interruptedByBreak=" << std::dec << static_cast<uint16_t>(obj->interruptedByBreak);
    std::cout << std::endl;
}

// LIN_DISTURBANCE_EVENT = 89
void show(Vector::BLF::LinDisturbanceEvent * obj) {
    std::cout << "LinDisturbanceEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " id=" << std::dec << static_cast<uint16_t>(obj->id);
    std::cout << " disturbingFrameId=" << std::dec << static_cast<uint16_t>(obj->disturbingFrameId);
    std::cout << " disturbanceType=" << std::dec << obj->disturbanceType;
    std::cout << " byteIndex=" << std::dec << obj->byteIndex;
    std::cout << " bitIndex=" << std::dec << obj->bitIndex;
    std::cout << " bitOffsetInSixteenthBits=" << std::dec << obj->bitOffsetInSixteenthBits;
    std::cout << " disturbanceLengthInSixteenthBits=" << std::dec << obj->disturbanceLengthInSixteenthBits;
    std::cout << std::endl;
}

// SERIAL_EVENT = 90
void show(Vector::BLF::SerialEvent * obj) {
    std::cout << "SerialEvent:";
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " port=" << std::dec << obj->port;
    std::cout << " baudrate=" << std::dec << obj->baudrate;
    if (obj->flags & Vector::BLF::SerialEvent::Flags::SingleByte) {
        /* SingleByteSerialEvent */
        std::cout << " byte=" << std::dec << static_cast<uint16_t>(obj->singleByte.byte);
    } else {
        if (obj->flags & Vector::BLF::SerialEvent::Flags::CompactByte) {
            /* CompactSerialEvent */
            std::cout << " compactLength=" << std::dec << static_cast<uint16_t>(obj->compact.compactLength);
            std::cout << " compactData=";
            printData(obj->compact.compactData.data(), min(obj->compact.compactData.size(), obj->compact.compactLength));
        } else {
            /* GeneralSerialEvent */
            std::cout << " dataLength=" << std::dec << obj->general.dataLength;
            std::cout << " timeStampsLength=" << std::dec << obj->general.timeStampsLength;
            std::cout << " data=";
            printData(obj->general.data.data(), min(obj->general.data.size(), obj->general.dataLength));
            std::cout << " timeStamps=";
            for (uint32_t i = 0; i < min(obj->general.timeStamps.size(), obj->general.timeStampsLength / 8); ++i) {
                if (i > 0)
                    std::cout << ",";
                std::cout << std::dec << obj->general.timeStamps[i];
            }
        }
    }
    std::cout << std::endl;
}

// OVERRUN_ERROR = 91
void show(Vector::BLF::DriverOverrun * obj) {
    std::cout << "DriverOverrun:";
    std::cout << " busType=" << std::dec << obj->busType;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << std::endl;
}

// EVENT_COMMENT = 92
void show(Vector::BLF::EventComment * obj) {
    std::cout << "EventComment:";
    std::cout << " commentedEventType=" << std::dec << obj->commentedEventType;
    std::cout << " textLength=" << std::dec << obj->textLength;
    std::cout << " text=" << obj->text;
    std::cout << std::endl;
}

// WLAN_FRAME = 93
void show(Vector::BLF::WlanFrame * obj) {
    std::cout << "WlanFrame:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " radioChannel=" << std::dec << static_cast<uint16_t>(obj->radioChannel);
    std::cout << " signalStrength=" << std::dec << obj->signalStrength;
    std::cout << " signalQuality=" << std::dec << obj->signalQuality;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " frameData=";
    printData(obj->frameData.data(), min(obj->frameData.size(), obj->frameLength));
    std::cout << std::endl;
}

// WLAN_STATISTIC = 94
void show(Vector::BLF::WlanStatistic * obj) {
    std::cout << "WlanStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " rxPacketCount=" << std::dec << obj->rxPacketCount;
    std::cout << " rxByteCount=" << std::dec << obj->rxByteCount;
    std::cout << " txPacketCount=" << std::dec << obj->txPacketCount;
    std::cout << " txByteCount=" << std::dec << obj->txByteCount;
    std::cout << " collisionCount=" << std::dec << obj->collisionCount;
    std::cout << " errorCount=" << std::dec << obj->errorCount;
    std::cout << std::endl;
}

// MOST_ECL = 95
void show(Vector::BLF::MostEcl * obj) {
    std::cout << "MostEcl:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " mode=" << std::dec << obj->mode;
    std::cout << " eclState=" << std::dec << obj->eclState;
    std::cout << std::endl;
}

// GLOBAL_MARKER = 96
void show(Vector::BLF::GlobalMarker * obj) {
    std::cout << "GlobalMarker:";
    std::cout << " commentedEventType=" << std::dec << obj->commentedEventType;
    std::cout << " foregroundColor=0x" << std::hex << obj->foregroundColor;
    std::cout << " backgroundColor=0x" << std::hex << obj->backgroundColor;
    std::cout << " isRelocatable=" << std::dec << obj->isRelocatable;
    std::cout << " groupNameLength=" << std::dec << obj->groupNameLength;
    std::cout << " markerNameLength=" << std::dec << obj->markerNameLength;
    std::cout << " descriptionLength=" << std::dec << obj->descriptionLength;
    std::cout << " groupName=" << obj->groupName;
    std::cout << " markerName=" << obj->markerName;
    std::cout << " description=" << obj->description;
    std::cout << std::endl;
}

// AFDX_FRAME = 97
void show(Vector::BLF::AfdxFrame * obj) {
    std::cout << "AfdxFrame:";
    std::cout << " sourceAddress=";
    printData(obj->sourceAddress.data(), obj->sourceAddress.size());
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " destinationAddress=";
    printData(obj->destinationAddress.data(), obj->destinationAddress.size());
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " type=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->type;
    std::cout << " tpid=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->tpid;
    std::cout << " tci=0x" << std::setfill('0') << std::setw(4) << std::hex << obj->tci;
    std::cout << " ethChan=" << std::dec << static_cast<uint16_t>(obj->ethChannel);
    std::cout << " flags=0x" << std::hex << obj->afdxFlags;
    std::cout << " bagUsec=" << std::dec << obj->bagUsec;
    std::cout << " len=0x" << std::hex << obj->payLoadLength;
    printData(obj->payLoad.data(), min(obj->payLoad.size(), obj->payLoadLength));
    std::cout << std::endl;
}

// AFDX_STATISTIC = 98
void show(Vector::BLF::AfdxStatistic * obj) {
    std::cout << "AfdxStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << static_cast<uint16_t>(obj->flags);
    std::cout << " rxPacketCount=" << std::dec << obj->rxPacketCount;
    std::cout << " rxByteCount=" << std::dec << obj->rxByteCount;
    std::cout << " txPacketCount=" << std::dec << obj->txPacketCount;
    std::cout << " txByteCount=" << std::dec << obj->txByteCount;
    std::cout << " collisionCount=" << std::dec << obj->collisionCount;
    std::cout << " errorCount=" << std::dec << obj->errorCount;
    std::cout << " statDroppedRedundantPacketCount=" << std::dec << obj->statDroppedRedundantPacketCount;
    std::cout << " statRedundantErrorPacketCount=" << std::dec << obj->statRedundantErrorPacketCount;
    std::cout << " statIntegrityErrorPacketCount=" << std::dec << obj->statIntegrityErrorPacketCount;
    std::cout << " statAvrgPeriodMsec=" << std::dec << obj->statAvrgPeriodMsec;
    std::cout << " statAvrgJitterMysec=" << std::dec << obj->statAvrgJitterMysec;
    std::cout << " vlid=" << std::dec << obj->vlid;
    std::cout << " statDuration=" << std::dec << obj->statDuration;
    std::cout << std::endl;
}

// KLINE_STATUSEVENT = 99
void show(Vector::BLF::KLineStatusEvent * obj) {
    std::cout << "KLineStatusEvent:";
    std::cout << " type=0x" << std::hex << obj->type;
    std::cout << " dataLen=" << std::dec << obj->dataLen;
    std::cout << " port=" << std::dec << obj->port;
    std::cout << " data=[";
    for (uint32_t i = 0; i < min(obj->data.size(), obj->dataLen / 8); ++i) {
        if (i > 0)
            std::cout << " ";
        std::cout << std::hex << obj->data[i];
    }
    std::cout << "]";
    std::cout << std::endl;
}

// CAN_FD_MESSAGE = 100
void show(Vector::BLF::CanFdMessage * obj) {
    std::cout << "CanFdMessage:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << static_cast<uint16_t>(obj->flags);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " id=" << std::dec << obj->id;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " arbBitCount=" << std::dec << static_cast<uint16_t>(obj->arbBitCount);
    std::cout << " canFdFlags=0x" << std::hex << static_cast<uint16_t>(obj->canFdFlags);
    std::cout << " validDataBytes=" << std::dec << static_cast<uint16_t>(obj->validDataBytes);
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    std::cout << std::endl;
}

// CAN_FD_MESSAGE_64 = 101
void show(Vector::BLF::CanFdMessage64 * obj) {
    std::cout << "CanFdMessage64:";
    std::cout << " channel=" << std::dec << static_cast<uint16_t>(obj->channel);
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " validDataBytes=" << std::dec << static_cast<uint16_t>(obj->validDataBytes);
    std::cout << " txCount=" << std::dec << static_cast<uint16_t>(obj->txCount);
    std::cout << " id=" << std::dec << obj->id;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " btrCfgArb=" << std::dec << obj->btrCfgArb;
    std::cout << " btrCfgData=" << std::dec << obj->btrCfgData;
    std::cout << " timeOffsetBrsNs=" << std::dec << obj->timeOffsetBrsNs;
    std::cout << " timeOffsetCrcDelNs=" << std::dec << obj->timeOffsetCrcDelNs;
    std::cout << " bitCount=" << std::dec << obj->bitCount;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " extDataOffset=" << std::dec << static_cast<uint16_t>(obj->extDataOffset);
    std::cout << " crc=0x" << std::dec << obj->crc;
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    if (obj->hasExtData()) {
        std::cout << " btrExtArb=0x" << std::hex << obj->btrExtArb;
        std::cout << " btrExtData=0x" << std::hex << obj->btrExtData;
    }
    std::cout << std::endl;
}

// ETHERNET_RX_ERROR = 102
void show(Vector::BLF::EthernetRxError * obj) {
    std::cout << "EthernetRxError:";
    std::cout << " structLength=" << std::dec << obj->structLength;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " hardwareChannel=" << std::dec << obj->hardwareChannel;
    std::cout << " fcs=" << std::dec << obj->fcs;
    std::cout << " frameDataLength=" << std::dec << obj->frameDataLength;
    std::cout << " error=" << std::dec << obj->error;
    std::cout << " frameData=";
    printData(obj->frameData.data(), min(obj->frameData.size(), obj->frameDataLength));
    std::cout << std::endl;
}

// ETHERNET_STATUS = 103
void show(Vector::BLF::EthernetStatus * obj) {
    std::cout << "EthernetStatus:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " linkStatus=" << std::dec << static_cast<uint16_t>(obj->linkStatus);
    std::cout << " ethernetPhy=" << std::dec << static_cast<uint16_t>(obj->ethernetPhy);
    std::cout << " duplex=" << std::dec << static_cast<uint16_t>(obj->duplex);
    std::cout << " mdi=" << std::dec << static_cast<uint16_t>(obj->mdi);
    std::cout << " connector=" << std::dec << static_cast<uint16_t>(obj->connector);
    std::cout << " clockMode=" << std::dec << static_cast<uint16_t>(obj->clockMode);
    std::cout << " pairs=" << std::dec << static_cast<uint16_t>(obj->pairs);
    std::cout << " hardwareChannel=" << std::dec << static_cast<uint16_t>(obj->hardwareChannel);
    std::cout << " bitrate=" << std::dec << obj->bitrate;
    std::cout << std::endl;
}

// CAN_FD_ERROR_64 = 104
void show(Vector::BLF::CanFdErrorFrame64 * obj) {
    std::cout << "CanFdErrorFrame64:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dlc=" << std::dec << static_cast<uint16_t>(obj->dlc);
    std::cout << " validDataBytes=" << std::dec << static_cast<uint16_t>(obj->validDataBytes);
    std::cout << " ecc=" << std::dec << static_cast<uint16_t>(obj->ecc);
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " errorCodeExt=0x" << std::hex << obj->errorCodeExt;
    std::cout << " extFlags=0x" << std::hex << obj->extFlags;
    std::cout << " extDataOffset=" << std::dec << static_cast<uint16_t>(obj->extDataOffset);
    std::cout << " id=" << std::dec << obj->id;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " btrCfgArb=" << std::dec << obj->btrCfgArb;
    std::cout << " btrCfgData=" << std::dec << obj->btrCfgData;
    std::cout << " timeOffsetBrsNs=" << std::dec << obj->timeOffsetBrsNs;
    std::cout << " timeOffsetCrcDelNs=" << std::dec << obj->timeOffsetCrcDelNs;
    std::cout << " crc=0x" << std::hex << obj->crc;
    std::cout << " errorPosition=" << std::dec << obj->errorPosition;
    std::cout << " data=";
    printData(obj->data.data(), min(obj->data.size(), obj->frameLength));
    if (obj->hasExtData()) {
        std::cout << " btrExtArb=0x" << std::hex << obj->btrExtArb;
        std::cout << " btrExtData=0x" << std::hex << obj->btrExtData;
    }
    std::cout << std::endl;
}

// LIN_SHORT_OR_SLOW_RESPONSE2 = 105
void show(Vector::BLF::LinShortOrSlowResponse2 * obj) {
    std::cout << "LinShortOrSlowResponse2:";
    std::cout << " numberOfRespBytes=" << std::dec << obj->numberOfRespBytes;
    std::cout << " respBytes=";
    printData(obj->respBytes.data(), min(obj->respBytes.size(), obj->numberOfRespBytes));
    std::cout << " slowResponse=" << std::dec << static_cast<uint16_t>(obj->slowResponse);
    std::cout << " interruptedByBreak=" << std::dec << static_cast<uint16_t>(obj->interruptedByBreak);
    std::cout << " exactHeaderBaudrate=" << std::fixed << obj->exactHeaderBaudrate;
    std::cout << " earlyStopbitOffset=" << std::dec << obj->earlyStopbitOffset;
    std::cout << std::endl;
}

// AFDX_STATUS = 106
void show(Vector::BLF::AfdxStatus * obj) {
    std::cout << "AfdxStatus:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " statusA::flags=0x" << std::hex << obj->statusA.flags;
    std::cout << " statusA::linkStatus=" << std::dec << static_cast<uint16_t>(obj->statusA.linkStatus);
    std::cout << " statusA::ethernetPhy=" << std::dec << static_cast<uint16_t>(obj->statusA.ethernetPhy);
    std::cout << " statusA::duplex=" << std::dec << static_cast<uint16_t>(obj->statusA.duplex);
    std::cout << " statusA::mdi=" << std::dec << static_cast<uint16_t>(obj->statusA.mdi);
    std::cout << " statusA::connector=" << std::dec << static_cast<uint16_t>(obj->statusA.connector);
    std::cout << " statusA::clockMode=" << std::dec << static_cast<uint16_t>(obj->statusA.clockMode);
    std::cout << " statusA::pairs=" << std::dec << static_cast<uint16_t>(obj->statusA.pairs);
    std::cout << " statusA::bitrate=" << std::dec << obj->statusA.bitrate;
    std::cout << " statusB::flags=0x" << std::hex << obj->statusB.flags;
    std::cout << " statusB::linkStatus=" << std::dec << static_cast<uint16_t>(obj->statusB.linkStatus);
    std::cout << " statusB::ethernetPhy=" << std::dec << static_cast<uint16_t>(obj->statusB.ethernetPhy);
    std::cout << " statusB::duplex=" << std::dec << static_cast<uint16_t>(obj->statusB.duplex);
    std::cout << " statusB::mdi=" << std::dec << static_cast<uint16_t>(obj->statusB.mdi);
    std::cout << " statusB::connector=" << std::dec << static_cast<uint16_t>(obj->statusB.connector);
    std::cout << " statusB::clockMode=" << std::dec << static_cast<uint16_t>(obj->statusB.clockMode);
    std::cout << " statusB::pairs=" << std::dec << static_cast<uint16_t>(obj->statusB.pairs);
    std::cout << " statusB::bitrate=" << std::dec << obj->statusB.bitrate;
    std::cout << std::endl;
}

// AFDX_BUS_STATISTIC = 107
void show(Vector::BLF::AfdxBusStatistic * obj) {
    std::cout << "AfdxBusStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " flags=" << std::dec << obj->flags;
    std::cout << " statDuration=" << std::dec << obj->statDuration;
    std::cout << " statRxPacketCountHW=" << std::dec << obj->statRxPacketCountHW;
    std::cout << " statTxPacketCountHW=" << std::dec << obj->statTxPacketCountHW;
    std::cout << " statRxErrorCountHW=" << std::dec << obj->statRxErrorCountHW;
    std::cout << " statTxErrorCountHW=" << std::dec << obj->statTxErrorCountHW;
    std::cout << " statRxBytesHW=" << std::dec << obj->statRxBytesHW;
    std::cout << " statTxBytesHW=" << std::dec << obj->statTxBytesHW;
    std::cout << " statRxPacketCount=" << std::dec << obj->statRxPacketCount;
    std::cout << " statTxPacketCount=" << std::dec << obj->statTxPacketCount;
    std::cout << " statDroppedPacketCount=" << std::dec << obj->statDroppedPacketCount;
    std::cout << " statInvalidPacketCount=" << std::dec << obj->statInvalidPacketCount;
    std::cout << " statLostPacketCount=" << std::dec << obj->statLostPacketCount;
    std::cout << " line=" << std::dec << static_cast<uint16_t>(obj->line);
    std::cout << " linkStatus=" << std::dec << static_cast<uint16_t>(obj->linkStatus);
    std::cout << " linkSpeed=" << std::dec << obj->linkSpeed;
    std::cout << " linkLost=" << std::dec << obj->linkLost;
    std::cout << std::endl;
}

// Reserved108 = 108

// AFDX_ERROR_EVENT = 109
void show(Vector::BLF::AfdxErrorEvent * obj) {
    std::cout << "AfdxErrorEvent:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " errorLevel=" << std::dec << obj->errorLevel;
    std::cout << " sourceIdentifier=" << std::dec << obj->sourceIdentifier;
    std::cout << " errorText=";
    printString(obj->errorText.data(), obj->errorText.size());
    std::cout << " errorAttributes=";
    printString(obj->errorAttributes.data(), obj->errorAttributes.size());
    std::cout << std::endl;
}

// A429_ERROR = 110
void show(Vector::BLF::A429Error * obj) {
    std::cout << "A429Error:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " errorType=" << std::dec << obj->errorType;
    std::cout << " sourceIdentifier=" << std::dec << obj->sourceIdentifier;
    std::cout << " errReason=" << std::dec << obj->errReason;
    std::cout << " errorText=";
    printString(obj->errorText.data(), obj->errorText.size());
    std::cout << " errorAttributes=";
    printString(obj->errorAttributes.data(), obj->errorAttributes.size());
    std::cout << std::endl;
}

// A429_STATUS = 111
void show(Vector::BLF::A429Status * obj) {
    std::cout << "A429Status:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " parity=" << std::dec << obj->parity;
    std::cout << " minGap=" << std::dec << obj->minGap;
    std::cout << " bitrate=" << std::dec << obj->bitrate;
    std::cout << " minBitrate=" << std::dec << obj->minBitrate;
    std::cout << " maxBitrate=" << std::dec << obj->maxBitrate;
    std::cout << std::endl;
}

// A429_BUS_STATISTIC = 112
void show(Vector::BLF::A429BusStatistic * obj) {
    std::cout << "A429BusStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " busload=" << std::dec << obj->busload;
    std::cout << " dataTotal=" << std::dec << obj->dataTotal;
    std::cout << " errorTotal=" << std::dec << obj->errorTotal;
    std::cout << " bitrate=" << std::dec << obj->bitrate;
    std::cout << " parityErrors=" << std::dec << obj->parityErrors;
    std::cout << " bitrateErrors=" << std::dec << obj->bitrateErrors;
    std::cout << " gapErrors=" << std::dec << obj->gapErrors;
    std::cout << " lineErrors=" << std::dec << obj->lineErrors;
    std::cout << " formatErrors=" << std::dec << obj->formatErrors;
    std::cout << " dutyFactorErrors=" << std::dec << obj->dutyFactorErrors;
    std::cout << " wordLenErrors=" << std::dec << obj->wordLenErrors;
    std::cout << " codingErrors=" << std::dec << obj->codingErrors;
    std::cout << " idleErrors=" << std::dec << obj->idleErrors;
    std::cout << " levelErrors=" << std::dec << obj->levelErrors;
    std::cout << " labelCount=";
    for (uint32_t i = 0; i < obj->labelCount.size(); ++i) {
        if (i > 0)
            std::cout << ",";
        std::cout << std::dec << obj->labelCount[i];
    }
    std::cout << std::endl;
}

// A429_MESSAGE = 113
void show(Vector::BLF::A429Message * obj) {
    std::cout << "A429Message:";
    std::cout << " a429Data=";
    printData(obj->a429Data.data(), obj->a429Data.size());
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " dir=" << std::dec << static_cast<uint16_t>(obj->dir);
    std::cout << " bitrate=" << std::dec << obj->bitrate;
    std::cout << " errReason=" << std::dec << obj->errReason;
    std::cout << " errPosition=" << std::dec << obj->errPosition;
    std::cout << " frameGap=" << std::dec << obj->frameGap;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " msgCtrl=" << std::dec << obj->msgCtrl;
    std::cout << " cycleTime=" << std::dec << obj->cycleTime;
    std::cout << " error=" << std::dec << obj->error;
    std::cout << " bitLenOfLastBit=" << std::dec << obj->bitLenOfLastBit;
    std::cout << std::endl;
}

// ETHERNET_STATISTIC = 114
void show(Vector::BLF::EthernetStatistic * obj) {
    std::cout << "EthernetStatistic:";
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " rcvOk_HW=" << std::dec << obj->rcvOk_HW;
    std::cout << " xmitOk_HW=" << std::dec << obj->xmitOk_HW;
    std::cout << " rcvError_HW=" << std::dec << obj->rcvError_HW;
    std::cout << " xmitError_HW=" << std::dec << obj->xmitError_HW;
    std::cout << " rcvBytes_HW=" << std::dec << obj->rcvBytes_HW;
    std::cout << " xmitBytes_HW=" << std::dec << obj->xmitBytes_HW;
    std::cout << " rcvNoBuffer_HW=" << std::dec << obj->rcvNoBuffer_HW;
    std::cout << " sqi=" << std::dec << obj->sqi;
    std::cout << " hardwareChannel=" << std::dec << obj->hardwareChannel;
    std::cout << std::endl;
}

// Unknown115 = 115
void show(Vector::BLF::RestorePointContainer * obj) {
    std::cout << "RestorePointContainer:";
    std::cout << " dataLength=" << std::dec << obj->dataLength;
    std::cout << " data=";
    printData(obj->data.data(), obj->data.size());
    std::cout << std::endl;
}

// Reserved116 = 116
// Reserved117 = 117

// TEST_STRUCTURE = 118
void show(Vector::BLF::TestStructure * obj) {
    std::cout << "TestStructure:";
    std::cout << " type=" << std::dec << obj->type;
    std::cout << " uniqueNo=" << std::dec << obj->uniqueNo;
    std::cout << " action=" << std::dec << obj->action;
    std::cout << " result=" << std::dec << obj->result;
    std::cout << " executingObjectNameLength=" << std::dec << obj->executingObjectNameLength;
    std::cout << " nameLength=" << std::dec << obj->nameLength;
    std::cout << " textLength=" << std::dec << obj->textLength;
#if _MSC_VER >= 1900
    /* @todo find a solution to compile under VS >=2015 */
#else
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
    std::cout << " executingObjectName=" << convert.to_bytes(obj->executingObjectName);
    std::cout << " name=" << convert.to_bytes(obj->name);
    std::cout << " text=" << convert.to_bytes(obj->text);
#endif
    std::cout << std::endl;
}

// DIAG_REQUEST_INTERPRETATION = 119
void show(Vector::BLF::DiagRequestInterpretation * obj) {
    std::cout << "DiagRequestInterpretation:";
    std::cout << " diagDescriptionHandle=" << std::dec << obj->diagDescriptionHandle;
    std::cout << " diagVariantHandle=" << std::dec << obj->diagVariantHandle;
    std::cout << " diagServiceHandle=" << std::dec << obj->diagServiceHandle;
    std::cout << " ecuQualifierLength=" << std::dec << obj->ecuQualifierLength;
    std::cout << " variantQualifierLength=" << std::dec << obj->variantQualifierLength;
    std::cout << " serviceQualifierLength=" << std::dec << obj->serviceQualifierLength;
    std::cout << " ecuQualifier=" << obj->ecuQualifier;
    std::cout << " variantQualifier=" << obj->variantQualifier;
    std::cout << " serviceQualifier=" << obj->serviceQualifier;
    std::cout << std::endl;
}

// ETHERNET_FRAME_EX = 120
void show(Vector::BLF::EthernetFrameEx * obj) {
    std::cout << "EthernetFrameEx:";
    std::cout << " structLength=" << std::dec << obj->structLength;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " hardwareChannel=" << std::dec << obj->hardwareChannel;
    std::cout << " frameDuration=" << std::dec << obj->frameDuration;
    std::cout << " frameChecksum=0x" << std::hex << obj->frameChecksum;
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " frameHandle=" << std::dec << obj->frameHandle;
    std::cout << " frameData=";
    printData(obj->frameData.data(), min(obj->frameData.size(), obj->frameLength));
    std::cout << std::endl;
}

// ETHERNET_FRAME_FORWARDED = 121
void show(Vector::BLF::EthernetFrameForwarded * obj) {
    std::cout << "EthernetFrameForwarded:";
    std::cout << " structLength=" << std::dec << obj->structLength;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " hardwareChannel=" << std::dec << obj->hardwareChannel;
    std::cout << " frameDuration=" << std::dec << obj->frameDuration;
    std::cout << " frameChecksum=0x" << std::hex << obj->frameChecksum;
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " frameHandle=" << std::dec << obj->frameHandle;
    std::cout << " frameData=";
    printData(obj->frameData.data(), min(obj->frameData.size(), obj->frameLength));
    std::cout << std::endl;
}

// ETHERNET_ERROR_EX = 122
void show(Vector::BLF::EthernetErrorEx * obj) {
    std::cout << "EthernetErrorEx:";
    std::cout << " structLength=" << std::dec << obj->structLength;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " hardwareChannel=" << std::dec << obj->hardwareChannel;
    std::cout << " frameDuration=" << std::dec << obj->frameDuration;
    std::cout << " frameChecksum=0x" << std::hex << obj->frameChecksum;
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " frameHandle=" << std::dec << obj->frameHandle;
    std::cout << " error=" << std::dec << obj->error;
    std::cout << " frameData=";
    printData(obj->frameData.data(), min(obj->frameData.size(), obj->frameLength));
    std::cout << std::endl;
}

// ETHERNET_ERROR_FORWARDED = 123
void show(Vector::BLF::EthernetErrorForwarded * obj) {
    std::cout << "EthernetErrorForwarded:";
    std::cout << " structLength=" << std::dec << obj->structLength;
    std::cout << " flags=0x" << std::hex << obj->flags;
    std::cout << " channel=" << std::dec << obj->channel;
    std::cout << " hardwareChannel=" << std::dec << obj->hardwareChannel;
    std::cout << " frameDuration=" << std::dec << obj->frameDuration;
    std::cout << " frameChecksum=0x" << std::hex << obj->frameChecksum;
    std::cout << " dir=" << std::dec << obj->dir;
    std::cout << " frameLength=" << std::dec << obj->frameLength;
    std::cout << " frameHandle=" << std::dec << obj->frameHandle;
    std::cout << " error=" << std::dec << obj->error;
    std::cout << " frameData=";
    printData(obj->frameData.data(), min(obj->frameData.size(), obj->frameLength));
    std::cout << std::endl;
}

// FUNCTION_BUS = 124
void show(Vector::BLF::FunctionBus * obj) {
    std::cout << "FunctionBus:";
    std::cout << " functionBusObjectType=" << std::dec << obj->functionBusObjectType;
    std::cout << " veType=" << std::dec << obj->veType;
    std::cout << " nameLength=" << std::dec << obj->nameLength;
    std::cout << " dataLength=" << std::dec << obj->dataLength;
    std::cout << " name=" << obj->name;
    std::cout << " data=";
    printData(obj->data.data(), min(obj->data.size(), obj->dataLength));
    std::cout << std::endl;
}

// DATA_LOST_BEGIN = 125
void show(Vector::BLF::DataLostBegin * obj) {
    std::cout << "DataLostBegin:";
    std::cout << " queueIdentifier=" << std::dec << obj->queueIdentifier;
    std::cout << std::endl;
}

// DATA_LOST_END = 126
void show(Vector::BLF::DataLostEnd * obj) {
    std::cout << "DataLostEnd:";
    std::cout << " queueIdentifier=" << std::dec << obj->queueIdentifier;
    std::cout << " firstObjectLostTimeStamp=" << std::dec << obj->firstObjectLostTimeStamp;
    std::cout << " numberOfLostEvents=" << std::dec << obj->numberOfLostEvents;
    std::cout << std::endl;
}

// WATER_MARK_EVENT = 127
void show(Vector::BLF::WaterMarkEvent * obj) {
    std::cout << "WaterMarkEvent:";
    std::cout << " queueState=" << std::dec << obj->queueState;
    std::cout << std::endl;
}

// TRIGGER_CONDITION = 128
void show(Vector::BLF::TriggerCondition * obj) {
    std::cout << "TriggerCondition:";
    std::cout << " state=" << std::dec << obj->state;
    std::cout << " triggerBlockNameLength=" << std::dec << obj->triggerBlockNameLength;
    std::cout << " triggerConditionLength=" << std::dec << obj->triggerConditionLength;
    std::cout << " triggerBlockName=" << obj->triggerBlockName;
    std::cout << " triggerCondition=" << obj->triggerCondition;
    std::cout << std::endl;
}

int main(int argc, char * argv[]) {
    if (argc != 2) {
        std::cout << "Parser <filename.blf>" << std::endl;
        return -1;
    }

    Vector::BLF::File file;
    file.open(argv[1]);
    if (!file.is_open()) {
        std::cout << "Unable to open file" << std::endl;
        return -1;
    }

    show(&file.fileStatistics);

    uint32_t objectsRead = 0;
    while (file.good()) {
        Vector::BLF::ObjectHeaderBase * ohb = nullptr;

        /* read and capture exceptions, e.g. unfinished files */
        try {
            ohb = file.read();
        } catch (std::runtime_error & e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }
        if (ohb == nullptr)
            break;

        /* show objects read */
        objectsRead++;
        std::cout << "(nr=" << std::dec << objectsRead << ") ";

        /* ObjectHeader */
        auto * oh = dynamic_cast<Vector::BLF::ObjectHeader *>(ohb);
        if (oh != nullptr) {
            std::cout << std::dec << oh->objectTimeStamp;
            switch (oh->objectFlags) {
            case Vector::BLF::ObjectHeader::ObjectFlags::TimeTenMics:
                std::cout << "0 ms: ";
                break;
            case Vector::BLF::ObjectHeader::ObjectFlags::TimeOneNans:
                std::cout << " ns: ";
                break;
            }
        }

        /* ObjectHeader2 */
        auto * oh2 = dynamic_cast<Vector::BLF::ObjectHeader2 *>(ohb);
        if (oh2 != nullptr) {
            std::cout << std::dec << oh2->objectTimeStamp;
            switch (oh2->objectFlags) {
            case Vector::BLF::ObjectHeader2::ObjectFlags::TimeTenMics:
                std::cout << "0 ms: ";
                break;
            case Vector::BLF::ObjectHeader2::ObjectFlags::TimeOneNans:
                std::cout << " ns: ";
                break;
            }
        }

        /* Object */
        switch (ohb->objectType) {
        case Vector::BLF::ObjectType::UNKNOWN:
            /* do nothing */
            break;

        case Vector::BLF::ObjectType::CAN_MESSAGE:
            show(reinterpret_cast<Vector::BLF::CanMessage *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_ERROR:
            show(reinterpret_cast<Vector::BLF::CanErrorFrame *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_OVERLOAD:
            show(reinterpret_cast<Vector::BLF::CanOverloadFrame *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_STATISTIC:
            show(reinterpret_cast<Vector::BLF::CanDriverStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::APP_TRIGGER:
            show(reinterpret_cast<Vector::BLF::AppTrigger *>(ohb));
            break;

        case Vector::BLF::ObjectType::ENV_INTEGER:
        case Vector::BLF::ObjectType::ENV_DOUBLE:
        case Vector::BLF::ObjectType::ENV_STRING:
        case Vector::BLF::ObjectType::ENV_DATA:
            show(reinterpret_cast<Vector::BLF::EnvironmentVariable *>(ohb));
            break;

        case Vector::BLF::ObjectType::LOG_CONTAINER:
            show(reinterpret_cast<Vector::BLF::LogContainer *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_MESSAGE:
            show(reinterpret_cast<Vector::BLF::LinMessage *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_CRC_ERROR:
            show(reinterpret_cast<Vector::BLF::LinCrcError *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_DLC_INFO:
            show(reinterpret_cast<Vector::BLF::LinDlcInfo *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_RCV_ERROR:
            show(reinterpret_cast<Vector::BLF::LinReceiveError *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SND_ERROR:
            show(reinterpret_cast<Vector::BLF::LinSendError *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SLV_TIMEOUT:
            show(reinterpret_cast<Vector::BLF::LinSlaveTimeout *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SCHED_MODCH:
            show(reinterpret_cast<Vector::BLF::LinSchedulerModeChange *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SYN_ERROR:
            show(reinterpret_cast<Vector::BLF::LinSyncError *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_BAUDRATE:
            show(reinterpret_cast<Vector::BLF::LinBaudrateEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SLEEP:
            show(reinterpret_cast<Vector::BLF::LinSleepModeEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_WAKEUP:
            show(reinterpret_cast<Vector::BLF::LinWakeupEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_SPY:
            show(reinterpret_cast<Vector::BLF::MostSpy *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_CTRL:
            show(reinterpret_cast<Vector::BLF::MostCtrl *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_LIGHTLOCK:
            show(reinterpret_cast<Vector::BLF::MostLightLock *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_STATISTIC:
            show(reinterpret_cast<Vector::BLF::MostStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::Reserved26:
        case Vector::BLF::ObjectType::Reserved27:
        case Vector::BLF::ObjectType::Reserved28:
            /* do nothing */
            break;

        case Vector::BLF::ObjectType::FLEXRAY_DATA:
            show(reinterpret_cast<Vector::BLF::FlexRayData *>(ohb));
            break;

        case Vector::BLF::ObjectType::FLEXRAY_SYNC:
            show(reinterpret_cast<Vector::BLF::FlexRaySync *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_DRIVER_ERROR:
            show(reinterpret_cast<Vector::BLF::CanDriverError *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_PKT:
            show(reinterpret_cast<Vector::BLF::MostPkt *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_PKT2:
            show(reinterpret_cast<Vector::BLF::MostPkt2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_HWMODE:
            show(reinterpret_cast<Vector::BLF::MostHwMode *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_REG:
            show(reinterpret_cast<Vector::BLF::MostReg *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_GENREG:
            show(reinterpret_cast<Vector::BLF::MostGenReg *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_NETSTATE:
            show(reinterpret_cast<Vector::BLF::MostNetState *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_DATALOST:
            show(reinterpret_cast<Vector::BLF::MostDataLost *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_TRIGGER:
            show(reinterpret_cast<Vector::BLF::MostTrigger *>(ohb));
            break;

        case Vector::BLF::ObjectType::FLEXRAY_CYCLE:
            show(reinterpret_cast<Vector::BLF::FlexRayV6StartCycleEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::FLEXRAY_MESSAGE:
            show(reinterpret_cast<Vector::BLF::FlexRayV6Message *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_CHECKSUM_INFO:
            show(reinterpret_cast<Vector::BLF::LinChecksumInfo *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SPIKE_EVENT:
            show(reinterpret_cast<Vector::BLF::LinSpikeEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_DRIVER_SYNC:
            show(reinterpret_cast<Vector::BLF::CanDriverHwSync *>(ohb));
            break;

        case Vector::BLF::ObjectType::FLEXRAY_STATUS:
            show(reinterpret_cast<Vector::BLF::FlexRayStatusEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::GPS_EVENT:
            show(reinterpret_cast<Vector::BLF::GpsEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::FR_ERROR:
            show(reinterpret_cast<Vector::BLF::FlexRayVFrError *>(ohb));
            break;

        case Vector::BLF::ObjectType::FR_STATUS:
            show(reinterpret_cast<Vector::BLF::FlexRayVFrStatus *>(ohb));
            break;

        case Vector::BLF::ObjectType::FR_STARTCYCLE:
            show(reinterpret_cast<Vector::BLF::FlexRayVFrStartCycle *>(ohb));
            break;

        case Vector::BLF::ObjectType::FR_RCVMESSAGE:
            show(reinterpret_cast<Vector::BLF::FlexRayVFrReceiveMsg *>(ohb));
            break;

        case Vector::BLF::ObjectType::REALTIMECLOCK:
            show(reinterpret_cast<Vector::BLF::RealtimeClock *>(ohb));
            break;

        case Vector::BLF::ObjectType::Reserved52:
        case Vector::BLF::ObjectType::Reserved53:
            /* do nothing */
            break;

        case Vector::BLF::ObjectType::LIN_STATISTIC:
            show(reinterpret_cast<Vector::BLF::LinStatisticEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::J1708_MESSAGE:
        case Vector::BLF::ObjectType::J1708_VIRTUAL_MSG:
            show(reinterpret_cast<Vector::BLF::J1708Message *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_MESSAGE2:
            show(reinterpret_cast<Vector::BLF::LinMessage2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SND_ERROR2:
            show(reinterpret_cast<Vector::BLF::LinSendError2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SYN_ERROR2:
            show(reinterpret_cast<Vector::BLF::LinSyncError2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_CRC_ERROR2:
            show(reinterpret_cast<Vector::BLF::LinCrcError2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_RCV_ERROR2:
            show(reinterpret_cast<Vector::BLF::LinReceiveError2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_WAKEUP2:
            show(reinterpret_cast<Vector::BLF::LinWakeupEvent2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SPIKE_EVENT2:
            show(reinterpret_cast<Vector::BLF::LinSpikeEvent2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_LONG_DOM_SIG:
            show(reinterpret_cast<Vector::BLF::LinLongDomSignalEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::APP_TEXT:
            show(reinterpret_cast<Vector::BLF::AppText *>(ohb));
            break;

        case Vector::BLF::ObjectType::FR_RCVMESSAGE_EX:
            show(reinterpret_cast<Vector::BLF::FlexRayVFrReceiveMsgEx *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_STATISTICEX:
            show(reinterpret_cast<Vector::BLF::MostStatisticEx *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_TXLIGHT:
            show(reinterpret_cast<Vector::BLF::MostTxLight *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_ALLOCTAB:
            show(reinterpret_cast<Vector::BLF::MostAllocTab *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_STRESS:
            show(reinterpret_cast<Vector::BLF::MostStress *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_FRAME:
            show(reinterpret_cast<Vector::BLF::EthernetFrame *>(ohb));
            break;

        case Vector::BLF::ObjectType::SYS_VARIABLE:
            show(reinterpret_cast<Vector::BLF::SystemVariable *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_ERROR_EXT:
            show(reinterpret_cast<Vector::BLF::CanErrorFrameExt *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_DRIVER_ERROR_EXT:
            show(reinterpret_cast<Vector::BLF::CanDriverErrorExt *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_LONG_DOM_SIG2:
            show(reinterpret_cast<Vector::BLF::LinLongDomSignalEvent2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_150_MESSAGE:
            show(reinterpret_cast<Vector::BLF::Most150Message *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_150_PKT:
            show(reinterpret_cast<Vector::BLF::Most150Pkt *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_ETHERNET_PKT:
            show(reinterpret_cast<Vector::BLF::MostEthernetPkt *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_150_MESSAGE_FRAGMENT:
            show(reinterpret_cast<Vector::BLF::Most150MessageFragment *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_150_PKT_FRAGMENT:
            show(reinterpret_cast<Vector::BLF::Most150PktFragment *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_ETHERNET_PKT_FRAGMENT:
            show(reinterpret_cast<Vector::BLF::MostEthernetPktFragment *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_SYSTEM_EVENT:
            show(reinterpret_cast<Vector::BLF::MostSystemEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_150_ALLOCTAB:
            show(reinterpret_cast<Vector::BLF::Most150AllocTab *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_50_MESSAGE:
            show(reinterpret_cast<Vector::BLF::Most50Message *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_50_PKT:
            show(reinterpret_cast<Vector::BLF::Most50Pkt *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_MESSAGE2:
            show(reinterpret_cast<Vector::BLF::CanMessage2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_UNEXPECTED_WAKEUP:
            show(reinterpret_cast<Vector::BLF::LinUnexpectedWakeup *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SHORT_OR_SLOW_RESPONSE:
            show(reinterpret_cast<Vector::BLF::LinShortOrSlowResponse *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_DISTURBANCE_EVENT:
            show(reinterpret_cast<Vector::BLF::LinDisturbanceEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::SERIAL_EVENT:
            show(reinterpret_cast<Vector::BLF::SerialEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::OVERRUN_ERROR:
            show(reinterpret_cast<Vector::BLF::DriverOverrun *>(ohb));
            break;

        case Vector::BLF::ObjectType::EVENT_COMMENT:
            show(reinterpret_cast<Vector::BLF::EventComment *>(ohb));
            break;

        case Vector::BLF::ObjectType::WLAN_FRAME:
            show(reinterpret_cast<Vector::BLF::WlanFrame *>(ohb));
            break;

        case Vector::BLF::ObjectType::WLAN_STATISTIC:
            show(reinterpret_cast<Vector::BLF::WlanStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::MOST_ECL:
            show(reinterpret_cast<Vector::BLF::MostEcl *>(ohb));
            break;

        case Vector::BLF::ObjectType::GLOBAL_MARKER:
            show(reinterpret_cast<Vector::BLF::GlobalMarker *>(ohb));
            break;

        case Vector::BLF::ObjectType::AFDX_FRAME:
            show(reinterpret_cast<Vector::BLF::AfdxFrame *>(ohb));
            break;

        case Vector::BLF::ObjectType::AFDX_STATISTIC:
            show(reinterpret_cast<Vector::BLF::AfdxStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::KLINE_STATUSEVENT:
            show(reinterpret_cast<Vector::BLF::KLineStatusEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_FD_MESSAGE:
            show(reinterpret_cast<Vector::BLF::CanFdMessage *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_FD_MESSAGE_64:
            show(reinterpret_cast<Vector::BLF::CanFdMessage64 *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_RX_ERROR:
            show(reinterpret_cast<Vector::BLF::EthernetRxError *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_STATUS:
            show(reinterpret_cast<Vector::BLF::EthernetStatus *>(ohb));
            break;

        case Vector::BLF::ObjectType::CAN_FD_ERROR_64:
            show(reinterpret_cast<Vector::BLF::CanFdErrorFrame64 *>(ohb));
            break;

        case Vector::BLF::ObjectType::LIN_SHORT_OR_SLOW_RESPONSE2:
            show(reinterpret_cast<Vector::BLF::LinShortOrSlowResponse2 *>(ohb));
            break;

        case Vector::BLF::ObjectType::AFDX_STATUS:
            show(reinterpret_cast<Vector::BLF::AfdxStatus *>(ohb));
            break;

        case Vector::BLF::ObjectType::AFDX_BUS_STATISTIC:
            show(reinterpret_cast<Vector::BLF::AfdxBusStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::Reserved108:
            /* do nothing */
            break;

        case Vector::BLF::ObjectType::AFDX_ERROR_EVENT:
            show(reinterpret_cast<Vector::BLF::AfdxErrorEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::A429_ERROR:
            show(reinterpret_cast<Vector::BLF::A429Error *>(ohb));
            break;

        case Vector::BLF::ObjectType::A429_STATUS:
            show(reinterpret_cast<Vector::BLF::A429Status *>(ohb));
            break;

        case Vector::BLF::ObjectType::A429_BUS_STATISTIC:
            show(reinterpret_cast<Vector::BLF::A429BusStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::A429_MESSAGE:
            show(reinterpret_cast<Vector::BLF::A429Message *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_STATISTIC:
            show(reinterpret_cast<Vector::BLF::EthernetStatistic *>(ohb));
            break;

        case Vector::BLF::ObjectType::Unknown115:
            show(reinterpret_cast<Vector::BLF::RestorePointContainer *>(ohb));
            break;

        case Vector::BLF::ObjectType::Reserved116:
        case Vector::BLF::ObjectType::Reserved117:
            /* do nothing */
            break;

        case Vector::BLF::ObjectType::TEST_STRUCTURE:
            show(reinterpret_cast<Vector::BLF::TestStructure *>(ohb));
            break;

        case Vector::BLF::ObjectType::DIAG_REQUEST_INTERPRETATION:
            show(reinterpret_cast<Vector::BLF::DiagRequestInterpretation *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_FRAME_EX:
            show(reinterpret_cast<Vector::BLF::EthernetFrameEx *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_FRAME_FORWARDED:
            show(reinterpret_cast<Vector::BLF::EthernetFrameForwarded *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_ERROR_EX:
            show(reinterpret_cast<Vector::BLF::EthernetErrorEx *>(ohb));
            break;

        case Vector::BLF::ObjectType::ETHERNET_ERROR_FORWARDED:
            show(reinterpret_cast<Vector::BLF::EthernetErrorForwarded *>(ohb));
            break;

        case Vector::BLF::ObjectType::FUNCTION_BUS:
            show(reinterpret_cast<Vector::BLF::FunctionBus *>(ohb));
            break;

        case Vector::BLF::ObjectType::DATA_LOST_BEGIN:
            show(reinterpret_cast<Vector::BLF::DataLostBegin *>(ohb));
            break;

        case Vector::BLF::ObjectType::DATA_LOST_END:
            show(reinterpret_cast<Vector::BLF::DataLostEnd *>(ohb));
            break;

        case Vector::BLF::ObjectType::WATER_MARK_EVENT:
            show(reinterpret_cast<Vector::BLF::WaterMarkEvent *>(ohb));
            break;

        case Vector::BLF::ObjectType::TRIGGER_CONDITION:
            show(reinterpret_cast<Vector::BLF::TriggerCondition *>(ohb));
            break;

        default:
            std::cout << "Unknown ObjectType" << std::endl;
        }

        /* check objectSize */
        if (ohb->objectSize != ohb->calculateObjectSize())
            std::cout << "ObjectSize=" << std::dec << ohb->objectSize << " doesn't match calculatedObjectSize()=" << ohb->calculateObjectSize() << std::endl;

        /* delete object */
        delete ohb;
    }

    std::cout << std::endl;
    std::cout << "End of file." << std::endl;
    std::cout << "uncompressedFileSize: " << std::dec << file.currentUncompressedFileSize << std::endl;
    std::cout << "objectCount: " << std::dec << file.currentObjectCount << std::endl;
    std::cout << "objectsRead: " << std::dec << objectsRead << std::endl;

    file.close();

    return 0;
}

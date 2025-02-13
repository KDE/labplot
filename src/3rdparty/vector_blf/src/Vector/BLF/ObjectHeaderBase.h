// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include <Vector/BLF/platform.h>

#include <cstdint>

#include <Vector/BLF/AbstractFile.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief object type
 *
 * Object type.
 */
enum class ObjectType : uint32_t {
    UNKNOWN = 0, /**< unknown object */
    CAN_MESSAGE = 1, /**< CAN message object */
    CAN_ERROR = 2, /**< CAN error frame object */
    CAN_OVERLOAD = 3, /**< CAN overload frame object */
    CAN_STATISTIC = 4, /**< CAN driver statistics object */
    APP_TRIGGER = 5, /**< application trigger object */
    ENV_INTEGER = 6, /**< environment integer object */
    ENV_DOUBLE = 7, /**< environment double object */
    ENV_STRING = 8, /**< environment string object */
    ENV_DATA = 9, /**< environment data object */
    LOG_CONTAINER = 10, /**< container object */
    LIN_MESSAGE = 11, /**< LIN message object */
    LIN_CRC_ERROR = 12, /**< LIN CRC error object */
    LIN_DLC_INFO = 13, /**< LIN DLC info object */
    LIN_RCV_ERROR = 14, /**< LIN receive error object */
    LIN_SND_ERROR = 15, /**< LIN send error object */
    LIN_SLV_TIMEOUT = 16, /**< LIN slave timeout object */
    LIN_SCHED_MODCH = 17, /**< LIN scheduler mode change object */
    LIN_SYN_ERROR = 18, /**< LIN sync error object */
    LIN_BAUDRATE = 19, /**< LIN baudrate event object */
    LIN_SLEEP = 20, /**< LIN sleep mode event object */
    LIN_WAKEUP = 21, /**< LIN wakeup event object */
    MOST_SPY = 22, /**< MOST spy message object */
    MOST_CTRL = 23, /**< MOST control message object */
    MOST_LIGHTLOCK = 24, /**< MOST light lock object */
    MOST_STATISTIC = 25, /**< MOST statistic object */
    Reserved26 = 26, /**< reserved */
    Reserved27 = 27, /**< reserved */
    Reserved28 = 28, /**< reserved */
    FLEXRAY_DATA = 29, /**< FLEXRAY data object */
    FLEXRAY_SYNC = 30, /**< FLEXRAY sync object */
    CAN_DRIVER_ERROR = 31, /**< CAN driver error object */
    MOST_PKT = 32, /**< MOST Packet */
    MOST_PKT2 = 33, /**< MOST Packet including original timestamp */
    MOST_HWMODE = 34, /**< MOST hardware mode event */
    MOST_REG = 35, /**< MOST register data (various chips) */
    MOST_GENREG = 36, /**< MOST register data (MOST register) */
    MOST_NETSTATE = 37, /**< MOST NetState event */
    MOST_DATALOST = 38, /**< MOST data lost */
    MOST_TRIGGER = 39, /**< MOST trigger */
    FLEXRAY_CYCLE = 40, /**< FLEXRAY V6 start cycle object */
    FLEXRAY_MESSAGE = 41, /**< FLEXRAY V6 message object */
    LIN_CHECKSUM_INFO = 42, /**< LIN checksum info event object */
    LIN_SPIKE_EVENT = 43, /**< LIN spike event object */
    CAN_DRIVER_SYNC = 44, /**< CAN driver hardware sync */
    FLEXRAY_STATUS = 45, /**< FLEXRAY status event object */
    GPS_EVENT = 46, /**< GPS event object */
    FR_ERROR = 47, /**< FLEXRAY error event object */
    FR_STATUS = 48, /**< FLEXRAY status event object */
    FR_STARTCYCLE = 49, /**< FLEXRAY start cycle event object */
    FR_RCVMESSAGE = 50, /**< FLEXRAY receive message event object */
    REALTIMECLOCK = 51, /**< Realtime clock object */
    Reserved52 = 52, /**< this object ID is available for the future */
    Reserved53 = 53, /**< this object ID is available for the future */
    LIN_STATISTIC = 54, /**< LIN statistic event object */
    J1708_MESSAGE = 55, /**< J1708 message object */
    J1708_VIRTUAL_MSG = 56, /**< J1708 message object with more than 21 data bytes */
    LIN_MESSAGE2 = 57, /**< LIN frame object - extended */
    LIN_SND_ERROR2 = 58, /**< LIN transmission error object - extended */
    LIN_SYN_ERROR2 = 59, /**< LIN sync error object - extended */
    LIN_CRC_ERROR2 = 60, /**< LIN checksum error object - extended */
    LIN_RCV_ERROR2 = 61, /**< LIN receive error object */
    LIN_WAKEUP2 = 62, /**< LIN wakeup event object  - extended */
    LIN_SPIKE_EVENT2 = 63, /**< LIN spike event object - extended */
    LIN_LONG_DOM_SIG = 64, /**< LIN long dominant signal object */
    APP_TEXT = 65, /**< text object */
    FR_RCVMESSAGE_EX = 66, /**< FLEXRAY receive message ex event object */
    MOST_STATISTICEX = 67, /**< MOST extended statistic event */
    MOST_TXLIGHT = 68, /**< MOST TxLight event */
    MOST_ALLOCTAB = 69, /**< MOST Allocation table event */
    MOST_STRESS = 70, /**< MOST Stress event */
    ETHERNET_FRAME = 71, /**< Ethernet frame object */
    SYS_VARIABLE = 72, /**< system variable object */
    CAN_ERROR_EXT = 73, /**< CAN error frame object (extended) */
    CAN_DRIVER_ERROR_EXT = 74, /**< CAN driver error object (extended) */
    LIN_LONG_DOM_SIG2 = 75, /**< LIN long dominant signal object - extended */
    MOST_150_MESSAGE = 76, /**< MOST150 Control channel message */
    MOST_150_PKT = 77, /**< MOST150 Asynchronous channel message */
    MOST_ETHERNET_PKT = 78, /**< MOST Ethernet channel message */
    MOST_150_MESSAGE_FRAGMENT = 79, /**< Partial transmitted MOST50/150 Control channel message */
    MOST_150_PKT_FRAGMENT = 80, /**< Partial transmitted MOST50/150 data packet on asynchronous channel */
    MOST_ETHERNET_PKT_FRAGMENT = 81, /**< Partial transmitted MOST Ethernet packet on asynchronous channel */
    MOST_SYSTEM_EVENT = 82, /**< Event for various system states on MOST */
    MOST_150_ALLOCTAB = 83, /**< MOST50/150 Allocation table event */
    MOST_50_MESSAGE = 84, /**< MOST50 Control channel message */
    MOST_50_PKT = 85, /**< MOST50 Asynchronous channel message */
    CAN_MESSAGE2 = 86, /**< CAN message object - extended */
    LIN_UNEXPECTED_WAKEUP = 87,
    LIN_SHORT_OR_SLOW_RESPONSE = 88,
    LIN_DISTURBANCE_EVENT = 89,
    SERIAL_EVENT = 90,
    OVERRUN_ERROR = 91, /**< driver overrun event */
    EVENT_COMMENT = 92,
    WLAN_FRAME = 93,
    WLAN_STATISTIC = 94,
    MOST_ECL = 95, /**< MOST Electrical Control Line event */
    GLOBAL_MARKER = 96,
    AFDX_FRAME = 97,
    AFDX_STATISTIC = 98,
    KLINE_STATUSEVENT = 99, /**< E.g. wake-up pattern */
    CAN_FD_MESSAGE = 100, /**< CAN FD message object */
    CAN_FD_MESSAGE_64 = 101, /**< CAN FD message object */
    ETHERNET_RX_ERROR = 102, /**< Ethernet RX error object */
    ETHERNET_STATUS = 103, /**< Ethernet status object */
    CAN_FD_ERROR_64 = 104, /**< CAN FD Error Frame object */
    LIN_SHORT_OR_SLOW_RESPONSE2 = 105,
    AFDX_STATUS = 106, /**< AFDX status object */
    AFDX_BUS_STATISTIC = 107, /**< AFDX line-dependent busstatistic object */
    Reserved108 = 108,
    AFDX_ERROR_EVENT = 109, /**< AFDX asynchronous error event */
    A429_ERROR = 110, /**< A429 error object */
    A429_STATUS = 111, /**< A429 status object */
    A429_BUS_STATISTIC = 112, /**< A429 busstatistic object */
    A429_MESSAGE = 113, /**< A429 Message */
    ETHERNET_STATISTIC = 114, /**< Ethernet statistic object */
    Unknown115 = 115,
    Reserved116 = 116,
    Reserved117 = 117,
    TEST_STRUCTURE = 118, /**< Event for test execution flow */
    DIAG_REQUEST_INTERPRETATION = 119, /**< Event for correct interpretation of diagnostic requests */
    ETHERNET_FRAME_EX = 120, /**< Ethernet packet extended object */
    ETHERNET_FRAME_FORWARDED = 121, /**< Ethernet packet forwarded object */
    ETHERNET_ERROR_EX = 122, /**< Ethernet error extended object */
    ETHERNET_ERROR_FORWARDED = 123, /**< Ethernet error forwarded object */
    FUNCTION_BUS = 124, /**< FunctionBus object */
    DATA_LOST_BEGIN = 125, /**< Data lost begin */
    DATA_LOST_END = 126, /**< Data lost end */
    WATER_MARK_EVENT = 127, /**< Watermark event */
    TRIGGER_CONDITION = 128, /**< Trigger Condition event */
    CAN_SETTING_CHANGED = 129, /**< CAN Settings Changed object */
    DISTRIBUTED_OBJECT_MEMBER = 130, /**< Distributed object member (communication setup) */
    ATTRIBUTE_EVENT = 131, /**< ATTRIBUTE event (communication setup) */
};

/** object signature */
const uint32_t ObjectSignature = 0x4A424F4C; /* LOBJ */

/**
 * @brief Base object header type definition
 *
 * Object header base structure.
 */
struct VECTOR_BLF_EXPORT ObjectHeaderBase {
    ObjectHeaderBase(const uint16_t headerVersion, const ObjectType objectType);
    virtual ~ObjectHeaderBase() noexcept = default;
    ObjectHeaderBase(const ObjectHeaderBase &) = default;
    ObjectHeaderBase & operator=(const ObjectHeaderBase &) = default;
    ObjectHeaderBase(ObjectHeaderBase &&) = default;
    ObjectHeaderBase & operator=(ObjectHeaderBase &&) = default;

    /**
     * Read the data of this object
     *
     * @param is input stream
     */
    virtual void read(AbstractFile & is);

    /**
     * Write the data of this object
     *
     * @param os output stream
     */
    virtual void write(AbstractFile & os);

    /**
     * Calculates the headerSize
     *
     * @return header size
     */
    virtual uint16_t calculateHeaderSize() const;

    /**
     * Calculates the objectSize
     *
     * @return object size
     */
    virtual uint32_t calculateObjectSize() const;

    /**
     * @brief signature (ObjectSignature)
     *
     * Object signature, must be ObjectSignature.
     */
    uint32_t signature {ObjectSignature};

    /**
     * @brief sizeof object header
     *
     * Size of header in bytes, set this member to
     * sizeof(ObjectHeader) or
     * sizeof(ObjectHeader2) depending on
     * the object header type used for the object.
     */
    uint16_t headerSize {};

    /**
     * @brief header version (1)
     *
     * Version number of object header.
     *
     * Set this member to 1 if the object has a member
     * of type ObjectHeader.
     *
     * Set this member to 2 if the object has a member
     * of type ObjectHeader2.
     *
     * @note is set in ObjectHeader/ObjectHeader2
     */
    uint16_t headerVersion {};

    /**
     * @brief object size
     *
     * Object size in bytes.
     */
    uint32_t objectSize {};

    /**
     * @brief object type
     *
     * Object type.
     *
     * @note is set in each event class constructor
     */
    ObjectType objectType {ObjectType::UNKNOWN};
};

}
}

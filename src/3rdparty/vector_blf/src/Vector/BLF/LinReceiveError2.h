// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinDatabyteTimestampEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LIN_RCV_ERROR2
 *
 * This event may have a wide variety of causes.
 * An external Master can cause a receive error event:
 *   - by transmitting sync break that is too short,
 *   - by not returning the correct value 0x55 in the sync field,
 *   - by assigning an incorrect parity to the frame identifier.
 * Other reasons:
 *   - Slave transmitting an illegal character during a Bus Idle phase (e.g. because it did not
 *     finish transmission quickly enough and the checksum byte of the response was sent during
 *     the Bus Idle phase),
 *   - Faulty (dominant) stop bit (i.e. framing error),
 *   - LIN hardware receives a character that is different from the character sent during
 *     transmission
 *   - LIN hardware only receives part of a frame, at the start of a measurement (in a correctly
 *     functioning system).
 */
struct VECTOR_BLF_EXPORT LinReceiveError2 final : ObjectHeader, LinDatabyteTimestampEvent {
    LinReceiveError2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief data bytes.
     */
    std::array<uint8_t, 8> data {};

    /**
     * Slave Identifier in the
     * Final State Machine
     * (obsolete)
     */
    uint8_t fsmId {};

    /**
     * State Identifier of a Slave in
     * the Final State Machine
     * (obsolete)
     */
    uint8_t fsmState {};

    /**
     * The lower 4 bits indicate the
     * LIN hard-ware state at the
     * time the error has occurred,
     * while the upper 4 bits
     * indicate the reason of the
     * error
     *
     * Value for the state:
     *   - 0: Bus idle
     *   - 1: Waiting for SynchBreak
     *   - 2: Waiting for SynchField
     *   - 3: Waiting for frame ID
     *   - 4-12: Waiting for data byte
     *     or checksum byte depending
     *     on the frame length. E.g.
     *     value 4 for FrameLength=0,
     *     value 12 for FrameLength=8
     *   - 14: Consecutive event (i.e.
     *     event resulting from further
     *     data interpretation, after
     *     already notified error for
     *     first offending byte)
     *   - 15: Not expected event (i.e.
     *     not WakeupRequest) during
     *     sleep mode. Occurs for LIN
     *     hardware in Master mode
     *     only
     *
     * Values for the reason:
     *   - 0: Timeout
     *   - 1: Received an unexpected
     *     byte violating protocol. In
     *     this case, mOffendingByte
     *     member contains its value
     *   - 2: Received a byte with
     *     framing error (with
     *     dominant stop bit). In this
     *     case, mOffendingByte
     *     member contains its value
     *   - 3: Unexpected Break field
     *   - 4: Unidentified error
     */
    uint8_t stateReason {};

    /**
     * Byte value that resulted the
     * protocol violation. Only
     * valid for certain values of
     * mStateReason
     */
    uint8_t offendingByte {};

    /**
     * Specifies the detail level of
     * the event. Following values
     * are possible:
     *   - 0: short
     *   - 1: full
     * Most members are not valid
     * unless this member is 1
     */
    uint8_t shortError {};

    /**
     * Flag indicating if the error is
     * a result of an attempt to
     * resolve frame length.
     * Following values are
     * possible:
     *   - 0: False
     *   - 1: True
     */
    uint8_t timeoutDuringDlcDetection {};

    /**
     * @brief ETF collision flag
     *
     * Flag indicating whether this
     * frame is Event-Triggered
     * one:
     *   - 0: not ETF
     *   - 1: ETF
     */
    uint8_t isEtf {};

    /**
     * Flag indicating whether at
     * least one data byte value is
     * valid
     */
    uint8_t hasDatabytes {};

    /* the following variables are only available in Version 2 and above */

    /**
     * @brief Response baudrate of the event in bit/sec
     *
     * Event's baudrate
     * measured in response [in
     * bits/sec]
     */
    uint32_t respBaudrate {};

    /** reserved */
    uint32_t reservedLinReceiveError {};

    /* the following variables are only available in Version 3 and above */

    /**
     * @brief Exact baudrate of the header in bit/sec
     *
     * Event's baudrate
     * measured in header [in
     * bits/sec]
     */
    double exactHeaderBaudrate {};

    /**
     * @brief Early stop bit offset for UART timestamps in ns
     *
     * Early stop bit offset in
     * frame header for UART
     * timestamps [in ns]
     */
    uint32_t earlyStopbitOffset {};

    /**
     * @brief Early stop bit offset in frame response for UART timestamps in ns
     *
     * Early stop bit offset in
     * frame response for
     * UART timestamps [in ns]
     */
    uint32_t earlyStopbitOffsetResponse {};
};

}
}

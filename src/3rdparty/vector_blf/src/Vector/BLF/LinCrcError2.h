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
 * @brief LIN_CRC_ERROR2
 *
 * Checksum error event occurring when Slave sends an incorrect checksum value for a
 * frame response that is otherwise correct.
 */
struct VECTOR_BLF_EXPORT LinCrcError2 final : ObjectHeader, LinDatabyteTimestampEvent {
    LinCrcError2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief data bytes.
     *
     * Data byte values
     */
    std::array<uint8_t, 8> data {};

    /**
     * @brief checksum byte
     *
     * Checksum byte value
     */
    uint16_t crc {};

    /**
     * @brief direction
     *
     * Direction of bus events
     *   - 0: Rx (received)
     *   - 1: Tx (transmit receipt)
     *   - 2: Tx Request (transmit request)
     */
    uint8_t dir {};

    /**
     * Slave Identifier in the Final State
     * Machine (obsolete)
     */
    uint8_t fsmId {};

    /**
     * State Identifier of a Slave in the
     * Final State Machine (obsolete)
     */
    uint8_t fsmState {};

    /**
     * @brief simulated frame
     *
     * Flag indicating whether this frame a
     * simulated one:
     *   - 0: real frame
     *   - 1: simulated frame
     */
    uint8_t simulated {};

    /** reserved */
    uint16_t reservedLinCrcError1 {};

    /* the following variables are only available in Version 2 and above */

    /**
     * @brief Response baudrate of the event in bit/sec
     *
     * Event’s baudrate measured in
     * response [in bits/sec]
     */
    uint32_t respBaudrate {};

    /** reserved */
    uint32_t reservedLinCrcError2 {};

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

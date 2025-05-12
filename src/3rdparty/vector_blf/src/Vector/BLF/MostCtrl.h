// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_CTRL
 *
 * Message on MOST 25 Control Channel; received or transmitted in node mode.
 */
struct VECTOR_BLF_EXPORT MostCtrl final : ObjectHeader {
    MostCtrl();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel the message was sent or received.
     */
    uint16_t channel {};

    /**
     * Direction of message events:
     *   - 0: Rx (received)
     *   - 1: Tx (transmit receipt)
     *   - 2: Tx Request (transmit request)
     */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedMostCtrl1 {};

    /**
     * Source address
     */
    uint32_t sourceAdr {};

    /**
     * Target address
     */
    uint32_t destAdr {};

    /**
     * 17 data bytes
     */
    std::array<uint8_t, 17> msg {};

    /** reserved */
    uint8_t reservedMostCtrl2 {};

    /**
     * @brief Control message sub type
     *
     * Sub type of a MOST 25 Control message (see data sheet of OS8104 also).
     *   - 0: Normal
     *   - 1: RemoteRead
     *   - 2: RemoteWrite
     *   - 3: Alloc
     *   - 4: Dealloc
     *   - 5: GetSource
     *   - >5: not used so far
     */
    uint16_t rTyp {};

    /**
     * @brief Addressing mode
     *
     * Addressing mode of MOST25 Control messages.
     *   - 0x00: Device (logical node address)
     *   - 0x10: Node position
     *   - 0x20: Broadcast
     *   - 0x30: Groupcast
     *   - 0xFF: Unknown
     */
    uint8_t rTypAdr {};

    /**
     * @brief Transmission state MOST25
     *
     * Transmission state of a MOST25 Control message.
     *   - Bit 0:
     *     Meaning:
     *     - 0: bus inactive
     *     - 1: bus active
     *     Restriction:
     *     - only for Dir = Rx (MOSTCtrl) or MOSTSpy
     *   - Bit 1:
     *     Meaning:
     *     - 1: unlock event during transmission (Unl)
     *     Restriction:
     *     - only for Dir = Rx (MOSTCtrl) or MOSTSpy
     *   - Bit 4:
     *     Meaning:
     *     - 1: acknowledged (Ack)
     *     Restriction:
     *     - only for Dir = Tx (always set to 1 for Rx messages in node mode)
     *   - Bit 5:
     *     Meaning:
     *     - 1: not acknowledged (NAck)
     *     Restriction:
     *     - only for Dir = Tx
     *   - Bit 6:
     *     Meaning:
     *     - Send result:
     *       - 0: Transmission error (TxF)
     *       - 1: OK
     *     Restriction:
     *     - only for Dir = Tx (MOSTCtrl)
     */
    uint8_t state {};

    /** reserved */
    uint8_t reservedMostCtrl3 {};

    /**
     * @brief acknowledge bits
     *
     * AckNack holds the transmit status of a control message (see Transmit Status Register of OS8104
     * for MOST25).
     *   - Bit 0:
     *     Meaning:
     *       - 1: no response (NoResp)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 1:
     *     Meaning:
     *       - 1: valid receipt (Valid)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 2:
     *     Meaning:
     *       - 1: CRC Error (CRCError)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 3:
     *     Meaning:
     *       - 1: receive buffer full (RxBufFull)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 4:
     *     Meaning:
     *       - 1: acknowledged (Ack)
     *     Restriction:
     *       - only for Dir = Tx or spy messages (always
     *         set to 1 for Rx messages in node mode)
     *   - Bit 5:
     *     Meaning:
     *       - 1: negative acknowledge (NAck)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     */
    uint8_t ackNack {};

    /** reserved */
    uint32_t reservedMostCtrl4 {};
};

}
}

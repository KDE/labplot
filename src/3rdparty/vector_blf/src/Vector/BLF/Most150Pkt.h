// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_150_PKT
 *
 * Message on MOST150 Packet Data Channel.
 */
struct VECTOR_BLF_EXPORT Most150Pkt final : ObjectHeader2 {
    Most150Pkt();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Application channel
     */
    uint16_t channel {};

    /**
     * @brief direction: 0: Rx; 1: Tx; 2: TxRequest
     *
     * Direction of message events:
     *   - 0: Rx (received)
     *   - 1: Tx (transmit receipt)
     *   - 2: Tx Request (transmit request)
     */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedMost150Pkt1 {};

    /**
     * @brief source address
     *
     * Source address
     */
    uint32_t sourceAdr {};

    /**
     * @brief target address
     *
     * Target address
     */
    uint32_t destAdr {};

    /**
     * @brief Tranfer Type
     *
     * Message-like events can either be recorded through the MOST transceiver chip or through a
     * separate network spy.
     *   - 1: Node
     *     MOST transceiver reported the message (either due to a successful reception or
     *     as acknowledgment for a transmit request).
     *   - 2: Spy
     *     Message was reported by the network spy. The Spy sees all messages
     *     independently of the desti-nation address.
     */
    uint8_t transferType {};

    /**
     * @brief transmission status
     *
     * Transmission states of MOST50/150 messages and packets.
     *   - Bit 0:
     *     Meaning:
     *       - 0: bus inactive
     *       - 1: bus active
     *     Restriction:
     *       - only for Dir = Rx or mTransferType = Spy
     *   - Bit 4:
     *     Meaning:
     *       - 1: acknowledged (Ack)
     *     Restriction
     *       - only for mTransferType = Node and Dir =
     *         Tx (always set to 1 for Rx messages in node
     *         mode)
     *   - Bit 5:
     *     Meaning:
     *       - 1: not acknowledged (NAck)
     *     Restriction:
     *       - only for Dir = Tx
     *   - Bit 6:
     *     Meaning: Send result:
     *       - 0: Transmission error (TxF)
     *       - 1: OK
     *     Restriction:
     *       - only for Dir = Tx
     *       - only for mTransferType = Node
     */
    uint8_t state {};

    /**
     * @brief acknowledge code
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
    uint8_t reservedMost150Pkt2 {};

    /**
     * @brief Cyclic Redundancy Check
     *
     * Cyclic Redundancy Check
     */
    uint32_t crc {};

    /**
     * @brief a preemptive acknowledge code
     *
     * Pre-emptive acknowledge code (spy only)
     *   - 0x00: No Response
     *   - 0x01: Buffer full
     *   - 0x04: OK
     */
    uint8_t pAck {};

    /**
     * @brief CRC acknowledge from the packet receiver(s) to the packet transmitter
     *
     * CRC acknowledge from the packet receiver(s) to
     * the packet transmitter (spy only)
     *   - 0x00: No Response
     *   - 0x01: CRC error
     *   - 0x04: OK
     */
    uint8_t cAck {};

    /**
     * @brief priority of the message
     *
     * Priority (not used; write 0x00)
     */
    uint8_t priority {};

    /**
     * @brief packet index, increments per message on MOST
     *
     * Packet index, increments per message on MOST
     */
    uint8_t pIndex {};

    /**
     * @brief length of variable data in bytes (1014 max)
     *
     * Length of variable data in bytes (1524 max)
     */
    uint32_t pktDataLength {};

    /** reserved */
    uint32_t reservedMost150Pkt3 {};

    /**
     * @brief variable data
     *
     * Variable data
     */
    std::vector<uint8_t> pktData {};
};

}
}

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
 * @brief MOST_PKT2
 *
 * Message on MOST25 Packet Data Channel.
 */
struct VECTOR_BLF_EXPORT MostPkt2 final : ObjectHeader2 {
    MostPkt2();

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
     * Direction of message events:
     *   - 0: Rx (received)
     *   - 1: Tx (transmit receipt)
     *   - 2: Tx Request (transmit request)
     */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedMostPkt1 {};

    /**
     * Source address
     */
    uint32_t sourceAdr {};

    /**
     * Target address
     */
    uint32_t destAdr {};

    /**
     * Arbitration byte
     */
    uint8_t arbitration {};

    /**
     * Obsolete member; read/write 0
     */
    uint8_t timeRes {};

    /**
     * Number of quadlets
     */
    uint8_t quadsToFollow {};

    /** reserved */
    uint8_t reservedMostPkt2 {};

    /**
     * Cyclic Redundancy Check
     */
    uint16_t crc {};

    /**
     * Priority
     */
    uint8_t priority {};

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
     * Transmission state
     *   - 0 for Rx
     *   - 0x40 for TxOk (transmit request)
     */
    uint8_t state {};

    /** reserved */
    uint8_t reservedMostPkt3 {};

    /** reserved */
    uint16_t reservedMostPkt4 {};

    /**
     * @brief length of variable data in bytes
     *
     * Length of variable data in bytes (1014 max)
     */
    uint32_t pktDataLength {};

    /** reserved */
    uint32_t reservedMostPkt5 {};

    /**
     * @brief variable data
     *
     * Variable data
     */
    std::vector<uint8_t> pktData {};
};

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>
#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief AFDX_FRAME
 *
 * AFDX frame.
 */
struct VECTOR_BLF_EXPORT AfdxFrame final : ObjectHeader {
    AfdxFrame();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * Ethernet (MAC) address of source computer
     * (network byte order).
     */
    std::array<uint8_t, 6> sourceAddress {};

    /**
     * The channel of the frame.
     */
    uint16_t channel {};

    /**
     * Ethernet (MAC) address of target computer
     * (network byte order).
     */
    std::array<uint8_t, 6> destinationAddress {};

    /** enumeration for dir */
    enum Dir : uint16_t {
        /** Receive */
        Rx = 0,

        /** Transmit */
        Tx = 1,

        /** Transmit Request */
        TxRq = 2
    };

    /**
     * @brief Direction flag
     */
    uint16_t dir {};

    /**
     * EtherType which indicates protocol for
     * Ethernet payload data
     *
     * See Ethernet standard specification for valid
     * values.
     */
    uint16_t type {};

    /**
     * TPID when VLAN tag valid, zweo when no
     * VLAN. See Ethernet stnadard specification.
     */
    uint16_t tpid {};

    /**
     * TCI when VLAND tag valid, zero when no
     * VLAN. See Ethernet standard specification.
     */
    uint16_t tci {};

    /**
     * Channel number of the underlying Ethernet
     * interface, where the frame originated from.
     */
    uint8_t ethChannel {};

    /** reserved */
    uint8_t reservedAfdxFrame1 {};

    /**
     * Status- and error flags as:
     *
     * - Bit 0: Frame from line-B
     * - Bit 1: Packet is redundant
     * - Bit 2: Frame is a fragment only
     * - Bit 3: Frame is already reassembled
     * - Bit 4: Packet is not a valid AFDX frame
     * - Bit 5: AFDX-SequenceNo is invalud
     * - Bit 6: Redundancy timeout violated
     * - Bit 7: Redundancy error encountered
     * - Bit 8: A / B interface mismatch
     * - Bit 11: Fragmentation error
     */
    uint16_t afdxFlags {};

    /** reserved */
    uint16_t reservedAfdxFrame2 {};

    /**
     * Time period since last received frame on this
     * virtual link in micro-seconds
     */
    uint32_t bagUsec {};

    /**
     * @brief Number of valid payLoad bytes
     *
     * Length of Ethernet payload data in bytes. Max.
     * 1500 Bytes (without Ethernet header)
     */
    uint16_t payLoadLength {};

    /** reserved */
    uint16_t reservedAfdxFrame3 {};

    /** reserved */
    uint32_t reservedAfdxFrame4 {};

    /**
     * @brief Ethernet payload data
     *
     * Ethernet payload data (without Ethernet
     * header).
     *
     * Max 1582 (1600 packet length - 18 header) data bytes per frame
     */
    std::vector<uint8_t> payLoad {};
};

}
}

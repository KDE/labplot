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
 * @brief ETHERNET_FRAME
 *
 * Ethernet frame
 */
struct VECTOR_BLF_EXPORT EthernetFrame final : ObjectHeader {
    EthernetFrame();

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
        Rx = 0,
        Tx = 1,
        TxRq = 2
    };

    /**
     * @brief Direction flag
     *
     * Direction flag
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
     * TPID when VLAN tag valid, zero when no
     * VLAN. See Ethernet standard specification.
     */
    uint16_t tpid {};

    /**
     * TCI when VLAND tag valid, zero when no
     * VLAN. See Ethernet standard specification.
     */
    uint16_t tci {};

    /**
     * @brief Number of valid payLoad bytes
     *
     * Length of Ethernet payload data in bytes. Max.
     * 1500 Bytes (without Ethernet header)
     */
    uint16_t payLoadLength {};

    /** reserved */
    uint64_t reservedEthernetFrame {};

    /**
     * @brief Max 1500 data bytes per frame
     *
     * Ethernet payload data (without Ethernet
     * header)
     */
    std::vector<uint8_t> payLoad {};
};

}
}

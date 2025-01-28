// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief ETHERNET_RX_ERROR
 *
 * Ethernet RX error frame.
 */
struct VECTOR_BLF_EXPORT EthernetRxError final : ObjectHeader {
    EthernetRxError();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * Calculates structLength.
     *
     * @return struct length
     */
    virtual uint16_t calculateStructLength() const;

    /**
     * @brief Length of this structure
     *
     * Length of this structure, without
     * sizeof(ObjectHeader) and without raw data
     * length
     */
    uint16_t structLength {};

    /**
     * The channel of the frame.
     */
    uint16_t channel {};

    /** enumeration for dir */
    enum Dir : uint16_t {
        /** receive */
        Rx = 0,

        /** transmit */
        Tx = 1,

        /** transmit request */
        TxRq = 2
    };

    /**
     * @brief Direction flag
     */
    uint16_t dir {};

    /**
     * @brief HW channel. 0 = invalid.
     */
    uint16_t hardwareChannel {};

    /**
     * @brief Frame Check Sum
     *
     * Ethernet frame checksum.
     */
    uint32_t fcs {};

    /**
     * @brief Number of valid raw ethernet data bytes
     *
     * Number of valid raw ethernet data bytes, starting
     * with Target MAC ID.
     */
    uint16_t frameDataLength {};

    /** reserved */
    uint16_t reservedEthernetRxError {};

    /**
     * Error code
     *
     * - 1: Data Length Error
     * - 2: Invalid CRC
     * - 3: Invalid Data received
     * - 4: Collision detected
     */
    uint32_t error {};

    /**
     * @brief Max 1600 data bytes per frame
     *
     * Raw Ethernet frame data.
     * Max 1522 data bytes per frame.
     */
    std::vector<uint8_t> frameData {};
};

}
}

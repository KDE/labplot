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
 * @brief FLEXRAY_MESSAGE
 *
 * FlexRay Message received or transmitted on a FlexRay channel.
 */
struct VECTOR_BLF_EXPORT FlexRayV6Message final : ObjectHeader {
    FlexRayV6Message();

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
     * @brief dir flag (tx, rx)
     *
     * Direction Flags
     *   - 0 = Rx
     *   - 1 = Tx
     *   - 2 = Tx Request
     *   - 3 and 4 are for internal use only.
     */
    uint8_t dir {};

    /**
     * @brief additional time field in simulation
     *
     * Additional time field in simulation
     */
    uint8_t lowTime {};

    /**
     * @brief timestamp generated from xModule
     *
     * Timestamp generated from xModule
     */
    uint32_t fpgaTick {};

    /**
     * @brief overflow counter of the timestamp
     *
     * Overflow counter of the timestamp
     */
    uint32_t fpgaTickOverflow {};

    /**
     * @brief clientindex of send node
     *
     * Client index of send node
     */
    uint32_t clientIndexFlexRayV6Message {};

    /**
     * @brief relatvie clustertime, from 0 to cyclelength
     *
     * Relatvie clustertime, from 0 to cyclelength
     */
    uint32_t clusterTime {};

    /**
     * @brief slot identifier
     *
     * slot identifier
     */
    uint16_t frameId {};

    /**
     * CRC of the frame header
     */
    uint16_t headerCrc {};

    /**
     * @brief V6 framestate
     *
     * V6 framestate:
     *   - 0 Payload preample indicator bit
     *   - 1 Sync. frame indicator
     *   - 2 Reserved bit
     *   - 3 Null frame indicator
     *   - 4 Startup frame indicator
     *   - 5-7 Frame state format mask (see below)
     *
     * Bit 5-7 meaning:
     *   - 0 (0x00) Motorola V.6
     *   - 1 (0x20) reserved
     *   - 2 (0x40) BusDoctor
     *   - 3 (0x60) reserved
     *   - 4 (0x80) FlexCard Cyclone
     *   - 5 (0xA0) reserved
     *   - 6 (0xC0) reserved
     *   - 7 (0xE0) reserved
     */
    uint16_t frameState {};

    /**
     * @brief dlc of message
     *
     * Payload length
     */
    uint8_t length {};

    /**
     * @brief current cycle
     *
     * Current cycle number
     */
    uint8_t cycle {};

    /**
     * @brief Bit0 = NMBit, Bit1 = SyncBit, Bit2 = Reserved
     *
     * - Bit 0 = NMBit
     * - Bit 1 = SyncBit
     * - Bit 2 = Reserved
     */
    uint8_t headerBitMask {};

    /** reserved */
    uint8_t reservedFlexRayV6Message1 {};

    /** reserved */
    uint16_t reservedFlexRayV6Message2 {};

    /**
     * @brief array of databytes
     *
     * Payload
     */
    std::array<uint8_t, 64> dataBytes {};
};

}
}

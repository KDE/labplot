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
 * @brief CAN_MESSAGE2
 *
 * CAN data or CAN remote frame received or transmitted on a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanMessage2 final : ObjectHeader {
    CanMessage2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel the frame was sent or received.
     */
    uint16_t channel {};

    /**
     * @brief CAN dir & rtr
     *
     * CAN Message Flags
     *
     * CAN dir, rtr, wu & nerr encoded into flags:
     * - Bit 0: TX
     * - Bit 5: NERR
     * - Bit 6: WU
     * - Bit 7: RTR
     */
    uint8_t flags {};

    /**
     * @brief CAN dlc
     *
     * Data length code of frame (number of valid data
     * bytes, max. 8)
     */
    uint8_t dlc {};

    /**
     * @brief CAN ID
     *
     * Frame identifier.
     */
    uint32_t id {};

    /**
     * @brief CAN data
     *
     * CAN data bytes
     */
    std::vector<uint8_t> data {};

    /**
     * @brief message length in ns
     *
     * Message duration [in ns]. Not including 3
     * Interframe Space bit times and by Rx-messages
     * also not including 1 End-Of-Frame bit time
     */
    uint32_t frameLength {};

    /**
     * @brief complete message length in bits
     *
     * Total number of bits of the message including
     * EOF and Interframe space [in bits]
     */
    uint8_t bitCount {};

    /** reserved */
    uint8_t reservedCanMessage1 {};

    /** reserved */
    uint16_t reservedCanMessage2 {};
};

}
}

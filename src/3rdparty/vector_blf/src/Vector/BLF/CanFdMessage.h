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
 * @brief CAN_FD_MESSAGE
 */
struct VECTOR_BLF_EXPORT CanFdMessage final : ObjectHeader {
    CanFdMessage();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    uint16_t channel {};

    /**
     * enumeration for flags
     */
    enum Flags : uint8_t {
        /** transmit direction */
        TX = (1 << 0),

        /** single wire operation */
        NERR = (1 << 5),

        /** wake up message (high voltage) */
        WU = (1 << 6),

        /** remote transmission request */
        RTR = (1 << 7)
    };

    /**
     * @brief CAN dir & rtr
     *
     * CAN Message Flags
     *
     * CAN dir, rtr, wu & nerr encoded into flags
     */
    uint8_t flags {};

    /**
     * @brief CAN dlc
     */
    uint8_t dlc {};

    /**
     * @brief CAN ID
     */
    uint32_t id {};

    /**
     * @brief message length in ns - without 3 inter frame space bits and by Rx-message also without 1 End-Of-Frame bit
     */
    uint32_t frameLength {};

    /**
     * @brief bit count of arbitration phase
     */
    uint8_t arbBitCount {};

    /**
     * enumeration for canFdFlags
     */
    enum CanFdFlags : uint8_t {
        /** extended data length */
        EDL = (1 << 0),

        /** bit rate switch */
        BRS = (1 << 1),

        /** error state indicator */
        ESI = (1 << 2)
    };

    /**
     * @brief CAN FD flags
     */
    uint8_t canFdFlags {};

    /**
     * @brief Valid payload length of data
     */
    uint8_t validDataBytes {};

    /** reserved */
    uint8_t reservedCanFdMessage1 {};

    /** reserved */
    uint32_t reservedCanFdMessage2 {};

    /**
     * @brief CAN FD data
     */
    std::array<uint8_t, 64> data {};

    /** reserved */
    uint32_t reservedCanFdMessage3 {};
};

}
}

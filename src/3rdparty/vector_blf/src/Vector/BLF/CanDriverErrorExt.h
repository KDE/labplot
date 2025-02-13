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
 * @brief CAN_DRIVER_ERROR_EXT
 *
 * Extended CAN driver error information for transceiver of a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanDriverErrorExt final : ObjectHeader {
    CanDriverErrorExt();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * CAN channel the driver error information
     * belongs to.
     */
    uint16_t channel {};

    /**
     * @brief # of TX errors
     *
     * Number of transmit errors that occurred in CAN
     * controller for that channel.
     */
    uint8_t txErrors {};

    /**
     * @brief # of RX errors
     *
     * Number of receive errors that occurred in CAN
     * controller for that channel.
     */
    uint8_t rxErrors {};

    /**
     * @brief CAN driver error code
     *
     * Error code
     */
    uint32_t errorCode {};

    /**
     * @brief flags
     *
     * To be defined.
     */
    uint32_t flags {};

    /**
     * @brief state register
     *
     * To be defined.
     */
    uint8_t state {};

    /** reserved */
    uint8_t reservedCanDriverErrorExt1 {};

    /** reserved */
    uint16_t reservedCanDriverErrorExt2 {};

    /** reserved */
    std::array<uint32_t, 4> reservedCanDriverErrorExt3 {};
};

}
}

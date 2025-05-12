// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief CAN_DRIVER_ERROR
 *
 * CAN driver error information for transceiver of a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanDriverError final : ObjectHeader {
    CanDriverError();

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
};

}
}

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
 * @brief LIN_SLV_TIMEOUT
 *
 * This event occurs on a timeout in Final State Machine defined on LIN Hardware via
 * CAPL. The technology of Final State Machine on LIN Hardware is still supported, but it is
 * obsolete.
 */
struct VECTOR_BLF_EXPORT LinSlaveTimeout final : ObjectHeader {
    LinSlaveTimeout();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel number where the event notified
     */
    uint16_t channel {};

    /**
     * Slave Identifier in the Final State Machine
     */
    uint8_t slaveId {};

    /**
     * Source state identifier of a Slave in the Final
     * State Machine
     */
    uint8_t stateId {};

    /**
     * Target state identifier of a Slave in the Final
     * State Machine
     */
    uint32_t followStateId {};
};

}
}

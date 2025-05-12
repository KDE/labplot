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
 * @brief LIN_SND_ERROR
 *
 * @deprecated
 *
 * This event occurs when no Slave responds to a frame header from Master.
 */
struct VECTOR_BLF_EXPORT LinSendError final : ObjectHeader {
    LinSendError();

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
     * @brief LIN ID
     *
     * Frame identifier
     */
    uint8_t id {};

    /**
     * @brief LIN DLC
     *
     * Frame length
     */
    uint8_t dlc {};

    /**
     * Slave Identifier in the Final State Machine
     * (obsolete)
     */
    uint8_t fsmId {};

    /**
     * State Identifier of a Slave in the Final State
     * Machine (obsolete)
     */
    uint8_t fsmState {};

    /**
     * Duration of the frame header [in bit times]
     */
    uint8_t headerTime {};

    /**
     * Duration of the entire frame [in bit times]
     */
    uint8_t fullTime {};
};

}
}

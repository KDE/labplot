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
 * @brief LIN_SYN_ERROR
 *
 * Synchronization errors occur if the LIN hardware cannot synchronize with an external
 * Master. This might happen if the baud rate actually used by the Master deviates by more than 15 %
 * from the baud rate specified by the LIN hardware. In this case the baud rate value should be
 * modified. This error event may also occur if the Master transmits an invalid or corrupted Sync
 * field.
 */
struct VECTOR_BLF_EXPORT LinSyncError final : ObjectHeader {
    LinSyncError();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel number where the event notified.
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedLinSyncError1 {};

    /**
     * Time intervals [in us] detected between the
     * falling signal edges of the Sync field
     */
    std::array<uint16_t, 4> timeDiff {};

    /** reserved */
    uint32_t reservedLinSyncError2 {};
};

}
}

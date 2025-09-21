// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinSynchFieldEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LIN_SYN_ERROR2
 *
 * Synchronization errors occur if the LIN hardware cannot synchronize with an external
 * Master. This might happen if the baud rate actually used by the Master deviates by more than 15 %
 * from the baud rate specified by the LIN hardware. In this case the baud rate value should be
 * modified. This error event may also occur if the Master transmits an invalid or corrupted Sync
 * field.
 */
struct VECTOR_BLF_EXPORT LinSyncError2 final : ObjectHeader, LinSynchFieldEvent {
    LinSyncError2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * Time intervals [in us] detected between the
     * falling signal edges of the Sync field
     */
    std::array<uint16_t, 4> timeDiff {};
};

}
}

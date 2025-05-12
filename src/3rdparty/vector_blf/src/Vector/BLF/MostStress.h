// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_STRESS
 *
 * Information about Stress activity of the hardware interface.
 */
struct VECTOR_BLF_EXPORT MostStress final : ObjectHeader2 {
    MostStress();

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
     * State of Stress mode:
     *   - 0 – Stopped
     *   - 1 – Started
     */
    uint16_t state {};

    /**
     * Stress mode of HW interface:
     *   - 1 – Light
     *   - 2 – Lock
     *   - 3 – Busload Ctrl
     *   - 4 – Busload Async
     *   - 5 – Rx Buffer Ctrl
     *   - 6 – TxLight power
     *   - 7 – Bypass toggling
     *   - 8 – SystemLock flag usage
     *   - 9 – Shutdown flag usage
     *   - 10 – Rx Buffer Async
     */
    uint16_t mode {};

    /** reserved */
    uint16_t reservedMostStress {};
};

}
}

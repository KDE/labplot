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
 * @brief MOST_LIGHTLOCK
 *
 * This event refers to the optical or electrical modulated signal at the transceiver's Rx.
 *
 * "Signal On" means that a modulated signal has been detected.
 *
 * "Lock" means that the receiver PLL (Phase Locked Loop) was able to establish synchronization
 * with the phase of the modulated signal (to "lock").
 *
 * "Stable Lock" means that for a certain period of time no unlock occurred (see MOST
 * specification).
 *
 * In case of a series of unlocks, the time of the different unlocks are accumulated. If this accumulated
 * time is greater than a certain threshold, it is called "Critical Unlock" (details see MOST
 * specification).
 */
struct VECTOR_BLF_EXPORT MostLightLock final : ObjectHeader {
    MostLightLock();

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
     * Signal state:
     *   - 0x01 – Signal On + Lock
     *   - 0x02 – Signal Off (implies No Lock)
     *   - 0x03 – Signal On + No Lock
     *   - 0x10 – Stable Lock
     *   - 0x20 – Critical Unlock
     */
    int16_t state {};

    /** reserved */
    uint32_t reservedMostLightLock {};
};

}
}

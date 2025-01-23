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
 * @brief MOST_SYSTEM_EVENT
 *
 * Event for various system states.
 */
struct VECTOR_BLF_EXPORT MostSystemEvent final : ObjectHeader2 {
    MostSystemEvent();

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
     * @brief identifier of transported data
     *
     * Identification of transported data
     * (enumeration):
     *   - 1 - System Lock (MOST150)
     *   - 2 - Shutdown Flag (MOST150)
     *   - 3 - Shutdown Reason (MOST150)
     */
    uint16_t id {};

    /**
     * @brief current value
     *
     * Current value
     */
    uint32_t value {};

    /**
     * @brief previous value
     *
     * Previous value
     */
    uint32_t valueOld {};

    /** reserved */
    uint32_t reservedMostSystemEvent {};
};

}
}

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
 * @brief MOST_TRIGGER
 *
 * Transports changes of HW IO pins. The event is used for debugging purposes only.
 */
struct VECTOR_BLF_EXPORT MostTrigger final : ObjectHeader2 {
    MostTrigger();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Application channel
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedMostTrigger {};

    /**
     * @brief trigger mode
     *
     * Trigger mode:
     *   - 0 – unknown
     *   - 1 – synchronization master
     *   - 2 – synchronization slave
     */
    uint16_t mode {};

    /**
     * @brief HW info
     *
     * HW that generated the trigger event
     *   - 0 – unknown
     *   - 1 – Optolyzer
     *   - 2 – reserved
     *   - 3 – reserved
     *   - 4 – VN2600/VN2610
     *   - 5 – OptoLyzer OL3150o
     *   - 6 – VN2640
     *   - 7 – OptoLyzer OL3050e
     *   - 8 – SMSC PCI 50
     *   - 9 – MOCCAcompact50e
     */
    uint16_t hw {};

    /**
     * value of IO register
     */
    uint32_t previousTriggerValue {};

    /**
     * value of IO register
     */
    uint32_t currentTriggerValue {};
};

}
}

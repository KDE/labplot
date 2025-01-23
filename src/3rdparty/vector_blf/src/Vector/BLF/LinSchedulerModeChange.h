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
 * @brief LIN_SCHED_MODCH
 *
 * This info event occurs when a Master is simulated and a frame header of a new
 * schedule table is transmitted for the first time. This info event may appear on starting a
 * measurement.
 */
struct VECTOR_BLF_EXPORT LinSchedulerModeChange final : ObjectHeader {
    LinSchedulerModeChange();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel number where the frame sent/received.
     */
    uint16_t channel {};

    /**
     * Index (0-based) of a previously active schedule
     * table
     */
    uint8_t oldMode {};

    /**
     * Index (0-based) of the newly activated schedule
     * table
     */
    uint8_t newMode {};

    /** reserved */
    uint32_t reservedLinSchedulerModeChange {};
};

}
}

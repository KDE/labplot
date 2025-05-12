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
 * @brief LIN_SLEEP
 *
 * This info event occurs at the start of a measurement in order to report the initial state
 * of the LIN hardware and every time the mode (awake/asleep) of LIN hardware changes.
 */
struct VECTOR_BLF_EXPORT LinSleepModeEvent final : ObjectHeader {
    LinSleepModeEvent();

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
     * This value indicates the reason for an event.
     * Following values are possible:
     *   - 0: Start state
     *
     * Transition to Sleep mode
     *   - 1: Go-to-Sleep frame
     *   - 2: Bus Idle Timeout
     *   - 3: Silent SleepMode command (for shortening
     *     the BusIdle Timeout)
     *
     * Leaving Sleep mode:
     *   - 9: External Wakeup signal
     *   - 10: Internal Wakeup signal
     *   - 11: Bus traffic (can only occur if the LIN
     *     hardware does not have a Master function)
     *
     * LIN hardware does not go into Sleep mode in
     * spite of request to do so:
     *   - 18: Bus traffic (can only occur if the LIN
     *     hardware does not have a Master function)
     */
    uint8_t reason {};

    /** Bit values for flags */
    enum Flags : uint8_t {
        /** @brief LIN "was awake" */
        WasAwake = 0x01,

        /** @brief LIN "is awake" */
        IsAwake = 0x02,

        /** @brief LIN "external" */
        External = 0x04
    };

    /**
     * Bit mask with bit values as following:
     *
     * Bit 0 (LSB): Indicates the state of the LIN
     * hardware before this event occurred:
     *   - 1: Awake
     *   - 0: Asleep
     *
     * Bit 1: Indicates the current state of the LIN
     * hardware:
     *   - 1: Awake
     *   - 0: Asleep
     *
     * Bit 2: Indicates whether this event caused by
     * external or internal event:
     *   - 1: External event
     *   - 0: Internal event
     */
    uint8_t flags {};

    /** reserved */
    uint32_t reservedLinSleepModeEvent {};
};

}
}

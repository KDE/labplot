// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinBusEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LIN_SPIKE_EVENT2
 *
 * This event occurs when a short (normally less than 1 bit time) dominant signal has
 * been detected on a LIN channel.
 */
struct VECTOR_BLF_EXPORT LinSpikeEvent2 final : ObjectHeader, LinBusEvent {
    LinSpikeEvent2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief the spike's width in microseconds
     *
     * Spike length in microseconds
     */
    uint32_t width {};

    /**
     * Flag indicating whether this event is a simulated
     * one:
     *   - 0: real event
     *   - 1: simulated event
     */
    uint8_t internal {};

    /** reserved */
    uint8_t reservedLinSpikeEvent1 {};

    /** reserved */
    uint16_t reservedLinSpikeEvent2 {};
};

}
}

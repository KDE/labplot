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
 * @brief LIN_SPIKE_EVENT
 *
 * @deprecated
 *
 * This event occurs when a short (normally less than 1 bit time) dominant signal has
 * been detected on a LIN channel.
 */
struct VECTOR_BLF_EXPORT LinSpikeEvent final : ObjectHeader {
    LinSpikeEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel number where the frame sent/received.
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedLinSpikeEvent {};

    /**
     * @brief the spike's width
     *
     * Spike length in microseconds
     */
    uint32_t width {};
};

}
}

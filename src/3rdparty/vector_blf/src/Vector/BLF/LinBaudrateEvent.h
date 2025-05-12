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
 * @brief LIN_BAUDRATE
 *
 * This info event is sent by the LIN hardware at the start of a measurement and
 * whenever the baud rate changes by more than 0.5 % during a measurement. If this info event
 * occurs, then the LIN hardware is synchronized with the baud rate of the external Master.
 */
struct VECTOR_BLF_EXPORT LinBaudrateEvent final : ObjectHeader {
    LinBaudrateEvent();

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
    uint16_t reservedLinBaudrateEvent {};

    /**
     * Measured baud rate [in bits/sec]
     */
    int32_t baudrate {};
};

}
}

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
 * @brief LIN_STATISTIC
 *
 * @deprecated
 *
 * This info event is sent by the LIN hardware and transports bus statistics.
 */
struct VECTOR_BLF_EXPORT LinStatisticEvent final : ObjectHeader {
    LinStatisticEvent();

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
    uint16_t reservedLinStatisticEvent1 {};

    /** reserved */
    uint32_t reservedLinStatisticEvent2 {};

    /**
     * @brief bus load
     *
     * Bus load in percents
     */
    double busLoad {};

    /**
     * @brief bursts total
     *
     * Total number of bursts
     */
    uint32_t burstsTotal {};

    /**
     * @brief bursts overrun
     *
     * Number of overrun bursts
     */
    uint32_t burstsOverrun {};

    /**
     * @brief frames sent
     *
     * Number of transmitted frames
     */
    uint32_t framesSent {};

    /**
     * @brief frames received
     *
     * Number of received frames
     */
    uint32_t framesReceived {};

    /**
     * @brief frames unanswered
     *
     * Number of frames without response
     */
    uint32_t framesUnanswered {};

    /** reserved */
    uint32_t reservedLinStatisticEvent3 {};
};

}
}

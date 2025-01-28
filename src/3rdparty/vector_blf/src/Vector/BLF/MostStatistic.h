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
 * @brief MOST_STATISTIC
 *
 * The event transports common network statistics. Usually the event is not visible in a
 * trace window.
 */
struct VECTOR_BLF_EXPORT MostStatistic final : ObjectHeader {
    MostStatistic();

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
     * Number of messages on Asynchronous channel
     * since the last Statistic event
     */
    uint16_t pktCnt {};

    /**
     * Number of messages on Control channel since
     * the last Statistic event
     */
    int32_t frmCnt {};

    /**
     * Number of signal stat transitions since the last
     * Statistic event
     */
    int32_t lightCnt {};

    /**
     * Receive buffer level of Optolyzer G1 in spy
     * mode
     */
    int32_t bufferLevel {};
};

}
}

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
 * @brief MOST_DATALOST
 *
 * Indicates loss of data. (Number of lost messages and start and end time stamp of data
 * loss.)
 */
struct VECTOR_BLF_EXPORT MostDataLost final : ObjectHeader2 {
    MostDataLost();

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
    uint16_t reservedMostDataLost {};

    /**
     * @brief info about data loss
     *
     * Data loss information
     *   - Bit0: 1: data loss on control channel (spy)
     *   - Bit1: 1: data loss on control channel (node)
     *   - Bit2: 1: data loss on asynchronous channel
     *     (spy)
     *   - Bit3: 1: data loss on asynchronous channel
     *     (node)
     *   - Bit4: 1: data loss on synchronous channel
     *   - Bit5: 1: data loss since driver queue full
     */
    uint32_t info {};

    /**
     * Number of lost messages on Control channel
     */
    uint32_t lostMsgsCtrl {};

    /**
     * Number of lost messages on Packet Data
     * Channel channel
     */
    uint32_t lostMsgsAsync {};

    /**
     * Absolute time in nano-seconds
     */
    uint64_t lastGoodTimeStampNs {};

    /**
     * Absolute time in nano-seconds
     */
    uint64_t nextGoodTimeStampNs {};
};

}
}

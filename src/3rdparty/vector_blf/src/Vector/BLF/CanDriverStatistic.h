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
 * @brief CAN_STATISTIC
 *
 * CAN driver statistic data for a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanDriverStatistic final : ObjectHeader {
    CanDriverStatistic();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * CAN channel the statistic data belongs to.
     */
    uint16_t channel {};

    /**
     * @brief CAN bus load
     *
     * Busload in 1/100 percent (e.g. 100 means
     * 1%)
     */
    uint16_t busLoad {};

    /**
     * @brief standard CAN id data frames
     *
     * Number of standard data frames sent on
     * that channel.
     */
    uint32_t standardDataFrames {};

    /**
     * @brief extended CAN id data frames
     *
     * Number of extended data frames sent on
     * that channel.
     */
    uint32_t extendedDataFrames {};

    /**
     * @brief standard CAN id remote frames
     *
     * Number of remote data frames sent on that
     * channel.
     */
    uint32_t standardRemoteFrames {};

    /**
     * @brief extented CAN id remote frames
     *
     * Number of extended remote data frames
     * sent on that channel.
     */
    uint32_t extendedRemoteFrames {};

    /**
     * @brief CAN error frames
     *
     * Number of error frams sent on that channel
     */
    uint32_t errorFrames {};

    /**
     * @brief CAN overload frames
     *
     * Number of overload frams sent on that
     * channel.
     */
    uint32_t overloadFrames {};

    /** reserved */
    uint32_t reservedCanDriverStatistic {};
};

}
}

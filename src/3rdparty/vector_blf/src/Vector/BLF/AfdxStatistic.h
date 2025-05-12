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
 * @brief AFDX_STATISTIC
 *
 * AFDX statistic event per virtual link.
 * AFDX combined bus- and VL- statistic event; used before 8.2
 */
struct VECTOR_BLF_EXPORT AfdxStatistic final : ObjectHeader {
    AfdxStatistic();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * The channel of the frame.
     */
    uint16_t channel {};

    /** enumeration for flags */
    enum Flags : uint16_t {
        /** channel is configured */
        ChannelIsConfigured = 0x0001,

        /** HW related counters valid */
        HwRelatedCountersValid = 0x0002,

        /** CANwin related counters are valid */
        CanWinRelatedCountersAreValid = 0x0004,

        /** link-related info is valid */
        LinkRelatedInfoIsValud = 0x0008,

        /** invalid packet counter is valid */
        InvalidPacketCounterIsValid = 0x0010,

        /** lost packet counter is valid */
        LostPacketCounterIsValud = 0x0020,

        /** dropped packet counter is valid */
        DroppedPacketCounterIsValid = 0x0040,

        /** byte counters are based on CANwin packets, not HW */
        ByteCountersAreBasedOnCanWinPackets = 0x0080
    };

    /** flags */
    uint16_t flags {};

    /**
     * Number of Rx packets since last
     * statistic event.
     */
    uint32_t rxPacketCount {};

    /**
     * Number of Rx bytes since last
     * statistic event.
     */
    uint32_t rxByteCount {};

    /**
     * Number of Tx packets since last
     * statistic event.
     */
    uint32_t txPacketCount {};

    /**
     * Number of Tx bytes since last
     * statistic event.
     */
    uint32_t txByteCount {};

    /**
     * Number of collisions since last
     * statistic event.
     */
    uint32_t collisionCount {};

    /**
     * Number of errors since last statistic
     * event.
     */
    uint32_t errorCount {};

    /**
     * Number of dropped packet due to
     * redundancy check since last
     * statistic event.
     */
    uint32_t statDroppedRedundantPacketCount {};

    /**
     * Number of errors found at
     * redundancy check since last
     * statistic event.
     */
    uint32_t statRedundantErrorPacketCount {};

    /**
     * Number of errors found at integrity
     * check since last statistic event.
     */
    uint32_t statIntegrityErrorPacketCount {};

    /**
     * Average period of frames on this
     * VL in [msec].
     */
    uint32_t statAvrgPeriodMsec {};

    /**
     * Average jitter of the time period of
     * frames on this VL in [mysec].
     */
    uint32_t statAvrgJitterMysec {};

    /**
     * Unique ID assigned to this VL.
     */
    uint32_t vlid {};

    /**
     * Time period covered by this event
     * in [msec].
     */
    uint32_t statDuration {};
};

}
}

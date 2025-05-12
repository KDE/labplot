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
 * @brief WLAN_STATISTIC
 *
 * WLAN statistic.
 */
struct VECTOR_BLF_EXPORT WlanStatistic final : ObjectHeader {
    WlanStatistic();

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
        /** Valid Rx/Tx Counter */
        ValidRxTxCounter = 0x01,

        /** Valid Error Counter */
        ValidErrorCounter = 0x02
    };

    /** flags */
    uint16_t flags {};

    /**
     * Number of Rx packets since last statistic event.
     */
    uint32_t rxPacketCount {};

    /**
     * Number of Rx bytes since last statistic event.
     */
    uint32_t rxByteCount {};

    /**
     * Number of Tx packets since last statistic event.
     */
    uint32_t txPacketCount {};

    /**
     * Number of Tx bytes since last statistic event.
     */
    uint32_t txByteCount {};

    /**
     * Number of collisions since last statistic event.
     */
    uint32_t collisionCount {};

    /**
     * Number of errors since last statistic event.
     */
    uint32_t errorCount {};

    /** reserved */
    uint32_t reservedWlanStatistic {};
};

}
}

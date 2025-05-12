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
 * @brief CAN_DRIVER_SYNC
 *
 * Event that occurs when hardware sync is executed.
 */
struct VECTOR_BLF_EXPORT CanDriverHwSync final : ObjectHeader {
    CanDriverHwSync();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief channel where sync occured
     *
     * Application channel
     */
    uint16_t channel {};

    /** enumeration for flags */
    enum Flags : uint8_t {
        /** sync was sent from this channel */
        Tx = 1,

        /** external sync received */
        Rx = 2,

        /** sync received but generated from this hardware */
        RxThis = 4
    };

    /** flags */
    uint8_t flags {};

    /** reserved */
    uint8_t reservedCanDriverHwSync1 {};

    /** reserved */
    uint32_t reservedCanDriverHwSync2 {};
};

}
}

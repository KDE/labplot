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
 * @brief OVERRUN_ERROR
 */
struct VECTOR_BLF_EXPORT DriverOverrun final : ObjectHeader {
    DriverOverrun();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for busType */
    enum BusType : uint32_t {
        /** CAN */
        Can = 1,

        /** LIN */
        Lin = 5,

        /** MOST */
        Most = 6,

        /** FlexRay */
        FlexRay = 7,

        /** J1708 */
        J1708 = 9,

        /** Ethernet */
        Ethernet = 10,

        /** WLAN */
        Wlan = 13,

        /** AFDX */
        Afdx = 14
    };

    /**
     * @brief bus type
     */
    uint32_t busType {};

    /**
     * @brief channel where overrun occured
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedDriverOverrun {};
};

}
}

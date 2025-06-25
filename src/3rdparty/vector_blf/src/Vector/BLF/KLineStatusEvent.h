// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief KLINE_STATUSEVENT
 */
struct VECTOR_BLF_EXPORT KLineStatusEvent final : ObjectHeader {
    KLineStatusEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for type */
    enum Type : uint16_t {
        /** If set in type, direction is tester -> ECU */
        toEcu = 0x8000,

        /** Use this mask to filter out the type from type */
        mask = 0x7FFF
    };

    /**
     * @brief BusSystemFacility::VKLineStatusEventType
     */
    uint16_t type {};

    /**
     * @brief number of *bytes* used in data
     */
    uint16_t dataLen {};

    /**
     * @brief channel of event
     */
    uint32_t port {};

    /** reserved */
    uint64_t reservedKLineStatusEvent {};

    /**
     * @brief the actual data, but only dataLen BYTES are used!
     */
    std::array<uint64_t, 3> data {};
};

}
}

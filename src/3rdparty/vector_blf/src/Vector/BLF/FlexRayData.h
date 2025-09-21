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
 * @brief FLEXRAY_DATA
 *
 * @deprecated
 */
struct VECTOR_BLF_EXPORT FlexRayData final : ObjectHeader {
    FlexRayData();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    uint16_t channel {};

    /** multiplexer */
    uint8_t mux {};

    /** length */
    uint8_t len {};

    /** message id */
    uint16_t messageId {};

    /** CRC */
    uint16_t crc {};

    /**
     * @brief dir
     */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedFlexRayData1 {};

    /** reserved */
    uint16_t reservedFlexRayData2 {};

    /** data bytes */
    std::array<uint8_t, 12> dataBytes {};
};

}
}

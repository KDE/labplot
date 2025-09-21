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
 * @brief J1708_MESSAGE, J1708_VIRTUAL_MSG
 *
 * J1708 message object
 */
struct VECTOR_BLF_EXPORT J1708Message final : ObjectHeader {
    J1708Message();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    uint16_t channel {};

    /**
     * @brief direction
     */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedJ1708Message1 {};

    /**
     * @brief error code
     */
    uint16_t error {};

    /**
     * @brief data size
     */
    uint8_t size {};

    /**
     * @brief data
     */
    std::array<uint8_t, 255> data {};

    /** reserved */
    uint16_t reservedJ1708Message2 {};
};

}
}

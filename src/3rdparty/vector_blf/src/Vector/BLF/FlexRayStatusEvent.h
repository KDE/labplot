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
 * @brief FLEXRAY_STATUS
 *
 * @deprecated
 */
struct VECTOR_BLF_EXPORT FlexRayStatusEvent final : ObjectHeader {
    FlexRayStatusEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    uint16_t channel {};

    /**
     * @brief object version
     */
    uint16_t version {};

    /**
     * @brief type of status event
     */
    uint16_t statusType {};

    /**
     * @brief additional info 1
     */
    uint16_t infoMask1 {};

    /**
     * @brief additional info 2
     */
    uint16_t infoMask2 {};

    /**
     * @brief additional info 3
     */
    uint16_t infoMask3 {};

    /** reserved */
    std::array<uint16_t, 18> reservedFlexRayStatusEvent {};
};

}
}

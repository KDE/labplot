// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/AfdxLineStatus.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief AFDX_STATUS
 *
 * AFDX adapter status event, available since 8.2
 */
struct VECTOR_BLF_EXPORT AfdxStatus final : ObjectHeader {
    AfdxStatus();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedAfdxStatus1 {};

    /**
     * @brief status of adapter lineA
     */
    AfdxLineStatus statusA {};

    /**
     * @brief status of adapter lineB
     */
    AfdxLineStatus statusB {};

    /** reserved */
    uint32_t reservedAfdxStatus2 {};
};

}
}

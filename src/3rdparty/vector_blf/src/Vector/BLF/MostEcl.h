// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_ECL
 *
 * State change of the MOST Electrical Control Line.
 */
struct VECTOR_BLF_EXPORT MostEcl final : ObjectHeader2 {
    MostEcl();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Application channel
     */
    uint16_t channel {};

    /**
     * - 0 – discrete
     * - 1 – sequence
     */
    uint16_t mode {};

    /**
     * @brief Electrical Control Line level
     *
     * mMode = 0:
     *   - 0 – line low
     *   - 1 – line high
     * mMode = 1:
     *   - 0 – sequence stopped
     *   - 1 – sequence started
     */
    uint16_t eclState {};

    /** reserved */
    uint16_t reservedMostEcl {};
};

}
}

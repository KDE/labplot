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
 * @brief MOST_TXLIGHT
 *
 * Optical physical layer: Information about light output of the Fiber Optical Transmitter
 *
 * Electrical physical layer: Signal output state
 */
struct VECTOR_BLF_EXPORT MostTxLight final : ObjectHeader2 {
    MostTxLight();

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
     * - 0 – TxLight/Signal off
     * - 1 – TxLight/Signal enabled
     * - 2 – TxLight/Signal forced on
     */
    uint16_t state {};

    /** reserved */
    uint32_t reservedMostTxLight {};
};

}
}

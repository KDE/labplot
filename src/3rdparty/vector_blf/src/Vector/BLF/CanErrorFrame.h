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
 * @brief CAN_ERROR
 *
 * CAN error frame received or transmitted on a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanErrorFrame final : ObjectHeader {
    CanErrorFrame();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel the frame was sent or received.
     */
    uint16_t channel {};

    /**
     * @brief CAN error frame length
     *
     * Length of error frame - can be left 0.
     */
    uint16_t length {};

    /** reserved */
    uint32_t reservedCanErrorFrame {};
};

}
}

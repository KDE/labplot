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
 * @brief LIN_WAKEUP
 *
 * @deprecated
 *
 * LIN Wakeup-Frame received or transmitted on a LIN channel.
 */
struct VECTOR_BLF_EXPORT LinWakeupEvent final : ObjectHeader {
    LinWakeupEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel number where the frame sent/received.
     */
    uint16_t channel {};

    /**
     * Byte value used by wakeup frame.
     */
    uint8_t signal {};

    /**
     * Flag indicating whether the wakeup frame has
     * been transmitted by an external device (selector
     * set) or by the LIN hardware itself (selector not
     * set).
     */
    uint8_t external {};

    /** reserved */
    uint32_t reservedLinWakeupEvent {};
};

}
}

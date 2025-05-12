// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinBusEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LIN_UNEXPECTED_WAKEUP
 *
 * This event occurs if an unexpected byte received in bus idle phase of wake mode
 * could be a wakeup frame
 */
struct VECTOR_BLF_EXPORT LinUnexpectedWakeup final : ObjectHeader, LinBusEvent {
    LinUnexpectedWakeup();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief width of the unexpected wakeup in nanoseconds (valid for LIN 2.x)
     *
     * Width of the unexpected wakeup in nanoseconds.
     * Valid for LIN 2.x
     */
    uint64_t width {};

    /**
     * @brief byte signal of the unexpected wakeup (valid for LIN 1.x)
     *
     * Byte signal of the unexpected wakeup. Valid for
     * LIN 1.x
     */
    uint8_t signal {};

    /** reserved */
    uint8_t reservedLinUnexpectedWakeup1 {};

    /** reserved */
    uint16_t reservedLinUnexpectedWakeup2 {};

    /** reserved */
    uint32_t reservedLinUnexpectedWakeup3 {};
};

}
}

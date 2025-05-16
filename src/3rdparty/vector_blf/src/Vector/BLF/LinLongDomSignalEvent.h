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
 * @brief LIN_LONG_DOM_SIG
 *
 * @deprecated
 *
 * This event occurs when a LIN channel remains in the dominant state for a time, which
 * is longer than a valid wakeup frame and it is not a valid sync break.
 */
struct VECTOR_BLF_EXPORT LinLongDomSignalEvent final : ObjectHeader, LinBusEvent {
    LinLongDomSignalEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * One dominant signal can be reported with
     * multiple events. This field indicate the order of
     * this event in a sequence:
     *   - 0: Signal just detected
     *   - 1: Signal continuation
     *   - 2: Signal finished
     */
    uint8_t type {};

    /** reserved */
    uint8_t reservedLinLongDomSignalEvent1 {};

    /** reserved */
    uint16_t reservedLinLongDomSignalEvent2 {};

    /** reserved */
    uint32_t reservedLinLongDomSignalEvent3 {};
};

}
}

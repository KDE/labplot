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
 * @brief MOST_HWMODE
 *
 * This event is fired when one or more HW state changes. HW states are the AllBypass
 * bit (e.g. ABY of OS8104), the Master/Slave selection (e.g. MTR of OS8104), the Control spy and
 * the Asynchronous spy. The event transports all states even if only a single state has changed.
 * hwModeMask denotes which state differs regarding to the previous HW mode event.
 */
struct VECTOR_BLF_EXPORT MostHwMode final : ObjectHeader2 {
    MostHwMode();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Application channel
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedMostHwMode {};

    /**
     * @brief bypass/master/slave/spy
     *
     * - Bit 0x01: Bypass: 0: open; 1: active
     * - Bit 0x02: Timing mode: 0: slave; 1: master
     * - Bit 0x04: Master mode: 0: static master; 1: nonstatic
     *   master
     * - Bit 0x08: 0: Ethernet Spy active: 1: blocks
     *   "Ethernet Spy over MOST" channel
     * - Bit 0x10: Control channel spy: 1: active
     * - Bit 0x20: Async. channel spy: 1: active
     * - Bit 0x40: 1: no "Ethernet over MOST" events
     *   (MOST150)
     * - Bit 0x80: 1: no events from async. channel
     */
    uint16_t hwMode {};

    /**
     * @brief marks the altered bits
     *
     * Bitmask of changed bits
     */
    uint16_t hwModeMask {};
};

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinMessageDescriptor.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LIN_SND_ERROR2
 *
 * This event occurs when no Slave responds to a frame header from Master.
 */
struct VECTOR_BLF_EXPORT LinSendError2 final : ObjectHeader, LinMessageDescriptor {
    LinSendError2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief EndOfHeader timestamp
     *
     * End of header timestamp [in nanosecond]
     */
    uint64_t eoh {};

    /**
     * @brief Event-triggered frame
     *
     * Flag indicating whether this frame is Event-
     * Triggered one:
     *   - 0: not ETF
     *   - 1: ETF
     */
    uint8_t isEtf {};

    /**
     * Slave Identifier in the Final State
     * Machine (obsolete)
     */
    uint8_t fsmId {};

    /**
     * State Identifier of a Slave in the Final State
     * Machine (obsolete)
     */
    uint8_t fsmState {};

    /** reserved */
    uint8_t reservedLinSendError1 {};

    /* the following variables are only available in Version 2 and above */

    /** reserved */
    uint32_t reservedLinSendError2 {};

    /* the following variables are only available in Version 3 and above */

    /**
     * @brief Exact baudrate of the header in bit/sec
     *
     * Event's baudrate measured in header
     * [in bits/sec]
     */
    double exactHeaderBaudrate {};

    /**
     * @brief Early stop bit offset for UART timestamps in ns
     *
     * Early stop bit offset in frame header
     * for UART timestamps [in ns]
     */
    uint32_t earlyStopbitOffset {};

    /** reserved */
    bool reservedLinSendError3_present{};
    uint32_t reservedLinSendError3 {};
};

}
}

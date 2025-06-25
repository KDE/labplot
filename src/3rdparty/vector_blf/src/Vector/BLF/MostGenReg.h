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
 * @brief MOST_GENREG
 *
 * This event reports the change of an important transceiver register like node position or
 * node address. It can occur spontaneous or as result of a read/write operation. Directly after
 * measurement start the current values of the special registers are reported even if they have not
 * changed.
 */
struct VECTOR_BLF_EXPORT MostGenReg final : ObjectHeader2 {
    MostGenReg();

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
     * @brief read/write request/result
     *
     * Operation type of a register event.
     *   - Unspecified = 0: unspecified (or HW does not support sub types)
     *   - Notify = 1: notification on register change (spontaneous)
     *   - ReadRequest = 2: request of a register read operation
     *   - WriteRequest = 3: request of a register write operation
     *   - ReadResult = 4: result of a register read operation
     *   - WriteResult = 5: result of a register write operation
     *   - ReadFailed = 6: register read operation failed
     *   - WriteFailed = 7: register write operation failed
     */
    uint8_t subType {};

    /** reserved */
    uint8_t reservedMostGenReg1 {};

    /**
     * @brief operation handle
     *
     * Operation handle (obsolete; write 0)
     */
    uint32_t handle {};

    /**
     * @brief register ID
     *
     * IDs for identifying registers.
     *   - Empty = 0x0
     *   - NPR = 0x87: 8 bit Node Position register
     *   - GA = 0x89: 8 bit Group (Group Address = 0x0300 + Group)
     *   - NA = 0x8A: 16 bit Node Address
     *   - NDR = 0x8F: 8 bit Node Delay (MOST25 only)
     *   - MPR = 0x90: 8 bit Number of nodes with open bypass
     *   - MDR = 0x91: 8 bit Maximum Delay (MOST25 only)
     *   - SBC = 0x96: 8 bit Synchronous Bandwidth Control (SBC)
     *     - Synchronous Bandwidth = 4 x SBC
     *     - Maximum values for SBC:
     *       - MOST25: 15 (= 60 Bytes)
     *       - MOST50: 29 (= 116 Bytes)
     *       - MOST150: 93 (= 372 Bytes)
     *   - APA = 0xE8: 16 bit Alternate Packet Address (MOST25 only)
     *   - XTIM = 0xBE: 8 bit Transmit Retry Time
     *   - XRTY = 0xBF: 8 bit Number of send attempts
     *   - MacAdr = 0xFE: 48 bit MAC address (EUI-48, conforming to the IEEE standard;
     *     MOST150 only)
     */
    uint16_t regId {};

    /** reserved */
    uint16_t reservedMostGenReg2 {};

    /** reserved */
    uint32_t reservedMostGenReg3 {};

    /**
     * @brief register value
     *
     * Register value
     */
    uint64_t regValue {};
};

}
}

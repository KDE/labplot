// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_REG
 *
 * This event transports a register read or write result (e.g. reading the routing engine of
 * the OS8104). Unlike the special register event (MostGenReg) this event does not occur
 * spontaneous.
 */
struct VECTOR_BLF_EXPORT MostReg final : ObjectHeader2 {
    MostReg();

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
    uint8_t reservedMostReg {};

    /**
     * @brief operation handle
     *
     * Operation handle (obsolete; write 0)
     */
    uint32_t handle {};

    /**
     * @brief start address
     *
     * Register address offset
     */
    uint32_t offset {};

    /**
     * @brief chip id
     *
     * ID of chip
     *   - 1 – OS8104
     */
    uint16_t chip {};

    /**
     * @brief number of bytes
     *
     * Number of valid bytes in regData
     */
    uint16_t regDataLen {};

    /**
     * @brief data bytes
     *
     * Register data
     */
    std::array<uint8_t, 16> regData {};
};

}
}

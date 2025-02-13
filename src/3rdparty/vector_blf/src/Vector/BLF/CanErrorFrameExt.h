// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief CAN_ERROR_EXT
 *
 * Extended CAN error frame received or transmitted on a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanErrorFrameExt final : ObjectHeader {
    CanErrorFrameExt();

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
     * Length of error frame, unused, may be 0.
     */
    uint16_t length {};

    /**
     * @brief extended CAN error frame flags
     *
     * Defines what additional information is valid.
     * Following values are possible:
     *
     * - 1: SJA 1000 ECC is valid (member ecc)
     * - 2: Vector CAN Core Error Code is valid.
     * - 4: Vector CAN Core Error Position
     * - 8: Vector CAN Core Frame Length in ns
     */
    uint32_t flags {};

    /**
     * @brief error control code
     *
     * Content of Philips SJA1000 Error Code Capture
     * (ECC) register, or the Vector CAN-Core error
     * register (see also flags).
     *
     * SJA1000-ECC
     *
     * See documentation of Philips SJA1000 CAN
     * Controller.
     *
     * Vector CAN-Core
     *
     * - Bit 0-5 Meaning:
     *   - 0: Bit Error
     *   - 1: Form Error
     *   - 2: Stuff Error
     *   - 3: Other Error
     *   - 4: CRC Error
     *   - 5: Ack-Del-Error
     * - Bit 6-7 Meaning:
     *   - 0: RX-NAK-Error
     *   - 1: TK-NAK-Error
     *   - 2: RX-Error
     *   - 3: TX-Error
     */
    uint8_t ecc {};

    /**
     * @brief error position
     *
     * Bit position of the error frame in the corrupted
     * message.
     */
    uint8_t position {};

    /**
     * @brief lower 4 bits: DLC from CAN-Core. Upper 4 bits: reserved
     *
     * Data length code of the corrupted message.
     */
    uint8_t dlc {};

    /** reserved */
    uint8_t reservedCanErrorFrameExt1 {};

    /**
     * @brief frame length in ns
     *
     * Length of the error frame in nanoseconds (time
     * difference between Start Of Frame and End Of
     * Frame)
     */
    uint32_t frameLengthInNs {};

    /**
     * @brief frame ID from CAN-Core
     *
     * Message ID of the corrupted message.
     */
    uint32_t id {};

    /**
     * @brief extended error flags
     *
     * Extended error flags.
     *
     * - Bit 0-4: Segment (only SJA1000)
     * - Bit 5: Direction, 1=RX
     * - Bit 6-11: Error Code
     *   - 0: Bit Error
     *   - 1: Form Error
     *   - 2: Stuff Error
     *   - 3: Other Error
     *   - 4: CRC Error
     *   - 5: ACK-DEL Error
     * - Bit 12-13: Extended Direction
     *   - 0: RX NAK
     *   - 1: TX NAK
     *   - 2: RX
     *   - 3: TX
     * - Bit 14: 1=The error frame was send from the application
     */
    uint16_t flagsExt {};

    /** reserved */
    uint16_t reservedCanErrorFrameExt2 {};

    /**
     * @brief Payload, only for CAN-Core
     *
     * Message data.
     */
    std::vector<uint8_t> data {};
};

}
}

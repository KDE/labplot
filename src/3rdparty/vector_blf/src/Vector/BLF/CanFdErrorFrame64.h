// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/CanFdExtFrameData.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief CAN_FD_ERROR_64
 *
 * CAN-FD error frame received or transmitted on a CAN-FD channel.
 */
struct VECTOR_BLF_EXPORT CanFdErrorFrame64 final : ObjectHeader, CanFdExtFrameData {
    CanFdErrorFrame64();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    virtual bool hasExtData() const;

    /**
     * @brief application channel
     *
     * Channel the frame was sent or received.
     */
    uint8_t channel {};

    /**
     * @brief CAN dlc
     *
     * Data length code of the corrupted message.
     */
    uint8_t dlc {};

    /**
     * @brief Valid payload length of data
     *
     * Number of data bytes of the corrupted message.
     */
    uint8_t validDataBytes {};

    /**
     * Content of Philips SJA1000 Error Code Capture
     * register, or the Vector CAN-Core error register.
     * See field ecc of CanErrorFrameExt.
     */
    uint8_t ecc {};

    /**
     * Defines what additional information is valid. See
     * field flags of CanErrorFrameExt.
     */
    uint16_t flags {};

    /**
     * Extended error flags. See field flagsExt of
     * CanErrorFrameExt.
     */
    uint16_t errorCodeExt {};

    /**
     * @brief FD specific flags
     *
     * CAN-FD specific flags.
     *
     * - Bit 0-3 Meaning:
     *   - unused
     * - Bit 0-4 Meaning:
     *   - 0: Error in Arbitration Phase
     *   - 1: Error in Data Phase
     * - Bit 5 Meaning:
     *   - 0: ESI is 0
     *   - 1: ESI is 1
     * - Bit 6 Meaning
     *   - 0: BRS is 0
     *   - 1: BRS is 1
     * - Bit 7 Meaning
     *   - 0: EDL is 0
     *   - 1: EDL is 1
     */
    uint16_t extFlags {};

    /** offset if extDataOffset is used */
    uint8_t extDataOffset {};

    /** resered */
    uint8_t reservedCanFdErrorFrame1 {};

    /**
     * @brief CAN ID
     *
     * Message ID of the corrupted message.
     */
    uint32_t id {};

    /**
     * @brief message length in ns
     *
     * Length of the error frame in nanoseconds (time
     * difference between Start Of Frame and End Of
     * Frame)
     *
     * without 3 inter frame space bits and by Rx-message also without 1 End-Of-Frame bit
     */
    uint32_t frameLength {};

    /**
     * @brief bit rate used in arbitration phase
     *
     * CAN-FD bit timing configuration for arbiration
     * phase, may be 0, if not supported by
     * hardware/driver
     *
     * - Bit 0-7: Quartz Frequency
     * - Bit 8-15: Prescaler
     * - Bit 16-23: BTL Cycles
     * - Bit 24-31: Sampling Point
     */
    uint32_t btrCfgArb {};

    /**
     * @brief bit rate used in data phase
     *
     * CAN-FD bit timing configuration for arbiration
     * phase, may be 0, if not supported by
     * hardware/driver. See btrCfgArb.
     */
    uint32_t btrCfgData {};

    /**
     * @brief time offset of brs field
     *
     * Time offset of bit rate switch within BRS field in
     * nanoseconds
     */
    uint32_t timeOffsetBrsNs {};

    /**
     * @brief time offset of brs field
     *
     * Time offset of bit rate switch within CRC
     * delimiter field in nanoseconds
     */
    uint32_t timeOffsetCrcDelNs {};

    /**
     * CRC checksum of corrupted message.
     */
    uint32_t crc {};

    /**
     * @brief error position as bit offset
     *
     * Bit position of the error frame in the corrupted
     * message.
     */
    uint16_t errorPosition {};

    /** reserved */
    uint16_t reservedCanFdErrorFrame2 {};

    /**
     * @brief CAN FD data
     *
     * CAN FD data bytes (the actual length may be
     * shorter than 64 bytes, according to the value of
     * dlc, e.g. when DLC is 12 data has length
     * 24)
     */
    std::vector<uint8_t> data {};

    /**
     * reserved
     *
     * @note This usually has 8 data bytes, when CanFdExtFrameData is not used.
     */
    std::vector<uint8_t> reservedCanFdErrorFrame64 {};
};

}
}

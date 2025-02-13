// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief FR_ERROR
 *
 * FlexRay Error event transmitted by the FlexRay hardware.
 */
struct VECTOR_BLF_EXPORT FlexRayVFrError final : ObjectHeader {
    FlexRayVFrError();

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
     * @brief object version
     *
     * Object version, for internal use
     */
    uint16_t version {};

    /**
     * @brief channel mask
     *
     * Channel Mask
     *   - 0 = Reserved or invalid
     *   - 1 = FlexRay Channel A
     *   - 2 = FlexRay Channel B
     *   - 3 = FlexRay Channels A and B
     */
    uint16_t channelMask {};

    /**
     * @brief current cycle
     *
     * Cycle number
     */
    uint8_t cycle {};

    /** reserved */
    uint8_t reservedFlexRayVFrError1 {};

    /**
     * @brief clientindex of send node
     *
     * Client index of send node. Must be set to 0 if file
     * is written from other applications
     */
    uint32_t clientIndexFlexRayVFrError {};

    /**
     * @brief number of cluster
     *
     * Number of cluster: channel number - 1
     */
    uint32_t clusterNo {};

    /**
     * @brief type of cc
     *
     * Type of communication controller
     *   - 0 = Architecture independent
     *   - 1 = Invalid CC type (for internal use only)
     *   - 2 = Cyclone I
     *   - 3 = BUSDOCTOR
     *   - 4 = Cyclone II
     *   - 5 = Vector VN interface
     *   - 6 = VN-Sync-Pulse (only in Status Event, for debugging purposes only)
     */
    uint32_t tag {};

    /**
     * @brief register flags
     *
     * Driver flags for internal usage
     *
     * CC-Type: Cyclone I
     *   - data[0]: Error flags from driver API
     *
     * CC-Type: Cyclone II
     *   - data[0]: Error packet flag:
     *     - 0 = No error
     *     - 1 = FlexCard overflow
     *     - 2 = PCO error mode changed
     *     - 3 = Sync frames below minimum
     *     - 4 = Sync frame overflow
     *     - 5 = Clock correction failure
     *     - 6 = Parity error
     *     - 7 = Receive FIFO overrun
     *     - 8 = Empty FIFO access
     *     - 9 = Illegal input buffer access
     *     - 10 = Illegal output buffer access
     *     - 11 = Syntax error
     *     - 12 = Content error
     *     - 13 = Slot boundary violation
     *     - 14 = Transmission across boundary
     *     - 15 = Latest transmit violation
     *   - data[1]: uint32_t layout depends on the error packet value (see previous row)
     *     - Error packet=2:
     *       - 0 = Unknown state
     *       - 1 = FlexRay protocol spec. > CONFIG
     *       - 2 = FlexRay protocol spec. > NORMAL_ACTIVE
     *       - 3 = FlexRay protocol spec. > NORMAL_PASSIVE
     *       - 4 = FlexRay protocol spec. > HALT
     *       - 5 = FlexRay protocol spec. > READY
     *       - 6 = FlexRay protocol spec. > STARTUP
     *       - 7 = FlexRay protocol spec. > WAKEUP
     *     - Error packet = 3 or 4:
     *       - Bits 0..3 > Sync frames even on channel A
     *       - Bits 4..7 > Sync frames even on channel B
     *       - Bits 8..11 > Sync frames odd on channel A
     *       - Bits 12..15 > Sync frames odd on channel B
     *     - Error packet = 5:
     *       - Bit 0 > Missing rate correction
     *       - Bit 1 > Rate correction limit reached
     *       - Bit 2 > Offset correction limit reached
     *       - Bit 3 > Missing offset correction
     *       - Bit 4..7 > Sync frames even on channel A
     *       - Bits 8..11 > Sync frames even on channel B
     *       - Bits 12..15 > Sync frames odd on channel A
     *       - Bits 16..19 > Sync frames odd on channel B
     *     - Error packet = 11..15:
     *       - LOW-WORD of mData[1] > Channel
     *       - HI-WORD of mData[1] > Slot count
     *
     * CC-Type: BUSDOCTOR
     *   - data[0]: Error flags from driver API
     *
     * CC-Type: VN-Interface
     *   - data[0]: Error tag:
     *     - 0 = FR_ERROR_POC_MODE
     *     - 1 = FR_ERROR_SYNC_FRAMES_BELOWMIN
     *     - 2 = FR_ERROR_SYNC_FRAMES_OVERLOAD
     *     - 3 = FR_ERROR_CLOCK_CORR_FAILURE
     *     - 4 = FR_ERROR_NIT_FAILURE
     *     - 5 = FR_ERROR_CC_ERROR
     *     - 6 = FR_ERROR_OVERFLOW
     *   - data[1] and data[2]: uint32_t layout depends on the error tag value (see previous row):
     *     - Error tag = 0:
     *       - Value 0: FR_ERROR_POC_ACTIVE
     *       - Value 1: FR_ERROR_POC_PASSIVE
     *       - Value 2: FR_ERROR_POC_COMM_HALT
     *     - Error tag = 1 or 2:
     *       - Bits 0..3: Sync frames even on channel A
     *       - Bits 4..7: Sync frames even on channel B
     *       - Bits 8..11: Sync frames odd on channel A
     *       - Bits 12..15: Sync frames odd on channel B
     *     - Error tag = 3:
     *       - Bit 0: Missing rate correction
     *       - Bit 1: Missing rate correction limit reached
     *       - Bit 2: Offset correction limit reached
     *       - Bit 3: Missing offset correction
     *       - Bits 4..19: Clock correction failed counter
     *       - Bits 20..23: Sync frames even on channel A
     *       - Bits 24..27: Sync frames even on channel B
     *       - Bits 28..31: Sync frames odd on channel A
     *       - Bits 32..35: Sync frames odd on channel B
     *     - Error tag = 4:
     *       - Value 1: FR_ERROR_NIT_SENA
     *       - Value 2: FR_ERROR_NIT_SBNA
     *       - Value 4: FR_ERROR_NIT_SENB
     *       - Value 8: FR_ERROR_NIT_SBNB
     *     - Error tag = 5:
     *       - Value 0x00000001: POC Error Mode Changed
     *       - Value 0x00000004: Sync Frames Below Minimum
     *       - Value 0x00000008: Sync Frame Overflow
     *       - Value 0x00000010: Clock Correction Failure
     *       - Value 0x00000040: Parity Error, data from MHDS (internal ERay
     *         error)
     *       - Value 0x00000200: Illegal Input Buffer Access (internal ERay error)
     *       - Value 0x00000400: Illegal Output Buffer Access (internal ERay error)
     *       - Value 0x00000800: Message Handler Constraints Flag data from
     *         MHDF (internal ERay error)
     *       - Value 0x00010000: Error Detection on channel A, data from ACS
     *       - Value 0x00020000: Latest Transmit Violation on channel A
     *       - Value 0x00040000: Transmit Across Boundary on Channel A
     *       - Value 0x01000000: Error Detection on channel B, data from ACS
     *       - Value 0x02000000: Latest Transmit Violation on channel B
     *       - Value 0x04000000: Transmit Across Boundary on Channel B
     */
    std::array<uint32_t, 4> data {};

    /** reserved */
    uint32_t reservedFlexRayVFrError2 {};
};

}
}

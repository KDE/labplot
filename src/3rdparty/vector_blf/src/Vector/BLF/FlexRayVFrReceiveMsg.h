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
 * @brief FR_RCVMESSAGE
 *
 * FlexRay message received or transmitted on FlexRay bus.
 */
struct VECTOR_BLF_EXPORT FlexRayVFrReceiveMsg final : ObjectHeader {
    FlexRayVFrReceiveMsg();

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
     * @brief version of data struct
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
     * @brief dir flag (tx, rx)
     *
     * Direction Flags
     *   - 0 = Rx
     *   - 1 = Tx
     *   - 2 = Tx Request
     *   - 3 and 4 are for internal use only.
     */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedFlexRayVFrReceiveMsg1 {};

    /**
     * @brief clientindex of send node
     *
     * Client index of send node. Must be set to 0 if file
     * is written from other applications.
     */
    uint32_t clientIndexFlexRayVFrReceiveMsg {};

    /**
     * @brief number of cluster
     *
     * Number of cluster: channel number - 1
     */
    uint32_t clusterNo {};

    /**
     * @brief slot identifier, word
     *
     * Slot identifier
     */
    uint16_t frameId {};

    /**
     * @brief header crc channel 1
     *
     * Header CRC FlexRay channel 1 (A)
     */
    uint16_t headerCrc1 {};

    /**
     * @brief header crc channel 2
     *
     * Header CRC FlexRay channel 2 (B)
     */
    uint16_t headerCrc2 {};

    /**
     * @brief byte count (not payload) of frame from CC receive buffer
     *
     * Payload length in bytes
     */
    uint16_t byteCount {};

    /**
     * @brief length of the data array (stretchy struct)
     *
     * Number of bytes of the payload stored in
     * dataBytes. If the CC-frame buffer was too
     * small to receive the complete payload, then
     * dataCount is smaller than byteCount.
     */
    uint16_t dataCount {};

    /**
     * @brief current cycle
     *
     * Cycle number
     */
    uint8_t cycle {};

    /** reserved */
    uint8_t reservedFlexRayVFrReceiveMsg2 {};

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
     * @brief frame flags
     *
     * Description of frame flags:
     * - Bit 0: 1 = Null frame.
     * - Bit 1: 1 = Data segment contains valid data
     * - Bit 2: 1 = Sync bit
     * - Bit 3: 1 = Startup flag
     * - Bit 4: 1 = Payload preamble bit
     * - Bit 5: 1 = Reserved bit
     * - Bit 6: 1 = Error flag (error frame or invalid frame)
     * - Bit 7: Reserved
     * - Bit 8: Internally used in CANoe/CANalyzer
     * - Bit 9: Internally used in CANoe/CANalyzer
     * - Bit 10: Internally used in CANoe/CANalyzer
     * - Bit 11: Internally used in CANoe/CANalyzer
     * - Bit 12: Internally used in CANoe/CANalyzer
     * - Bit 13: Internally used in CANoe/CANalyzer
     * - Bit 14: Internally used in CANoe/CANalyzer
     * - Bit 15: 1 = Async. monitoring has generated this event
     * - Bit 16: 1 = Event is a PDU
     * - Bit 17: Valid for PDUs only. The bit is set if the PDU is valid (either if the PDU has no update
     *   bit, or the update bit for the PDU was set in the received frame).
     * - Bit 18: Reserved
     * - Bit 19: 1 = Raw frame (only valid if PDUs are used in the configuration). A raw frame may
     *   contain PDUs in its payload
     * - Bit 20: 1 = Dynamic segment
     *   0 = Static segment
     * - Bit 21 This flag is only valid for frames and not for PDUs.
     *   1 = The PDUs in the payload of this frame are logged in separate logging entries.
     *   0 = The PDUs in the payload of this frame must be extracted out of this frame. The
     *   logging file does not contain separate PDU-entries.
     * - Bit 22 Valid for PDUs only. The bit is set if the PDU has an update bit
     *
     * The reserved bits and the bits which are for internally CANoe/CANalyzer usage must be ignored
     * from other applications. Other applications must set these bits to 0 when writing logging files.
     */
    uint32_t data {};

    /**
     * @brief frame flags
     *
     * Description of frame flags:
     * - Bit 0: 1 = Null frame.
     * - Bit 1: 1 = Data segment contains valid data
     * - Bit 2: 1 = Sync bit
     * - Bit 3: 1 = Startup flag
     * - Bit 4: 1 = Payload preamble bit
     * - Bit 5: 1 = Reserved bit
     * - Bit 6: 1 = Error flag (error frame or invalid frame)
     * - Bit 7: Reserved
     * - Bit 8: Internally used in CANoe/CANalyzer
     * - Bit 9: Internally used in CANoe/CANalyzer
     * - Bit 10: Internally used in CANoe/CANalyzer
     * - Bit 11: Internally used in CANoe/CANalyzer
     * - Bit 12: Internally used in CANoe/CANalyzer
     * - Bit 13: Internally used in CANoe/CANalyzer
     * - Bit 14: Internally used in CANoe/CANalyzer
     * - Bit 15: 1 = Async. monitoring has generated this event
     * - Bit 16: 1 = Event is a PDU
     * - Bit 17: Valid for PDUs only. The bit is set if the PDU is valid (either if the PDU has no update
     *   bit, or the update bit for the PDU was set in the received frame).
     * - Bit 18: Reserved
     * - Bit 19: 1 = Raw frame (only valid if PDUs are used in the configuration). A raw frame may
     *   contain PDUs in its payload
     * - Bit 20: 1 = Dynamic segment
     *   0 = Static segment
     * - Bit 21 This flag is only valid for frames and not for PDUs.
     *   1 = The PDUs in the payload of this frame are logged in separate logging entries.
     *   0 = The PDUs in the payload of this frame must be extracted out of this frame. The
     *   logging file does not contain separate PDU-entries.
     * - Bit 22 Valid for PDUs only. The bit is set if the PDU has an update bit
     *
     * The reserved bits and the bits which are for internally CANoe/CANalyzer usage must be ignored
     * from other applications. Other applications must set these bits to 0 when writing logging files.
     */
    uint32_t frameFlags {};

    /**
     * @brief TxRq, TxAck flags
     *
     * Not used, reserved
     */
    uint32_t appParameter {};

    /**
     * @brief array of databytes
     *
     * Payload
     */
    std::array<uint8_t, 254> dataBytes {};

    /** reserved */
    uint16_t reservedFlexRayVFrReceiveMsg3 {};

    /** reserved */
    uint32_t reservedFlexRayVFrReceiveMsg4 {};
};

}
}

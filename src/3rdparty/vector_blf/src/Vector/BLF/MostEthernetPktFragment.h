// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_ETHERNET_PKT_FRAGMENT
 *
 * Partial transmitted message on MOST Ethernet Packet Channel.
 */
struct VECTOR_BLF_EXPORT MostEthernetPktFragment final : ObjectHeader2 {
    MostEthernetPktFragment();

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
    uint8_t reservedMostEthernetPktFragment1 {};

    /**
     * @brief acknowledge code
     *
     * AckNack holds the transmit status of a control message (see Transmit Status Register of OS8104
     * for MOST25).
     *   - Bit 0:
     *     Meaning:
     *       - 1: no response (NoResp)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 1:
     *     Meaning:
     *       - 1: valid receipt (Valid)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 2:
     *     Meaning:
     *       - 1: CRC Error (CRCError)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 3:
     *     Meaning:
     *       - 1: receive buffer full (RxBufFull)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     *   - Bit 4:
     *     Meaning:
     *       - 1: acknowledged (Ack)
     *     Restriction:
     *       - only for Dir = Tx or spy messages (always
     *         set to 1 for Rx messages in node mode)
     *   - Bit 5:
     *     Meaning:
     *       - 1: negative acknowledge (NAck)
     *     Restriction:
     *       - only for Dir = Tx or spy messages
     */
    uint8_t ackNack {};

    /**
     * @brief bitfield indicating which members have valid data
     *
     * If a bit in the mask is set, the corresponding data field has been seen on the bus.
     *   - Bit 0: 0x00000001: dataLenAnnounced
     *   - Bit 1: 0x00000002: sourceAdr
     *   - Bit 2: 0x00000004: destAdr
     *   - Bit 3: 0x00000008: ackNack
     *   - Bit 4: 0x00000010: pAck
     *   - Bit 5: 0x00000020: pIndex
     *   - Bit 6: 0x00000040: piority
     *   - Bit 7: 0x00000080: crc
     *   - Bit 8: 0x00000100: cAck
     *   - Bit 9: 0x00000200: sourceMacAdr
     *   - Bit 10: 0x00000400: destMacAdr
     *   - Bit 31: 0x80000000: 0: MOST150 fragment; 1: MOST50 fragment
     */
    uint32_t validMask {};

    /**
     * @brief 48 bit source address
     *
     * 48 bit source address
     */
    uint64_t sourceMacAdr {};

    /**
     * @brief 48 bit target address
     *
     * 48 bit target address
     */
    uint64_t destMacAdr {};

    /**
     * @brief a preemptive acknowledge code
     *
     * Pre-emptive acknowledge code
     *   - 0x00: No Response
     *   - 0x01: Buffer full
     *   - 0x04: OK
     */
    uint8_t pAck {};

    /**
     * @brief CRC acknowledge from the packet receiver(s) to the packet transmitter
     *
     * CRC acknowledge from the packet receiver(s) to
     * the packet transmitter
     *   - 0x00: No Response
     *   - 0x01: CRC error
     *   - 0x04: OK
     */
    uint8_t cAck {};

    /** reserved */
    uint16_t reservedMostEthernetPktFragment2 {};

    /**
     * @brief Cyclic Redundancy Check
     *
     * Cyclic Redundancy Check
     */
    uint32_t crc {};

    /**
     * @brief number of transmitted user data bytes
     *
     * Number of transmitted user data bytes. These
     * bytes were counted on bus.
     *
     * Note: The number of bytes saved in this event is
     * stored in firstDataLen.
     */
    uint32_t dataLen {};

    /**
     * @brief announced user data length at the start of the transmission
     *
     * Announced user data length at start of
     * transmission. In some cases (e.g. sending to an
     * invalid target address, receive buffer full of
     * target device) the transmission is terminated
     * before all data bytes have been sent. Then the
     * counted number of bytes on bus is less than the
     * announced data length (dataLen <
     * dataLenAnnonced). Due to rare and very
     * specific bit errors (degrade of end termination
     * byte) dataLen can also be greater than
     * dataLenAnnonced.
     */
    uint32_t dataLenAnnounced {};

    /**
     * @brief number of bytes in firstData
     *
     * Number of bytes stored in firstData (1524
     * max).
     *
     * Note: In order to limit the size of this event not
     * all counted bytes on bus will be stored in the
     * payload of the logging event.
     */
    uint32_t firstDataLen {};

    /** reserved */
    uint32_t reservedMostEthernetPktFragment3 {};

    /**
     * @brief variable data
     *
     * Variable data
     */
    std::vector<uint8_t> firstData {};
};

}
}

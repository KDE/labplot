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
 * @brief ETHERNET_FRAME_EX
 *
 * Ethernet frame2
 */
struct VECTOR_BLF_EXPORT EthernetFrameEx final : ObjectHeader {
    EthernetFrameEx();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * Calculates structLength.
     *
     * @return struct length
     */
    virtual uint16_t calculateStructLength() const;

    /**
     * @brief Length of this structure, without sizeof(VBLObjectHeader), without mStructLength member and without mFrameData member
     */
    uint16_t structLength {};

    /**
     * @brief flags, which indicates the valid fields:
     *   - Bit 0 - normal packet = 0, forwarded packet = 1
     *   - Bit 1 - mHardwareChannel valid
     *   - Bit 2 - mFrameDuration valid
     *   - Bit 3 - mFrameChecksum valid
     *   - Bit 4 - mFrameHandle valid
     */
    uint16_t flags {};

    /**
     * @brief application channel, i.e. Eth 1
     */
    uint16_t channel {};

    /**
     * @brief HW channel
     */
    uint16_t hardwareChannel {};

    /**
     * @brief Transmission duration in [ns]
     */
    uint64_t frameDuration {};

    /**
     * @brief Ethernet checksum
     */
    uint32_t frameChecksum {};

    /**
     * @brief Direction flag: 0=Rx, 1=Tx, 2=TxRq
     */
    uint16_t dir {};

    /**
     * @brief Number of valid frameData bytes
     */
    uint16_t frameLength {};

    /**
     * @brief Handle which refer the corresponding EthernetFrameForwarded event
     */
    uint32_t frameHandle {};

    /** reserved */
    uint32_t reservedEthernetFrameEx {};

    /**
     * @brief Max 1612 data bytes per frame. Contains Ethernet header + Ethernet payload
     */
    std::vector<uint8_t> frameData {};
};

}
}

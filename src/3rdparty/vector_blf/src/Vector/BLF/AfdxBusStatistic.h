// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief AFDX_BUS_STATISTIC
 *
 * AFDX line-specific bus-statistic event used since 8.2
 */
struct VECTOR_BLF_EXPORT AfdxBusStatistic final : ObjectHeader {
    AfdxBusStatistic();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     */
    uint16_t channel {};

    /**
     * @brief Bit0=Valid Rx/Tx Counter, Bit1=Valid Error Counter; Bit2=Valid VLId
     */
    uint16_t flags {};

    /**
     * @brief real time period in mysec of statistic datacollection
     */
    uint32_t statDuration {};

    /* bus-specific info */

    /**
     * @brief read frames taken from hardware, i.e. on bus
     */
    uint32_t statRxPacketCountHW {};

    /**
     * @brief send frames as taken from hardware, i.e. on bus
     */
    uint32_t statTxPacketCountHW {};

    /**
     * @brief number of erronous Rx-frames detected by HW
     */
    uint32_t statRxErrorCountHW {};

    /**
     * @brief number of erronous Tx-frames detected by HW
     */
    uint32_t statTxErrorCountHW {};

    /**
     * @brief bytes received by HW during this time period
     */
    uint32_t statRxBytesHW {};

    /**
     * @brief bytes sent by HW during this time period
     */
    uint32_t statTxBytesHW {};

    /* CANwin specific info */

    /**
     * @brief received frames within CANwin
     */
    uint32_t statRxPacketCount {};

    /**
     * @brief send packets from within CANwin
     */
    uint32_t statTxPacketCount {};

    /**
     * @brief number of packets aktively dropped by CANwin
     */
    uint32_t statDroppedPacketCount {};

    /**
     * @brief number of packets with incompatible eth-header regarding AFDX-spec
     */
    uint32_t statInvalidPacketCount {};

    /**
     * @brief number of packets lost by CABwin due to queue overflow etc
     */
    uint32_t statLostPacketCount {};

    /* connection related info */

    /**
     * @brief lineA (0) or lineB (1)
     */
    uint8_t line {};

    /**
     * @brief status of adapter as per EthernetStatus
     */
    uint8_t linkStatus {};

    /**
     * @brief link speed: 0:=10mbps 1:=100mbps 2:=1000mbps
     */
    uint16_t linkSpeed {};

    /**
     * @brief counter of link-losses during this period
     */
    uint16_t linkLost {};

    /** reserved */
    uint16_t reservedAfdxBusStatistic1 {};

    /** reserved */
    uint32_t reservedAfdxBusStatistic2 {};
};

}
}

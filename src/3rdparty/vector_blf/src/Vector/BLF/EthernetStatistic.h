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
 * @brief ETHERNET_STATISTIC
 *
 * Ethernet Statistics
 */
struct VECTOR_BLF_EXPORT EthernetStatistic final : ObjectHeader {
    EthernetStatistic();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** channel */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedEthernetStatistic1 {};

    /** reserved */
    uint32_t reservedEthernetStatistic2 {};

    /** receive ok */
    uint64_t rcvOk_HW {};

    /** transmit ok */
    uint64_t xmitOk_HW {};

    /** receive error */
    uint64_t rcvError_HW {};

    /** transmit error */
    uint64_t xmitError_HW {};

    /** receive bytes */
    uint64_t rcvBytes_HW {};

    /** transmit bytes */
    uint64_t xmitBytes_HW {};

    /** receive no buffer */
    uint64_t rcvNoBuffer_HW {};

    /** signal quality */
    int16_t sqi {};

    /** hardware channel */
    uint16_t hardwareChannel {};

    /** reserved */
    uint32_t reservedEthernetStatistic3 {};
};

}
}

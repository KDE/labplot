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

/** line A/B information in AfdxStatus class */
struct VECTOR_BLF_EXPORT AfdxLineStatus final {
    AfdxLineStatus() noexcept = default;
    virtual ~AfdxLineStatus() noexcept = default;
    AfdxLineStatus(const AfdxLineStatus &) = default;
    AfdxLineStatus & operator=(const AfdxLineStatus &) = default;
    AfdxLineStatus(AfdxLineStatus &&) = default;
    AfdxLineStatus & operator=(AfdxLineStatus &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(AbstractFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(AbstractFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual uint32_t calculateObjectSize() const;

    /**
     * Flags
     *   - Bit 0 - Link Status
     *   - Bit 1 - Bitrate
     *   - Bit 2 - Ethernet Phy
     *   - Bit 3 - Duplex
     */
    uint16_t flags {};

    /**
     * Link Status
     *   - 0 - Unknown
     *   - 1 - Link down
     *   - 2 - Link up
     *   - 3 - Negotiate
     *   - 4 - Link error
     */
    uint8_t linkStatus {};

    /**
     * Eternet Phy
     *   - 0 - Unknown
     *   - 1 - IEEE 802.3
     *   - 2 - BroadR-Reach
     */
    uint8_t ethernetPhy {};

    /**
     * Duplex
     *   - 0 - Unknown
     *   - 1 - Half Duplex
     *   - 2 - Full Duplex
     */
    uint8_t duplex {};

    /**
     * MDI
     *   - 0 - Unknown
     *   - 1 - Direct
     *   - 2 - Crossover
     */
    uint8_t mdi {};

    /**
     * Connector
     *   - 0 - Unknown
     *   - 1 - RJ45
     *   - 2 - D-Sub
     */
    uint8_t connector {};

    /**
     * Clock Mode
     *   - 0 - Unknown
     *   - 1 - Master
     *   - 2 - Slave
     */
    uint8_t clockMode {};

    /**
     * Pairs
     *   - 0 - Unknown
     *   - 1 - BR 1-pair
     *   - 2 - BR 2-pair
     *   - 3 - BR 4-pair
     */
    uint8_t pairs {};

    /** reserved */
    uint8_t reservedAfdxLineStatus1 {};

    /** reserved */
    uint16_t reservedAfdxLineStatus2 {};

    /**
     * Bitrate in [kbit/sec]
     */
    uint32_t bitrate {};
};

}
}

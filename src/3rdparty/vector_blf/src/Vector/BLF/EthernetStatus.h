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
 * @brief ETHERNET_STATUS
 *
 * Ethernet status.
 */
struct VECTOR_BLF_EXPORT EthernetStatus final : ObjectHeader {
    EthernetStatus();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * The channel of the event.
     */
    uint16_t channel {};

    /** enumeration for flags */
    enum Flags : uint16_t {
        /** Link Status */
        LinkStatus = 0x0001,

        /** Bit rate */
        Bitrate = 0x0002,

        /** Ethernet Phy */
        EthernetPhy = 0x0004,

        /** Duplex */
        Duplex = 0x0008,

        /** MDI Type */
        MdiType = 0x0010,

        /** Connector */
        Connector = 0x0020,

        /** Clock Mode */
        ClockMode = 0x0040,

        /** BR Pair */
        BrPair = 0x0080,

        /** Hardware Channel */
        HardwareChannel = 0x0100,
    };

    /** flags */
    uint16_t flags;

    /** enumeration for linkStatus */
    enum LinkStatus : uint8_t {
        /** Unknown */
        UnknownLinkStatus = 0,

        /** Link down */
        LinkDown = 1,

        /** Link up */
        LinkUp = 2,

        /** Negotiate */
        Negotiate = 3,

        /** Link error */
        LinkError = 4
    };

    /**
     * @brief Link Status
     */
    uint8_t linkStatus {};

    /** enumeration for ethernetPhy */
    enum EthernetPhy : uint8_t {
        /** Unknown */
        UnknownEthernetPhy = 0,

        /** IEEE 802.3 */
        Ieee802_3 = 1,

        /** BroadR-Reach */
        BroadR_Reach = 2
    };

    /**
     * @brief Ethernet Phy
     */
    uint8_t ethernetPhy {};

    /** enumeration for duplex */
    enum Duplex : uint8_t {
        /** Unknown */
        UnknownDuplex = 0,

        /** Half Duplex */
        HalfDuplex = 1,

        /** Full Duplex */
        FullDuplex = 2
    };

    /**
     * @brief Duplex
     */
    uint8_t duplex {};

    /** enumeration for mdi */
    enum Mdi : uint8_t {
        /** Unknown */
        UnknownMdi = 0,

        /** Direct */
        Direct = 1,

        /** Crossover */
        Crossover = 2
    };

    /** MDI */
    uint8_t mdi {};

    /** enumeration for connector */
    enum Connector : uint8_t {
        /** Unknown */
        UnknownConnector = 0,

        /** RJ45*/
        Rj45 = 1,

        /** D-Sub */
        DSub = 2
    };

    /** connector */
    uint8_t connector {};

    /** enumeration for clockMode */
    enum ClockMode : uint8_t {
        /** Unknown */
        UnknownClockMode = 0,

        /** Master */
        Master = 1,

        /** Slave */
        Slave = 2
    };

    /** clock mode */
    uint8_t clockMode {};

    /** enumeration for pairs */
    enum Pairs : uint8_t {
        /** Unknown */
        UnknownPairs = 0,

        /** BR 1-pair */
        Br1Pair = 1,

        /** BR 2-pair */
        Br2Pair = 2,

        /** BR 4-pair */
        Br4Pair = 3
    };

    /** pairs */
    uint8_t pairs {};

    /** hardware channel */
    uint8_t hardwareChannel {};

    /**
     * @brief Bitrate in [kbit/sec]
     */
    uint32_t bitrate {};

    /**
     * reservedEthernetStatus1
     */
    uint32_t reservedEthernetStatus1;

    /**
     * reservedEthernetStatus1
     */
    uint32_t reservedEthernetStatus2;

    /**
     * API major number (see FileStatistics)
     *
     * This is used to determine which member variables are valid.
     */
    uint8_t apiMajor{ 2 };
};

}
}

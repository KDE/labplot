// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <string>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief APP_TEXT
 *
 * Application defined text to be saved in BLF log file (currently not used in
 * CANoe/CANalyzer).
 */
struct VECTOR_BLF_EXPORT AppText final : ObjectHeader {
    AppText();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for source */
    enum Source : uint32_t {
        /** measurement comment */
        MeasurementComment = 0x00000000,

        /** database channel info */
        DbChannelInfo = 0x00000001,

        /** meta data */
        MetaData = 0x00000002,
    };

    /**
     * @brief source of text
     *
     * Defines the source/semantic of the text. Actually two
     * different values are defined:
     *
     * 0: Measurement comment
     * - reserved is not used
     * - text contains a measurement comment
     *
     * 1: Database channel information
     * - reserved contains channel information. The following
     * - table show how the 4 bytes are used:
     *   - Bit 0-7: Version of the data
     *   - Bit 8-15: Channel number
     *   - Bit 15-23: Bus type of the channel. One of the
     *     following values:
     *     - 1: BL_BUSTYPE_CAN
     *     - 5: BL_BUSTYPE_LIN
     *     - 6: BL_BUSTYPE_MOST
     *     - 7: BL_BUSTYPE_FLEXRAY
     *     - 9: BL_BUSTYPE_J1708
     *     - 10: BL_BUSTYPE_ETHERNET
     *     - 13: BL_BUSTYPE_WLAN
     *     - 14: BL_BUSTYPE_AFDX
     *   - Bit 24: Flag, that determines, if channel is a CAN-
     *     FD channel
     *   - Bit 25-31: Unused at the moment
     * - text contains database information for the specific
     *   channel. Each database is defined by the database path and
     *   the cluster name (if available). The single databases and the
     *   cluster name are separated by a semicolon. Example:
     *   \<Path1\>;\<ClusterName1\>;\<Path2\>;\<ClusterName2\>;...
     *   If for a database there's no cluster name available, an
     *   empty string is written as cluster name.
     *
     * 2: Meta data
     */
    uint32_t source {};

    /**
     * @brief reserved
     *
     * Depends on source.
     */
    uint32_t reservedAppText1 {};

    /**
     * @brief text length in bytes
     *
     * Length of text without ending 0.
     */
    uint32_t textLength {};

    /** reserved */
    uint32_t reservedAppText2 {};

    /**
     * @brief text in MBCS
     *
     * Text to be saved to log file.
     */
    std::string text {};
};

}
}

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
 * @brief FR_STARTCYCLE
 *
 * FlexRay StartCycle event transmitted by the FlexRay hardware.
 */
struct VECTOR_BLF_EXPORT FlexRayVFrStartCycle final : ObjectHeader {
    FlexRayVFrStartCycle();

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

    /**
     * @brief current cycle
     *
     * Cycle number
     */
    uint8_t cycle {};

    /**
     * @brief clientindex of send node
     *
     * Client index of send node
     */
    uint32_t clientIndexFlexRayVFrStartCycle {};

    /**
     * @brief number of cluster
     *
     * Number of cluster: channel number - 1
     */
    uint32_t clusterNo {};

    /**
     * @brief size of NM Vector
     *
     * Length of NM-Vector in bytes
     */
    uint16_t nmSize {};

    /**
     * @brief array of databytes (NM vector max. length)
     *
     * Array of databytes (NM vector max. length)
     */
    std::array<uint8_t, 12> dataBytes {};

    /** reserved */
    uint16_t reservedFlexRayVFrStartCycle1 {};

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
     * Cyclone I:
     *   - 0: Rate correction of CC, read from RCVR register
     *   - 1: Offset correction of CC, read from OCVR register
     *
     * Cyclone II:
     *   - 0: Sync correction of CC, read from RCV register
     *   - 1: Offset correction of CC, read from OCV register
     *   - 2: Cycles with no correction, read from CCEV register
     *   - 3: Cycles with correction in passive mode, read from CCEV register
     *   - 4: Sync Frame status, read from SFS register
     *
     * VN-Interface:
     *   - 0: Sync correction of CC, read from RCV register
     *   - 1: Offset correction of CC, read from OCV register
     *   - 2: Cycles with no correction, read from CCEV register
     *   - 3: Cycles with correction in passive mode, read from CCEV register
     *   - 4: Sync Frame status, read from SFS register
     */
    std::array<uint32_t, 5> data {};

    /**
     * Reserved
     */
    uint64_t reservedFlexRayVFrStartCycle2 {};
};

}
}

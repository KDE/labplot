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
 * @brief FR_STATUS
 *
 * The content of the FlexRay status event depends on the type of hardware interface.
 * The event is generated in one of the following situations:
 * - A symbol is received
 * - The POC state or wakeup state of the CC has changed
 * - The status of the symbol window has changed
 */
struct VECTOR_BLF_EXPORT FlexRayVFrStatus final : ObjectHeader {
    FlexRayVFrStatus();

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
    uint8_t reservedFlexRayVFrStatus1 {};

    /**
     * @brief clientindex of send node
     *
     * Client index of send node. Must be set to 0 if file is
     * written from other applications
     */
    uint32_t clientIndexFlexRayVFrStatus {};

    /**
     * @brief number of cluster
     *
     * Number of cluster: channel number – 1
     */
    uint32_t clusterNo {};

    /**
     * @brief wakeup status
     *
     * WakeUp state. Only valid
     *   - for Vector interfaces and for Cyclone II,
     *   - if symbol is void (mReserved[0] = 0)
     *
     * Meaning (see E-Ray specification for a
     * detailed description):
     *   - 0: UNDEFINED
     *   - 1: RECEIVED_HEADER
     *   - 2: RECEIVED_WUP
     *   - 3: COLLISION_HEADER
     *   - 4: COLLISION_WUP
     *   - 5: COLLISION_UNKNOWN
     *   - 6: TRANSMITTED
     *   - 7: EXTERNAL_WAKEUP
     *   - 8: WUP_RECEIVED_WITHOUT_WUS_TX
     */
    uint32_t wus {};

    /**
     * @brief sync state of cc
     *
     * Sync-State, only valid
     *   - for Cyclone 1
     *   - for Cyclone II if the wakup state value is 0.
     *
     * Meaning:
     *   - 0 = Not synced passive
     *   - 1 = Synced active
     *   - 2 = Not synced
     */
    uint32_t ccSyncState {};

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
     *   - data[0]: Content of Protocol state register (PSR)
     *   - data[1]: Content of Module config register (MCR0)
     *
     * CC-Type: BUSDOCTOR
     *   - LOW-WORD of data[0]: Symbol length
     *   - HI-WORD of data[0]: Flags: 1 = possible CAS
     *   - data[1]: Reserved
     *
     * CC-Type: VN-Interface
     *   - data[0]: POC state of E-Ray register CCSV. Only valid
     *       - for Vector interfaces
     *       - if wakeup state is 0
     *     POC State in the operation control phase:
     *       - 0x00: DEFAULT_CONFIG
     *       - 0x01: READY
     *       - 0x02: NORMAL_ACTIVE
     *       - 0x03: NORMAL_PASSIVE
     *       - 0x04: HALT
     *       - 0x05: MONITOR_NODE
     *       - 0x06: CONFIG
     *     POC State in the wake-up phase:
     *       - 0x10: WAKEUP_STANDBY
     *       - 0x11: WAKEUP_LISTEN
     *       - 0x12: WAKEUP_SEND
     *       - 0x13: WAKEUP_DETECT
     *     POC State in the start-up phase:
     *       - 0x20: STARTUP_PREPARE
     *       - 0x21: COLDSTART_LISTEN
     *       - 0x22: COLDSTART_COLLISION_RESOLUTION
     *       - 0x23: COLDSTART_CONSISTENCY_CHECK
     *       - 0x24: COLDSTART_GAP
     *       - 0x25: COLDSTART_JOIN
     *       - 0x26: INTEGRATION_COLDSTART_CHECK
     *       - 0x27: INTEGRATION_LISTEN
     *       - 0x28: INTEGRATION_CONSISTENCY_CHECK
     *       - 0x29: INITIALIZE_SCHEDULE
     *       - 0x30: ABORT_STARTUP
     *       - 0x31: STARTUP_SUCCESS
     *     All other values are reserved.
     *   - LOW-WORD of data[1]:
     *     Bit field indicating the symbol window status of the controller and the event
     *     source.
     *       - 1: SESA (Syntax error in symbol window channel A)
     *       - 2: SBSA (Slot boundary violation in symbol window channel A)
     *       - 4: TCSA (Transmission conflict in symbol window channel A)
     *       - 8: SESB (Syntax error in symbol window channel B)
     *       - 16: SBSB (Slot boundary violation in symbol window channel B)
     *       - 32: TCSB (Transmission conflict in symbol window channel B)
     *       - 64: The event was generated from a controller-independent protocol
     *             interpreter (Spy).
     *       - 128: Cold-start helper POC indicator, if set, event contains the POC
     *              state of the cold-start helper
     *     All other bits are reserved. CANoe/CANalyzer may set some of these bits to 1.
     *     Other applications must set them to 0.
     *   - HI-WORD of data[1]:
     *     Symbol length in bit times. Only valid for symbol type 4 and if the value is not
     *     zero.
     */
    std::array<uint32_t, 2> data {};

    /**
     * @brief reserved
     *
     * reserved[0]:
     * If this value is not zero, then the event contains the
     * information about a symbol.
     *   - 0 = Void
     *   - 1 = CAS
     *   - 2 = MTS
     *   - 3 = WUS
     *   - 4 = Network interface doesn’t provide a symbol interpretation,
     *         e.g. if spy-mode is used or the BUSDOCTOR
     *         interface. In spy mode, the symbol length is
     *         stored in the HI-WORD of data[1].
     *
     * reserved[1..15]:
     * Reserved
     */
    std::array<uint16_t, 18> reservedFlexRayVFrStatus2 {};
};

}
}

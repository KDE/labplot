// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_NETSTATE
 *
 * Network state derived by MOST Supervisor Layer I+II
 */
struct VECTOR_BLF_EXPORT MostNetState final : ObjectHeader2 {
    MostNetState();

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
     * @brief MOST NetState
     *
     * Current network state
     *   - 0 (undefined): Before the first event (shortly after
     *     measurement start) the network status is
     *     unknown.
     *   - 1 (reserved for Ring Break Diagnostics mode)
     *   - 2 (PowerOff): The network interface to the
     *     MOST ring is deactivated. The Tx FOT is not
     *     emitting any light.
     *   - 3 (NetInterfaceInit): The network interface is
     *     ready to communicate in the MOST ring.
     *   - 4 (ConfigNotOk): The network interface is in
     *     normal operating mode (stable lock).
     *   - 5 (ConfigOk): From the perspective of the
     *     Network Master the system configuration is
     *     valid.
     *   - 6 (NetOn/InitReady): NetOn/InitReady reported
     *     to application
     */
    uint16_t stateNew {};

    /**
     * Previous network state
     */
    uint16_t stateOld {};

    /** reserved */
    uint16_t reservedMostNetState {};
};

}
}

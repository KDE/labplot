// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinBusEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Common header of LIN bus events containing break field data
 */
struct VECTOR_BLF_EXPORT LinSynchFieldEvent : LinBusEvent {
    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief Sync Break Length in ns
     *
     * Length of dominant part [in nanoseconds]
     */
    uint64_t synchBreakLength {};

    /**
     * @brief Sync Delimiter Length in ns
     *
     * Length of delimiter (recessive) [in nanoseconds]
     */
    uint64_t synchDelLength {};
};

}
}

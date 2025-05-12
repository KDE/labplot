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
 * @brief DATA_LOST_END
 */
struct VECTOR_BLF_EXPORT DataLostEnd final : ObjectHeader {
    DataLostEnd();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for queueIdentifier */
    enum QueueIdentifier : uint32_t {
        RtQueue = 0,
        AnlyzQueue = 1,
        RtAndAnlyzQueue = 2
    };

    /** identifier for the leaking queue */
    uint32_t queueIdentifier {};

    /** timestamp of the first object lost */
    uint64_t firstObjectLostTimeStamp {};

    /** number of lost events */
    uint32_t numberOfLostEvents {};
};

}
}

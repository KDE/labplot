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
 * @brief WATER_MARK_EVENT
 */
struct VECTOR_BLF_EXPORT WaterMarkEvent final : ObjectHeader {
    WaterMarkEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for queueState */
    enum QueueState : uint32_t {
        StatusNormal = 0,
        StatusEmergency = 1,
        StatusLostData = 2
    };

    /** the current state of the queue */
    uint32_t queueState {};

    /** reserved */
    uint32_t reservedWaterMarkEvent {};
};

}
}

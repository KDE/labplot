// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/CanFdExtFrameData.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Event that occurs when a reset or a bit timing change occurs on a CAN channel.
 */
struct VECTOR_BLF_EXPORT CanSettingChanged final : ObjectHeader {
    CanSettingChanged();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Application channel
     */
    uint16_t channel;

    /**
     * @brief -1 - Invalid Type; 0 - Reseted; 1 - Bit Timing Changed
     *
     * The following values are possible:
     * -1: Invalid Change Type
     * 0: Reset event
     * 1: Bit timing changed
     */
    uint8_t changedType;

    CanFdExtFrameData bitTimings;
};

}
}

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
 * @brief TRIGGER_CONDITION
 */
struct VECTOR_BLF_EXPORT TriggerCondition final : ObjectHeader {
    TriggerCondition();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for state */
    enum State : uint32_t {
        Unknown = 0,
        Start = 1,
        Stop = 2,
        Startstop = 3
    };

    /** status */
    uint32_t state {};

    /** length of trigger block name in bytes */
    uint32_t triggerBlockNameLength {};

    /** length of trigger condition in bytes */
    uint32_t triggerConditionLength {};

    /** trigger block name */
    std::string triggerBlockName {};

    /** trigger condition */
    std::string triggerCondition {};
};

}
}

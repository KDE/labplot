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
 * @brief A429_ERROR
 *
 * A429 error object
 */
struct VECTOR_BLF_EXPORT A429Error final : ObjectHeader {
    A429Error();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** application channel */
    uint16_t channel {};

    /** error type, 0=error, 1=warning, 2=info */
    uint16_t errorType {};

    /** source identifier */
    uint32_t sourceIdentifier {};

    /** error reason */
    uint32_t errReason {};

    /** error text */
    std::array<char, 512> errorText {};

    /** error attributes */
    std::array<char, 512> errorAttributes {};

    /** reserved */
    uint32_t reservedA429Error {};
};

}
}

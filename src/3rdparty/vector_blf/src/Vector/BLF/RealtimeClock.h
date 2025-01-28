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
 * @brief REALTIMECLOCK
 *
 * Realtime clock object
 */
struct VECTOR_BLF_EXPORT RealtimeClock final : ObjectHeader {
    RealtimeClock();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief logging start time in ns since 00:00 1.1.1970 GMT
     */
    uint64_t time {};

    /**
     * @brief measurement zero offset in ns to 00:00 1.1.1970 GMT
     */
    uint64_t loggingOffset {};
};

}
}

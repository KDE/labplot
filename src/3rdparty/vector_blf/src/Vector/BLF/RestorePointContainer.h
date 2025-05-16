// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>
#include <vector>

#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief UNKNOWN_115
 *
 * These objects store a contiguous block of data containing the Restore
 * Points, similar to what a LogContainer does for Log data.
 *
 * The default dataLength is 2000, which results in an objectSize of 2048.
 *
 * @note
 *   This class is based on observations, as there is no
 *   public documentation available. There are undocumented API functions for
 *   RestorePoint handling. And this seems like it.
 */
struct VECTOR_BLF_EXPORT RestorePointContainer final : public ObjectHeader {
    RestorePointContainer();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** reserved */
    std::array<uint8_t, 14> reservedRestorePointContainer {};

    /** restore point data size */
    uint16_t dataLength {};

    /** restore point data */
    std::vector<uint8_t> data {};
};

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinMessageDescriptor.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Common header of LIN bus events containing response data bytes
 */
struct VECTOR_BLF_EXPORT LinDatabyteTimestampEvent : LinMessageDescriptor {
    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief Databyte timestamps, where d[0] = EndOfHeader, d[1]=EndOfDataByte1, ..., d[8]=EndOfDataByte8
     *
     * Data byte timestamps [in nanoseconds]
     *
     * Index 0 corresponds to last header byte
     *
     * Indexes 1-9 correspond to response data
     * bytes D1-D8
     */
    std::array<uint64_t, 9> databyteTimestamps {};
};

}
}

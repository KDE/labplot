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
 * @brief A429_STATUS
 *
 * A429 status object
 */
struct VECTOR_BLF_EXPORT A429Status final : ObjectHeader {
    A429Status();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** application channel */
    uint16_t channel {};

    /** direction flag: 0=Rx, 1=Tx */
    uint8_t dir {};

    /** reserved */
    uint8_t reservedA429Status1 {};

    /** parity */
    uint16_t parity {};

    /** reserved */
    uint16_t reservedA429Status2 {};

    /** minimum gap */
    uint32_t minGap {};

    /** Tx bit rate */
    uint32_t bitrate {};

    /** Rx min bit rate */
    uint32_t minBitrate {};

    /** Rx max bit rate */
    uint32_t maxBitrate {};
};

}
}

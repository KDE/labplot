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
 * @brief LIN_CHECKSUM_INFO
 *
 * This info event occurs when the LIN hardware successfully detected the checksum
 * model of an unknown frame. This checksum model is set as the expected one for this frame in the
 * future.
 */
struct VECTOR_BLF_EXPORT LinChecksumInfo final : ObjectHeader {
    LinChecksumInfo();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Channel number where the event notified
     */
    uint16_t channel {};

    /**
     * @brief LIN ID
     *
     * Frame identifier
     */
    uint8_t id {};

    /**
     * @brief LIN checksum model
     *
     * Used checksum model. Following values are
     * possible:
     *   - 0: Classic
     *   - 1: Enhanced
     *   - 0xFF: Unknown
     */
    uint8_t checksumModel {};

    /** reserved */
    uint32_t reservedLinChecksumInfo {};
};

}
}

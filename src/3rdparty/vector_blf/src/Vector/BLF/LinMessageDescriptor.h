// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinSynchFieldEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Common header of LIN bus events containing LIN header data
 */
struct VECTOR_BLF_EXPORT LinMessageDescriptor : LinSynchFieldEvent {
    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief LIN Sub-Identifier - Supplier ID
     *
     * Supplier identifier of the frame’s transmitter
     * as it is specified in LDF. LIN protocol 2.0
     * and higher
     */
    uint16_t supplierId {};

    /**
     * @brief LIN Sub-Identifier - Message ID (16 bits)
     *
     * LIN protocol 2.0: Message identifier (16-bit)
     * of the frame as it is specified in LDF in the
     * list of transmitter’s configurable frames.
     *
     * LIN protocol 2.1: Position index of the frame
     * as it is specified in LDF in the list of
     * transmitter’s configurable frames.
     */
    uint16_t messageId {};

    /**
     * @brief LIN Sub-Identifier - NAD
     *
     * Configured Node Address of the frame’s
     * transmitter as it is specified in LDF. LIN
     * protocol 2.0 and higher
     */
    uint8_t nad {};

    /**
     * @brief LIN ID
     *
     * Frame identifier (6-bit)
     */
    uint8_t id {};

    /**
     * @brief LIN DLC
     *
     * Frame length [in bytes]
     */
    uint8_t dlc {};

    /**
     * @brief LIN checksum model
     *
     * Expected checksum model of checksum
     * value. Only valid if objectVersion >= 1.
     */
    uint8_t checksumModel {};
};

}
}

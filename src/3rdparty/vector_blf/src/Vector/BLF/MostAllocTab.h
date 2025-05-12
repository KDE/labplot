// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader2.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief MOST_ALLOCTAB
 *
 * Transports current state of the MOST25 Allocation Table of connected hardware
 * interface.
 */
struct VECTOR_BLF_EXPORT MostAllocTab final : ObjectHeader2 {
    MostAllocTab();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief application channel
     *
     * Application channel
     */
    uint16_t channel {};

    /**
     * Length of variable data (usually 60 bytes for
     * MOST25)
     */
    uint16_t length {};

    /** reserved */
    uint32_t reservedMostAllocTab {};

    /**
     * Allocation Table
     *
     * The label of a synchronous connection can be
     * distributed over several bytes in the Allocation
     * Table.
     *
     * Each byte in mTableData contains a value that
     * specifies the identification number of the label it
     * belongs to. If the device is a timing master, the
     * MSB of the byte value is used to indicate if the
     * label is in use or not, otherwise the MSB should
     * be ignored. The label number thus can be
     * determined by byte value & 0x7F. If the resulting
     * label number is 0x70, the byte is not used for any
     * label.
     */
    std::vector<uint8_t> tableData {};
};

}
}

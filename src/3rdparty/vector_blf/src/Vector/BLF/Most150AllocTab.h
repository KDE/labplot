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
 * @brief MOST_150_ALLOCTAB
 *
 * Transports current state and changes of the MOST50/150 Allocation Table.
 */
struct VECTOR_BLF_EXPORT Most150AllocTab final : ObjectHeader2 { /* applied for MOST50 and MOST150 */
    Most150AllocTab();

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
     * @brief determines the data layout
     *
     * Determines the data layout (see below)
     */
    uint16_t eventModeFlags {};

    /**
     * @brief number of free bytes after the operation
     *
     * Number of free bytes after the operation
     *   - Max. 116 with SBC=29 for MOST50
     *   - Max. 372 with SBC=93 for MOST150
     */
    uint16_t freeBytes {};

    /**
     * @brief number of bytes in tableData
     *
     * Length of variable data in bytes. The value must
     * be a multiple of 4.
     */
    uint16_t length {};

    /** reserved */
    uint64_t reservedMost150AllocTab {};

    /**
     * Allocation Table data
     *
     * The data layout tableData depends on bit 0 of eventModeFlags.
     *
     * If bit 0 of eventModeFlags is clear tableData contains length/4 records with the following
     * fields. (Other data layouts are not specified yet.)
     * - Bit 0..12 (LabelIdent): Synchronous Connection Label
     * - Bit 12..15 (LabelStatus):
     *   - 0: label unchanged
     *   - 4: label has been added (allocated)
     *   - 8: label has been removed (de-allocated)
     *     List removed labels at the end of the table!
     *     Listing of removed labels is optional.
     * - Bit 16..31 (LabelWidth): Width of the label in bytes
     *
     * Data layout:
     * if((eventModeFlags & 0x0001) == 0)
     *   layout A: SLLLWWWWSLLLWWWWSLLLWWWW...
     * if((eventModeFlags & 0x0001) == 0x0001)
     *   layout B: SLLLWWWW< channels >SLLLWWWW< channels >SLLLWWWW< channels >...
     * - S: status flags
     *   - 0x4: 1: new label (alloc)
     *   - 0x8: 1: this label has been removed (dealloc)
     * - LLL:  label number
     * - WWWW: label width
     * - < channels >: list of 16-bit channel numbers (size = label width)
     */
    std::vector<uint8_t> tableData {};
};

}
}

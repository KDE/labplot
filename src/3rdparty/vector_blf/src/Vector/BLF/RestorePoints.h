// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <cstdint>
#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/RestorePoint.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * List of restore points.
 *
 * @note
 *   This class is based on observations, as there is no
 *   public documentation available. There are undocumented API functions for
 *   RestorePoint handling. And this seems like it.
 */
struct VECTOR_BLF_EXPORT RestorePoints final {
    /**
     * Read the data of this object
     *
     * @param is input stream
     */
    virtual void read(AbstractFile & is);

    /**
     * Write the data of this object
     *
     * @param os output stream
     */
    virtual void write(AbstractFile & os);

    /**
     * Calculates the objectSize
     *
     * @return object size
     */
    virtual uint32_t calculateObjectSize() const;

    /**
     * @todo Is this the maximum byte size of the restorePoints vector?
     *
     * Not sure with this.
     *
     * objectSize is usually 0x00180008=1572872. Reduced by sizeof(objectSize) +
     * sizeof(objectInterval) lefts 1572864 bytes for restorePoints.
     * Each RestorePoint has 24 bytes, which would result in 65536 elements.
     * Seems like a potential maximum for the RestorePoints.
     * But why does this need to be stored in the file?
     *
     * Actually restorePointData.size() in this example, was just 20936, based on available data,
     * which is 502464 bytes. So I would have expected to see objectSize=502472.
     */
    uint32_t objectSize {0x00180008};

    /**
     * object interval
     *
     * This is the interval between RestorePoints. Actually, due to a bug in the
     * implementation, the interval is actually objectInterval + 1;
     * Default is 1000, so it's storing a list, which point to each 1001stth
     * object.
     */
    uint32_t objectInterval {1000};

    /**
     * Restore Points
     */
    std::vector<RestorePoint> restorePoints {};

};

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <cstdint>
#include <vector>

#include <Vector/BLF/AbstractFile.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Restore Point
 *
 * Restore Points are kept in a list that is basically an index that references
 * each 1001stth object. So skipping object 0, it counts 1000, 2001, 3002, ...
 *
 * @note
 *   This class is based on observations, as there is no
 *   public documentation available. There are undocumented API functions for
 *   RestorePoint handling. And this seems like it.
 */
struct VECTOR_BLF_EXPORT RestorePoint final {
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
    static uint32_t calculateObjectSize();

    /**
     * time stamp (in ns)
     *
     * The following file positions and offsets refer to an Object.
     * The time stamp is from this object.
     */
    uint64_t timeStamp {};

    /**
     * compressed file position
     *
     * This designates the position of a LogContainer in the compressed
     * file.
     */
    uint64_t compressedFilePosition {};

    /**
     * uncompressed file offset
     *
     * This designates the offset within the uncompressed content of
     * the LogContainer, where an Object starts. This Object is not
     * necessarily the first or last Object, but an arbitrary one.
     */
    uint32_t uncompressedFileOffset {};

    /**
     * @todo It's unclear what this variable is.
     *
     * Maybe just a reserved field for future extensions.
     *
     * Examples show:
     * - File1: 0, 0x9a
     * - File2: 0, 1, 2, 3, 4, 5, 6
     * - File3: 0, 0xffffffff, 0x10020
     * - File4: 0xb9
     * - File5: 0, 0x2c9a8a00
     *
     * This is similar data as for LogContainer.unknownLogContainer.
     *
     * @see LogContainer
     */
    uint32_t unknownRestorePoint {};
};

}
}

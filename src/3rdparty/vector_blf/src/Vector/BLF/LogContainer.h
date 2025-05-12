// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeaderBase.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LOG_CONTAINER
 */
struct VECTOR_BLF_EXPORT LogContainer final : ObjectHeaderBase {
    LogContainer();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * compression method
     *
     *   - 0: no compression
     *   - 2: zlib deflate
     */
    uint16_t compressionMethod {};

    /**
     * reserved
     */
    uint16_t reservedLogContainer1 {};

    /**
     * reserved
     */
    uint32_t reservedLogContainer2 {};

    /**
     * uncompressed file size in bytes
     */
    uint32_t uncompressedFileSize {};

    /** reserved */
    uint32_t reservedLogContainer3 {}; // @todo what is this? 0x0215, 0x9a, 0, 0x0c...

    /** compressed file content */
    std::vector<uint8_t> compressedFile {};

    /* following data is calculated */

    /** uncompressed file content */
    std::vector<uint8_t> uncompressedFile {};

    /** compressed file size in bytes */
    uint32_t compressedFileSize {};

    /** position of this LogContainer::uncompressedFile within UncompressedFile class */
    std::streampos filePosition {};

    /**
     * internalHeaderSize returns the size of all headers.
     *
     * @note Not to be mixed with headerSize, which only takes ObjectHeaderBase headers into account.
     *
     * @return Size of all headers.
     */
    uint16_t internalHeaderSize() const;

    /**
     * uncompress data
     */
    virtual void uncompress();

    /**
     * compress data
     *
     * @param[in] compressionMethod compression method
     * @param[in] compressionLevel compression level (different for each method)
     */
    virtual void compress(const uint16_t compressionMethod, const int compressionLevel);
};

}
}

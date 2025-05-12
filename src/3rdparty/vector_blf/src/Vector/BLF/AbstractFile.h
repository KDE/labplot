// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <ios>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Generic interface to access CompressedFile and UncompressedFile in the same way.
 */
struct VECTOR_BLF_EXPORT AbstractFile {
    AbstractFile() noexcept = default;
    virtual ~AbstractFile() noexcept = default;
    AbstractFile(const AbstractFile &) = default;
    AbstractFile & operator=(const AbstractFile &) = default;
    AbstractFile(AbstractFile &&) = default;
    AbstractFile & operator=(AbstractFile &&) = default;

    /**
     * Get characters returned by last read operation.
     *
     * @return Number of characters
     */
    virtual std::streamsize gcount() const = 0;

    /**
     * Read block of data.
     *
     * This operation blocks until the data is available.
     *
     * @param[out] s Pointer to data
     * @param[in] n Requested size of data
     */
    virtual void read(char * s, std::streamsize n) = 0;

    /**
     * Get position in input sequence.
     *
     * @return Read position
     */
    virtual std::streampos tellg() = 0;

    /**
     * Set position in input sequence.
     *
     * @param[in] off Offset
     * @param[in] way Direction
     */
    virtual void seekg(std::streamoff off, const std::ios_base::seekdir way = std::ios_base::cur) = 0;

    /**
     * Write block of data.
     *
     * @param[in] s Pointer to data
     * @param[in] n Size of data
     */
    virtual void write(const char * s, std::streamsize n) = 0;

    /**
     * Get position in output sequence.
     *
     * @return Write position
     */
    virtual std::streampos tellp() = 0;

    /**
     * Check whether state of stream is good.
     *
     * @return true if no error flags are set
     */
    virtual bool good() const = 0;

    /**
     * Check whether eofbit is set.
     *
     * @return true if eofbit is set
     */
    virtual bool eof() const = 0;

    /**
     * Write padding null bytes.
     *
     * @param s Number of padding bytes.
     */
    virtual void skipp(std::streamsize s) final;
};

}
}

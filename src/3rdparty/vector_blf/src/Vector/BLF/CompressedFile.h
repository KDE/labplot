// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <fstream>
#include <mutex>

#include <Vector/BLF/AbstractFile.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * CompressedFile (Input/output file stream)
 *
 * This class is thread-safe.
 */
class VECTOR_BLF_EXPORT CompressedFile final : public AbstractFile {
  public:
    CompressedFile() = default;
    ~CompressedFile() override;
    CompressedFile(const CompressedFile &) = delete;
    CompressedFile & operator=(const CompressedFile &) = delete;
    CompressedFile(CompressedFile &&) = delete;
    CompressedFile & operator=(CompressedFile &&) = delete;

    std::streamsize gcount() const override;
    void read(char * s, std::streamsize n) override;
    std::streampos tellg() override;
    void seekg(std::streamoff off, const std::ios_base::seekdir way = std::ios_base::cur) override;
    void write(const char * s, std::streamsize n) override;
    std::streampos tellp() override;
    bool good() const override;
    bool eof() const override;

    /**
     * open file
     *
     * @param filename file name
     * @param openMode open in read or write mode
     */
    virtual void open(const char * filename, std::ios_base::openmode openMode);

    /**
     * is file open?
     *
     * @return true if file is open
     */
    virtual bool is_open() const;

    /**
     * Close file.
     */
    virtual void close();

    /**
     * Set position in output sequence.
     *
     * @param[in] pos Position
     */
    virtual void seekp(std::streampos pos);

  private:
    /**
     * file stream
     */
    std::fstream m_file {};

    /** mutex */
    mutable std::mutex m_mutex {};
};

}
}

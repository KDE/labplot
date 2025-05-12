// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <condition_variable>
#include <limits>
#include <list>
#include <memory>
#include <mutex>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LogContainer.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * UncompressedFile (Input/output memory stream)
 *
 * This class is like a virtual file buffer. It only sees
 * the fragments that are contained in m_data and addresses
 * by the underlying uncompressed LogContainers.
 * Read is done at position m_tellg and write position is at m_tellp.
 * Write or seek operations exceeding the end of the file, will
 * automatically create new logContainers. An explicit dropOldData
 * drops logContainers that have already been processed.
 *
 * This class is thread-safe.
 */
class VECTOR_BLF_EXPORT UncompressedFile final : public AbstractFile {
  public:
    UncompressedFile() = default;
    ~UncompressedFile() override;

    std::streamsize gcount() const override;
    void read(char * s, std::streamsize n) override;
    std::streampos tellg() override;
    void seekg(std::streamoff off, const std::ios_base::seekdir way = std::ios_base::cur) override;
    void write(const char * s, std::streamsize n) override;
    std::streampos tellp() override;
    bool good() const override;
    bool eof() const override;

    /**
     * Stop further operations. Return from waiting reads.
     */
    virtual void abort();

    /**
     * write LogContainer
     *
     * @param[in] logContainer log container
     */
    virtual void write(const std::shared_ptr<LogContainer> & logContainer);

    /**
     * Close the current logContainer.
     */
    virtual void nextLogContainer();

    /**
     * Return current file size resp. end-of-file position.
     *
     * @return file size
     */
    virtual std::streamsize fileSize() const;

    /**
     * Set file size resp. end-of-file position.
     *
     * @param[in] fileSize file size
     */
    virtual void setFileSize(std::streamsize fileSize);

    /**
     * Sets the maximum file size.
     * Write operations block, if the size is reached.
     *
     * @param[in] bufferSize maximum file size
     */
    virtual void setBufferSize(std::streamsize bufferSize);

    /**
     * drop old log container, if tellg/tellp are beyond it
     */
    virtual void dropOldData();

    /**
     * Get default log container size.
     *
     * @return default log container size
     */
    virtual uint32_t defaultLogContainerSize() const;

    /**
     * Set default log container size.
     *
     * @param[in] defaultLogContainerSize default log container size
     */
    virtual void setDefaultLogContainerSize(uint32_t defaultLogContainerSize);

    /** tellg was changed (after read or seekg) */
    std::condition_variable tellgChanged;

    /** tellp was changed (after write or seekp) */
    std::condition_variable tellpChanged;

  private:
    /** abort further operations */
    bool m_abort {};

    /** data */
    std::list<std::shared_ptr<LogContainer>> m_data {};

    /** get position */
    std::streampos m_tellg {};

    /** put position */
    std::streampos m_tellp {};

    /** last read size */
    std::streamsize m_gcount {};

    /** file size */
    std::streamsize m_fileSize {std::numeric_limits<std::streamsize>::max()};

    /** buffer size */
    std::streamsize m_bufferSize {std::numeric_limits<std::streamsize>::max()};

    /** error state */
    std::ios_base::iostate m_rdstate {std::ios_base::goodbit};

    /** mutex */
    mutable std::mutex m_mutex {};

    /** default log container size */
    uint32_t m_defaultLogContainerSize {0x20000};

    /**
     * Returns the file container, which contains pos.
     *
     * Searches through the data vector to find the logContainer, which contains the position.
     * If the position is behind the last logContainer, return nullptr to indicate a new
     * LogContainer need to be appended.
     *
     * @param[in] pos position
     * @return log container or nullptr
     */
    std::shared_ptr<LogContainer> logContainerContaining(const std::streampos pos) const;
};

}
}

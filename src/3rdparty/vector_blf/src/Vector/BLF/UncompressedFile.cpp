// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/UncompressedFile.h>

#undef DEBUG_WRITE_LOG_CONTAINERS_TO_DISK

#include <algorithm>
#include <cstring>
#ifdef DEBUG_WRITE_LOG_CONTAINERS_TO_DISK
#include <fstream>
#endif

#include <Vector/BLF/Exceptions.h>

namespace Vector {
namespace BLF {

UncompressedFile::~UncompressedFile() {
    abort();
}

std::streamsize UncompressedFile::gcount() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_gcount;
}

void UncompressedFile::read(char * s, std::streamsize n) {
    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* wait until there is sufficient data */
    tellpChanged.wait(lock, [&] {
        return
        m_abort ||
        (n + m_tellg <= m_tellp) ||
        (n + m_tellg > m_fileSize);
    });

    /* handle read behind eof */
    if (n + m_tellg > m_fileSize) {
        n = m_fileSize - m_tellg;
        m_rdstate = std::ios_base::eofbit | std::ios_base::failbit;
    } else
        m_rdstate = std::ios_base::goodbit;

    /* read data */
    m_gcount = 0;
    while (n > 0) {
        /* find starting log container */
        std::shared_ptr<LogContainer> logContainer = logContainerContaining(m_tellg);
        if (!logContainer)
            break;

        /* offset to read */
        std::streamoff offset = m_tellg - logContainer->filePosition;

        /* copy data */
        std::streamsize gcount = std::min(n, static_cast<std::streamsize>(logContainer->uncompressedFileSize - offset));
        std::copy(logContainer->uncompressedFile.cbegin() + offset, logContainer->uncompressedFile.cbegin() + offset + gcount, s);

        /* remember get count */
        m_gcount += gcount;

        /* new get position */
        m_tellg += gcount;

        /* advance */
        s += gcount;

        /* calculate remaining data to copy */
        n -= gcount;
    }

    /* notify */
    tellgChanged.notify_all();
}

std::streampos UncompressedFile::tellg() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* in case of failure return -1 */
    if (m_rdstate & (std::ios_base::failbit | std::ios_base::badbit))
        return -1;
    return m_tellg;
}

void UncompressedFile::seekg(std::streamoff off, const std::ios_base::seekdir /*way*/) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* new get position */
    m_tellg = std::min(static_cast<std::streamsize>(m_tellg + off), m_fileSize);

    /* notify */
    tellgChanged.notify_all();
}

void UncompressedFile::write(const char * s, std::streamsize n) {
    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* wait for free space */
    tellgChanged.wait(lock, [&] {
        return
        m_abort ||
        ((m_tellp - m_tellg) < m_bufferSize);
    });

    /* write data */
    while (n > 0) {
        /* find starting log container */
        std::shared_ptr<LogContainer> logContainer = logContainerContaining(m_tellp);

        /* append new log container */
        if (!logContainer) {
            /* append new log container */
            logContainer = std::make_shared<LogContainer>();
            logContainer->uncompressedFile.resize(m_defaultLogContainerSize);
            logContainer->uncompressedFileSize = logContainer->uncompressedFile.size();
            if (!m_data.empty()) {
                logContainer->filePosition =
                    m_data.back()->uncompressedFileSize +
                    m_data.back()->filePosition;
            }
            m_data.push_back(logContainer);
        }

        /* offset to write */
        std::streamoff offset = m_tellp - logContainer->filePosition;

        /* copy data */
        std::streamsize pcount = std::min(n, static_cast<std::streamsize>(logContainer->uncompressedFileSize - offset));
        if (pcount > 0) {
            std::copy(s, s + pcount, logContainer->uncompressedFile.begin() + offset);

            /* new put position */
            m_tellp += pcount;

            /* advance */
            s += pcount;

            /* calculate remaining data to copy */
            n -= pcount;
        }
    }

    /* if new position is behind eof, shift it */
    if (m_tellp >= m_fileSize)
        m_fileSize = m_tellp;

    /* notify */
    tellpChanged.notify_all();
}

std::streampos UncompressedFile::tellp() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* in case of failure return -1 */
    if (m_rdstate & (std::ios_base::failbit | std::ios_base::badbit))
        return -1;
    return m_tellp;
}

bool UncompressedFile::good() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return (m_rdstate == std::ios_base::goodbit);
}

bool UncompressedFile::eof() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return (m_rdstate & std::ios_base::eofbit);
}

void UncompressedFile::abort() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* stop */
    m_abort = true;

    /* trigger blocked threads */
    tellgChanged.notify_all();
    tellpChanged.notify_all();
}

void UncompressedFile::write(const std::shared_ptr<LogContainer> & logContainer) {
    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* wait for free space */
    tellgChanged.wait(lock, [&] {
        return
        m_abort ||
        static_cast<uint32_t>(m_tellp - m_tellg) < m_bufferSize;
    });

    /* append logContainer */
    m_data.push_back(logContainer);
    logContainer->filePosition = m_tellp;

#ifdef DEBUG_WRITE_LOG_CONTAINERS_TO_DISK
    /* write the logContainer to disk */
    std::ofstream ofs(std::to_string(logContainer->filePosition) + ".blf");
    ofs.write(reinterpret_cast<const char *>(logContainer->uncompressedFile.data()), logContainer->uncompressedFileSize);
#endif

    /* advance put pointer */
    m_tellp += logContainer->uncompressedFileSize;

    /* notify */
    tellpChanged.notify_all();
}

void UncompressedFile::nextLogContainer() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* find starting log container */
    std::shared_ptr<LogContainer> logContainer = logContainerContaining(m_tellp);
    if (logContainer) {
        /* offset to write */
        std::streamoff offset = m_tellp - logContainer->filePosition;

        /* resize logContainer, if it's not already a newly created one */
        if (offset > 0) {
            logContainer->uncompressedFile.resize(offset);
            logContainer->uncompressedFileSize = offset;
        }
    }
}

std::streamsize UncompressedFile::fileSize() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* return file size */
    return m_fileSize;
}

void UncompressedFile::setFileSize(std::streamsize fileSize) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* set eof at m_dataEnd */
    m_fileSize = fileSize;

    /* notify */
    tellpChanged.notify_all();
}

void UncompressedFile::setBufferSize(std::streamsize bufferSize) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* set max size */
    m_bufferSize = bufferSize;
}

void UncompressedFile::dropOldData() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* check if drop should be done now */
    if (m_data.empty()) {
        return;
    }
    std::shared_ptr<LogContainer> logContainer = m_data.front();
    if (logContainer) {
        std::streampos position = logContainer->uncompressedFileSize + logContainer->filePosition;
        if ((position > m_tellg) || (position > m_tellp) || (position > m_fileSize)) {
            /* don't drop yet */
            return;
        }
    }

    /* drop data */
    m_data.pop_front();
}

uint32_t UncompressedFile::defaultLogContainerSize() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_defaultLogContainerSize;
}

void UncompressedFile::setDefaultLogContainerSize(uint32_t defaultLogContainerSize) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* set default log container size */
    m_defaultLogContainerSize = defaultLogContainerSize;
}

std::shared_ptr<LogContainer> UncompressedFile::logContainerContaining(const std::streampos pos) const {
    /* find logContainer that contains file position */
    std::list<std::shared_ptr<LogContainer>>::const_iterator result = std::find_if(m_data.cbegin(), m_data.cend(), [&pos](std::shared_ptr<LogContainer> logContainer) {
        return
            (pos >= logContainer->filePosition) &&
            (pos < logContainer->uncompressedFileSize + logContainer->filePosition);
    });

    /* if found, return logContainer */
    if (result != m_data.cend()) {
        return *result;
    }

    /* otherwise return nullptr */
    return nullptr;
}

}
}

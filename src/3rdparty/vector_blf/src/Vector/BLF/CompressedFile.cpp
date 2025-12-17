// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CompressedFile.h>

namespace Vector {
namespace BLF {

CompressedFile::~CompressedFile() {
    close();
}

std::streamsize CompressedFile::gcount() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_file.gcount();
}

void CompressedFile::read(char * s, std::streamsize n) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_file.read(s, n);
}

std::streampos CompressedFile::tellg() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_file.tellg();
}

void CompressedFile::seekg(std::streamoff off, const std::ios_base::seekdir way) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_file.seekg(off, way);
}

void CompressedFile::write(const char * s, std::streamsize n) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_file.write(s, n);
}

std::streampos CompressedFile::tellp() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_file.tellp();
}

bool CompressedFile::good() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_file.good();
}

bool CompressedFile::eof() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_file.eof();
}

void CompressedFile::open(const char * filename, std::ios_base::openmode openMode) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_file.open(filename, openMode);
}

bool CompressedFile::is_open() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_file.is_open();
}

void CompressedFile::close() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_file.close();
}

void CompressedFile::seekp(std::streampos pos) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    m_file.seekp(pos);
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/ObjectQueue.h>

#include <Vector/BLF/Exceptions.h>
#include <Vector/BLF/LogContainer.h>

namespace Vector {
namespace BLF {

template<typename T>
ObjectQueue<T>::~ObjectQueue() {
    abort();

    /* delete elements in queue */
    while (!m_queue.empty()) {
        delete m_queue.front();
        m_queue.pop();
    }
}

template<typename T>
T * ObjectQueue<T>::read() {
    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* wait for data */
    tellpChanged.wait(lock, [&] {
        return
        m_abort ||
        !m_queue.empty() ||
        (m_tellg >= m_fileSize);
    });

    /* get first entry */
    T * ohb = nullptr;
    if (m_queue.empty())
        m_rdstate = std::ios_base::eofbit | std::ios_base::failbit;
    else {
        ohb = m_queue.front();
        m_queue.pop();

        /* set state */
        m_rdstate = std::ios_base::goodbit;

        /* increase get count */
        m_tellg++;
    }

    /* notify */
    tellgChanged.notify_all();

    return ohb;
}

template<typename T>
uint32_t ObjectQueue<T>::tellg() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_tellg;
}

template<typename T>
void ObjectQueue<T>::write(T * obj) {
    /* mutex lock */
    std::unique_lock<std::mutex> lock(m_mutex);

    /* wait for free space */
    tellgChanged.wait(lock, [&] {
        return
        m_abort ||
        static_cast<uint32_t>(m_queue.size()) < m_bufferSize;
    });

    /* push data */
    m_queue.push(obj);

    /* increase put count */
    m_tellp++;

    /* shift eof */
    if (m_tellp > m_fileSize)
        m_fileSize = m_tellp;

    /* notify */
    tellpChanged.notify_all();
}

template<typename T>
uint32_t ObjectQueue<T>::tellp() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return m_tellp;
}

template<typename T>
bool ObjectQueue<T>::good() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return (m_rdstate == std::ios_base::goodbit);
}

template<typename T>
bool ObjectQueue<T>::eof() const {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    return (m_rdstate & std::ios_base::eofbit);
}

template<typename T>
void ObjectQueue<T>::abort() {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* stop */
    m_abort = true;

    /* trigger blocked threads */
    tellgChanged.notify_all();
    tellpChanged.notify_all();
}

template<typename T>
void ObjectQueue<T>::setFileSize(uint32_t fileSize) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* set object count */
    m_fileSize = fileSize;

    /* notify */
    tellpChanged.notify_all();
}

template<typename T>
void ObjectQueue<T>::setBufferSize(uint32_t bufferSize) {
    /* mutex lock */
    std::lock_guard<std::mutex> lock(m_mutex);

    /* set max size */
    m_bufferSize = bufferSize;
}

template class ObjectQueue<ObjectHeaderBase>;

}
}

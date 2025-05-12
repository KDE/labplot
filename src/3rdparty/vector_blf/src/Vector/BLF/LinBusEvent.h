// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * Common header of LIN bus events
 */
struct VECTOR_BLF_EXPORT LinBusEvent {
    LinBusEvent() = default;
    virtual ~LinBusEvent() noexcept = default;
    LinBusEvent(const LinBusEvent &) = default;
    LinBusEvent & operator=(const LinBusEvent &) = default;
    LinBusEvent(LinBusEvent &&) = default;
    LinBusEvent & operator=(LinBusEvent &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(AbstractFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(AbstractFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual uint32_t calculateObjectSize() const;

    /**
     * @brief Start Of Frame timestamp
     *
     * Timestamp of frame/event start
     */
    uint64_t sof {};

    /**
     * @brief Baudrate of the event in bit/sec
     *
     * Baudrate of frame/event in bit/sec
     */
    uint32_t eventBaudrate {};

    /**
     * @brief application channel
     *
     * Channel number where the frame/event notified
     */
    uint16_t channel {};

    /** reserved */
    uint16_t reservedLinBusEvent {};
};

}
}

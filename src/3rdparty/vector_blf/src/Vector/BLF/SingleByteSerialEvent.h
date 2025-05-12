// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>
#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * single byte serial event
 */
struct VECTOR_BLF_EXPORT SingleByteSerialEvent final {
    SingleByteSerialEvent() = default;
    virtual ~SingleByteSerialEvent() noexcept = default;
    SingleByteSerialEvent(const SingleByteSerialEvent &) = default;
    SingleByteSerialEvent & operator=(const SingleByteSerialEvent &) = default;
    SingleByteSerialEvent(SingleByteSerialEvent &&) = default;
    SingleByteSerialEvent & operator=(SingleByteSerialEvent &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(AbstractFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(AbstractFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual uint32_t calculateObjectSize() const;

    /** single byte */
    uint8_t byte {};
};

}
}

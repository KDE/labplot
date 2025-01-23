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
 * compact serial event
 */
struct VECTOR_BLF_EXPORT CompactSerialEvent final {
    CompactSerialEvent() = default;
    virtual ~CompactSerialEvent() noexcept = default;
    CompactSerialEvent(const CompactSerialEvent &) = default;
    CompactSerialEvent & operator=(const CompactSerialEvent &) = default;
    CompactSerialEvent(CompactSerialEvent &&) = default;
    CompactSerialEvent & operator=(CompactSerialEvent &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(AbstractFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(AbstractFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual uint32_t calculateObjectSize() const;

    /** compact length */
    uint8_t compactLength {};

    /** compact data */
    std::array<uint8_t, 15> compactData {};
};

}
}

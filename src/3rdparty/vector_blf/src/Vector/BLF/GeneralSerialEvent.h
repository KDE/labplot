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
 * general serial event
 */
struct VECTOR_BLF_EXPORT GeneralSerialEvent final {
    GeneralSerialEvent() = default;
    virtual ~GeneralSerialEvent() noexcept = default;
    GeneralSerialEvent(const GeneralSerialEvent &) = default;
    GeneralSerialEvent & operator=(const GeneralSerialEvent &) = default;
    GeneralSerialEvent(GeneralSerialEvent &&) = default;
    GeneralSerialEvent & operator=(GeneralSerialEvent &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(AbstractFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(AbstractFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual uint32_t calculateObjectSize() const;

    /**
     * @brief length of variable data in bytes
     *
     * length of variable data in bytes
     */
    uint32_t dataLength {};

    /**
     * @brief length of variable timestamps in bytes
     *
     * length of variable timestamps in bytes
     */
    uint32_t timeStampsLength {};

    /** reserved */
    uint64_t reservedGeneralSerialEvent {};

    /**
     * @brief variable data
     *
     * variable data
     */
    std::vector<uint8_t> data {};

    /**
     * @brief variable timestamps (optional)
     *
     * variable timestamps (optional)
     */
    std::vector<int64_t> timeStamps {};
};

}
}

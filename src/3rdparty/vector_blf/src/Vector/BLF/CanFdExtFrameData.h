// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/** optional information in CanFdMessage64 and CanFdErrorFrame64 classes */
struct VECTOR_BLF_EXPORT CanFdExtFrameData {
    CanFdExtFrameData() = default;
    virtual ~CanFdExtFrameData() noexcept = default;
    CanFdExtFrameData(const CanFdExtFrameData &) = default;
    CanFdExtFrameData & operator=(const CanFdExtFrameData &) = default;
    CanFdExtFrameData(CanFdExtFrameData &&) = default;
    CanFdExtFrameData & operator=(CanFdExtFrameData &&) = default;

    /** @copydoc ObjectHeader::read */
    virtual void read(AbstractFile & is);

    /** @copydoc ObjectHeader::write */
    virtual void write(AbstractFile & os);

    /** @copydoc ObjectHeader::calculateObjectSize */
    virtual uint32_t calculateObjectSize() const;

    /**
     * bit rate in arbitration phase
     *
     * - Bit 0-7: TSEG1-1
     * - Bit 8-15: TSEG2-1
     * - Bit 16-27: Prescaler
     * - Bit 28-31: Quartz Frequency (enumeration). Supported values:
     *   - 0: 16 MHz
     *   - 1: 32 MHz
     *   - 2: 80 MHz
     */
    uint32_t btrExtArb {};

    /**
     * bit rate in data phase
     *
     * - Bit 0-7: TSEG1-1
     * - Bit 8-15: TSEG2-1
     * - Bit 16-27: Prescaler
     * - Bit 28-31: Quartz Frequency (enumeration). Supported values:
     *   - 0: 16 MHz
     *   - 1: 32 MHz
     *   - 2: 80 MHz
     */
    uint32_t btrExtData {};

    /** reserved */
    std::vector<uint8_t> reservedCanFdExtFrameData {};

    // @note might be extended in future versions
};

}
}

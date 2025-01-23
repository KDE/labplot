// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/LinDatabyteTimestampEvent.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief LIN_SHORT_OR_SLOW_RESPONSE2
 */
struct VECTOR_BLF_EXPORT LinShortOrSlowResponse2 final : ObjectHeader, LinDatabyteTimestampEvent {
    LinShortOrSlowResponse2();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief number of valid response bytes
     *
     * The number of response bytes.
     */
    uint32_t numberOfRespBytes {};

    /**
     * @brief the response bytes (can include the checksum)
     *
     * The response bytes (can include the
     * checksum).
     */
    std::array<uint8_t, 9> respBytes {};

    /**
     * @brief non-zero, if the response was too slow
     *
     * Non-zero, if the response was too
     * slow; otherwise zero.
     */
    uint8_t slowResponse {};

    /**
     * @brief non-zero, if the response was interrupted by a sync break
     *
     * Non-zero, if the response was
     * interrupted by a sync break;
     * otherwise zero.
     */
    uint8_t interruptedByBreak {};

    /** reserved */
    uint8_t reservedLinShortOrSlowResponse1 {};

    /**
     * @brief Exact baudrate of the header in bit/sec
     *
     * Event's baudrate measured in
     * header [in bits/sec]
     */
    double exactHeaderBaudrate {};

    /**
     * @brief Early stop bit offset for UART timestamps in ns
     *
     * Early stop bit offset in frame
     * header for UART timestamps
     * [in ns]
     */
    uint32_t earlyStopbitOffset {};

    /** reserved */
    uint32_t reservedLinShortOrSlowResponse2 {};
};

}
}

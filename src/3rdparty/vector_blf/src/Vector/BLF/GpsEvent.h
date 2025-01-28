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
 * @brief GPS_EVENT
 *
 * GPS event.
 */
struct VECTOR_BLF_EXPORT GpsEvent final : ObjectHeader {
    GpsEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * Not used, must be 0.
     */
    uint32_t flags {};

    /**
     * @brief channel of event
     *
     * GPS channel the GPS event was sent.
     */
    uint16_t channel {};

    /**
     * Reserved, must be 0.
     */
    uint16_t reservedGpsEvent {};

    /**
     * Latitude, possible values reach from -180 to 180.
     *
     * Negative values are western hemisphere, positive
     * values are eastern hemisphere.
     */
    double latitude {};

    /**
     * Longitude, possible values reach from -90 to 90.
     * Negative values are Southern hemisphere,
     * positive values are northern hemisphere.
     */
    double longitude {};

    /**
     * Altitude in meters, measured above sea line.
     */
    double altitude {};

    /**
     * Current vehicle speed in km/h.
     */
    double speed {};

    /**
     * Current driving course, possible values reach
     * from -180 to 180. A value of 0 means driving
     * north, 90 means driving east, -90 means driving
     * west, -180 and 180 mean driving south.
     */
    double course {};
};

}
}

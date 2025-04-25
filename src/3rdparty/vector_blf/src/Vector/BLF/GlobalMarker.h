// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <string>

#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief GLOBAL_MARKER
 *
 * Global Marker assigned to another event or to a time stamp.
 */
struct VECTOR_BLF_EXPORT GlobalMarker final : ObjectHeader {
    GlobalMarker();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief commented event type
     *
     * Type of the commented events
     */
    uint32_t commentedEventType {};

    /**
     * Foreground color of the marker group.
     */
    uint32_t foregroundColor {};

    /**
     * Background color of the marker group.
     */
    uint32_t backgroundColor {};

    /**
     * Defines whether a marker can be relocated
     */
    uint8_t isRelocatable {};

    /** reserved */
    uint8_t reservedGlobalMarker1 {};

    /** reserved */
    uint16_t reservedGlobalMarker2 {};

    /**
     * @brief group name length in bytes
     *
     * Length of groupName without ending 0.
     */
    uint32_t groupNameLength {};

    /**
     * @brief marker name length in bytes
     *
     * Length of markerName without ending 0.
     */
    uint32_t markerNameLength {};

    /**
     * @brief description length in bytes
     *
     * Length of description without ending 0.
     */
    uint32_t descriptionLength {};

    /** reserved */
    uint32_t reservedGlobalMarker3 {};

    /** reserved */
    uint64_t reservedGlobalMarker4 {};

    /**
     * @brief group name
     *
     * Group name.
     */
    std::string groupName {};

    /**
     * @brief marker name
     *
     * Marker.
     */
    std::string markerName {};

    /**
     * @brief description
     *
     * Description text.
     */
    std::string description {};
};

}
}

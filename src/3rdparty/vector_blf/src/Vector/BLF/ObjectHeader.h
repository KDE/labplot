// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include <Vector/BLF/platform.h>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeaderBase.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief Object header
 *
 * Object header. Version 1.
 */
struct VECTOR_BLF_EXPORT ObjectHeader : ObjectHeaderBase {
    ObjectHeader(const ObjectType objectType, const uint16_t objectVersion = 0);

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint16_t calculateHeaderSize() const override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for objectFlags */
    enum ObjectFlags : uint32_t {
        /**
         * @brief 10 micro second timestamp
         *
         * Object time stamp is saved as multiple of ten
         * microseconds.
         * (BL_OBJ_FLAG_TIME_TEN_MICS)
         */
        TimeTenMics = 0x00000001,

        /**
         * @brief 1 nano second timestamp
         *
         * Object time stamp is saved in nanoseconds.
         * (BL_OBJ_FLAG_TIME_ONE_NANS)
         */
        TimeOneNans = 0x00000002
    };

    /**
     * @brief object flags
     *
     * Unit of object timestamp.
     */
    uint32_t objectFlags {ObjectFlags::TimeOneNans};

    /**
     * @brief client index of send node
     */
    uint16_t clientIndex {};

    /**
     * @brief object specific version
     *
     * Object specific version, has to be set to 0 unless
     * stated otherwise in the description of a specific
     * event.
     *
     * @note can be set in event class constructor
     */
    uint16_t objectVersion {0};

    /**
     * @brief object timestamp
     *
     * Time stamp of this object in the unit specified in
     * objectFlags.
     */
    uint64_t objectTimeStamp {};
};

}
}

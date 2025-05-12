// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <Vector/BLF/platform.h>

#include <string>
#include <vector>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/ObjectHeader.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * @brief SYS_VARIABLE
 *
 * System variable that can be used with CANoe.
 */
struct VECTOR_BLF_EXPORT SystemVariable final : ObjectHeader {
    SystemVariable();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for type */
    enum Type : uint32_t {
        /** DOUBLE */
        Double = 1,

        /** LONG */
        Long = 2,

        /** STRING */
        String = 3,

        /** Array of DOUBLE */
        DoubleArray = 4,

        /** Array of LONG */
        LongArray = 5,

        /** LONGLONG */
        LongLong = 6,

        /** Array of BYTE */
        ByteArray = 7
    };

    /**
     * @brief type of system variable
     */
    uint32_t type {};

    /**
     * @brief signed, later perhaps also string codepage
     */
    uint32_t representation {};

    /** reserved */
    uint64_t reservedSystemVariable1 {};

    /**
     * @brief length of variable name in bytes
     *
     * Length of the name of the system variable
     * (without terminating 0)
     */
    uint32_t nameLength {};

    /**
     * @brief length of variable data in bytes
     *
     * Length of the data of the environment variable in
     * bytes.
     */
    uint32_t dataLength {};

    /** reserved */
    uint64_t reservedSystemVariable2 {};

    /**
     * @brief variable name in MBCS
     *
     * Name of the system variable.
     */
    std::string name {};

    /**
     * @brief variable data
     *
     * Data value of the system variable.
     */
    std::vector<uint8_t> data {};
};

}
}

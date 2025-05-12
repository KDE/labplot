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
 * @brief ENV_INTEGER, ENV_DOUBLE, ENV_STRING, ENV_DATA
 *
 * Environment variable that can be used with CANoe.
 */
struct VECTOR_BLF_EXPORT EnvironmentVariable final : ObjectHeader {
    EnvironmentVariable(/*const ObjectType objectType*/);

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief length of variable name in bytes
     *
     * Length of the name of the environment variable
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
    uint64_t reservedEnvironmentVariable {};

    /**
     * @brief variable name in MBCS
     *
     * Name of the environment variable.
     */
    std::string name {};

    /**
     * @brief variable data
     *
     * Data value of the environment variable.
     */
    std::vector<uint8_t> data {};
};

}
}

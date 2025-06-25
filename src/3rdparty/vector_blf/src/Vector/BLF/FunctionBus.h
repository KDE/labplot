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

/**
 * @brief FUNCTION_BUS
 */
struct VECTOR_BLF_EXPORT FunctionBus final : ObjectHeader {
    FunctionBus();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /** enumeration for functionBusObjectType */
    enum FunctionBusObjectType : uint32_t {
        Undefined = 0,
        Signal = 1,
        ServiceFunction = 2,
        State = 3
    };

    /** type of system variable */
    uint32_t functionBusObjectType {};

    uint32_t veType {};

    /** length of variable name in bytes */
    uint32_t nameLength {};

    /** length of variable data in bytes */
    uint32_t dataLength {};

    /** path name in the port server */
    std::string name {};

    /** variable data */
    std::vector<uint8_t> data {};
};

}
}

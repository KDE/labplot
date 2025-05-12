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

struct VECTOR_BLF_EXPORT AttributeEvent final : ObjectHeader {
    AttributeEvent();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /* static */

    /** @brief length of variable mainAttributableObjectPath in bytes */
    uint32_t mainAttributableObjectPathLength;

    /** @brief length of variable memberPath in bytes */
    uint32_t memberPathLength;

    /** @brief length of variable attributeDefinitionPath in bytes */
    uint32_t attributeDefinitionPathLength;

    /** @brief length of variable data in bytes */
    uint32_t dataLength;

    /* dynamic */

    /** @brief path of the main attributable object */
    std::string mainAttributableObjectPath;

    /** @brief path of the member (optional) */
    std::string memberPath;

    /** @brief path of the attribute definition */
    std::string attributeDefinitionPath;

    /** @brief variable data */
    std::vector<uint8_t> data;
};

}
}

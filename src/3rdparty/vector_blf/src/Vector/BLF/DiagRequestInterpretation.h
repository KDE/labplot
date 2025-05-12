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
 * @brief DIAG_REQUEST_INTERPRETATION
 */
struct VECTOR_BLF_EXPORT DiagRequestInterpretation final : ObjectHeader {
    DiagRequestInterpretation();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief unique ID identifying the used diagnostic description
     */
    uint32_t diagDescriptionHandle {};

    /**
     * @brief unique ID identifying the used diagnostic variant
     */
    uint32_t diagVariantHandle {};

    /**
     * @brief unique ID identifying the used diagnostic service
     */
    uint32_t diagServiceHandle {};

    /**
     * @brief string length for ecuQualifier
     */
    uint32_t ecuQualifierLength {};

    /**
     * @brief string length for variantQualifier
     */
    uint32_t variantQualifierLength {};

    /**
     * @brief string length for serviceQualifier
     */
    uint32_t serviceQualifierLength {};

    /**
     * @brief qualifier of the ECU the request was sent to
     */
    std::string ecuQualifier {};

    /**
     * @brief qualifier of the active diagnostic variant
     */
    std::string variantQualifier {};

    /**
     * @brief qualifier of the diagnostic service
     */
    std::string serviceQualifier {};
};

}
}

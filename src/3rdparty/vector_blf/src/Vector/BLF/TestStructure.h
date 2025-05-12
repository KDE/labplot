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
 * @brief TEST_STRUCTURE
 */
struct VECTOR_BLF_EXPORT TestStructure final : ObjectHeader {
    TestStructure();

    void read(AbstractFile & is) override;
    void write(AbstractFile & os) override;
    uint32_t calculateObjectSize() const override;

    /**
     * @brief unique ID identifying the executing test module or test configuration
     */
    uint32_t executionObjectIdentify {};

    /** enumeration for type */
    enum Type : uint16_t {
        TM_TESTMODULE = 1,
        TM_TESTGROUP = 2,
        TM_TESTCASE = 3,
        TESTCONFIGURATION = 8,
        TESTUNIT = 9,
        TESTGROUP = 10,
        TESTFIXTURE = 11,
        TESTSEQUENCE = 12,
        TESTSEQUENCELIST = 13,
        TESTCASE = 14,
        TESTCASELIST = 15
    };

    /**
     * @brief type of structure element, see BL_TESTSTRUCT_TYPE_xxx
     */
    uint16_t type {};

    /** reserved */
    uint16_t reservedTestStructure {};

    /**
     * @brief unique number of structure element (in this test run, transitive, can be used to correlate begin/end events)
     */
    uint32_t uniqueNo {};

    /** enumeration for action */
    enum Action : uint16_t {
        BEGIN = 1,
        END = 2,

        /**
         * early abortion of test execution (due to e.g. verdict impact, user stop or failed assure pattern),
         * always correlated to test module / test configuration and followed by "end" action
         */
        ABORT = 3
    };

    /**
     * @brief indicates begin/end of structure element, see BL_TESTSTRUCT_ACTION_xxx
     */
    uint16_t action {};

    /** enumeration for result */
    enum Result : uint16_t {
        UNDEFINED = 0,
        NONE = 1,
        PASSED = 2,
        INCONCLUSIVE = 3,
        FAILED = 4,
        ERRORINTESTSYSTEM = 5
    };

    /**
     * @brief overall result (verdict) for end of structure element events
     */
    uint16_t result {};

    /**
     * @brief string length in wchar_t's for executingObjectName
     */
    uint32_t executingObjectNameLength {};

    /**
     * @brief string length in wchar_t's for name
     */
    uint32_t nameLength {};

    /**
     * @brief string length in wchar_t's for text
     */
    uint32_t textLength {};

    /**
     * @brief name of the executing test module or test configuration as shown in CANoe (wchar_t)
     */
    std::u16string executingObjectName {};

    /**
     * @brief name of structure element (can change between begin/end when using CAPL function TestCaseTitle or similar (wchar_t)
     */
    std::u16string name {};

    /**
     * @brief full informational text for event as it appears in CANoe trace window
     */
    std::u16string text {};
};

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include <Vector/BLF/platform.h>

#include <array>

#include <Vector/BLF/AbstractFile.h>

#include <Vector/BLF/vector_blf_export.h>

namespace Vector {
namespace BLF {

/**
 * File signature
 */
const uint32_t FileSignature = 0x47474F4C; /* LOGG */

/**
 * Application ID
 */
enum ApplicationId : uint8_t {
    /** Unknown */
    Unknown = 0,

    /** CANalyzer */
    Canalyzer = 1,

    /** CANoe */
    Canoe = 2,

    /** CANstress */
    Canstress = 3,

    /** CANlog */
    Canlog = 4,

    /** CANape */
    Canape = 5,

    /** CANcaseXL log */
    Cancasexllog = 6,

    /** Vector Logger Configurator */
    Vlconfig = 7,

    /** Porsche Logger */
    Porschelogger = 200,

    /** CAETEC Logger */
    Caeteclogger = 201,

    /** Vector Network Simulator */
    Vectornetworksimulator = 202,

    /** IPETRONIK Logger */
    Ipetroniklogger=203,

    /** RT PK */
    RtPk=204,

    /** PikeTec */
    Piketec=205,

    /** Sparks */
    Sparks=206
};

/** system time */
struct SYSTEMTIME {
    /** year */
    uint16_t year;

    /** month */
    uint16_t month;

    /** day of week (0=Sunday, 6=Saturday) */
    uint16_t dayOfWeek;

    /** day */
    uint16_t day;

    /** hour */
    uint16_t hour;

    /** minute */
    uint16_t minute;

    /** second */
    uint16_t second;

    /** milliseconds */
    uint16_t milliseconds;
};

/**
 * File statistics
 */
struct VECTOR_BLF_EXPORT FileStatistics final {
    FileStatistics() = default;
    virtual ~FileStatistics() noexcept = default;
    FileStatistics(const FileStatistics &) = default;
    FileStatistics & operator=(const FileStatistics &) = default;
    FileStatistics(FileStatistics &&) = default;
    FileStatistics & operator=(FileStatistics &&) = default;

    /**
     * read file statistics
     *
     * @param is input stream
     */
    virtual void read(AbstractFile & is);

    /**
     * write file statistics
     *
     * @param os output stream
     */
    virtual void write(AbstractFile & os);

    /**
     * Calculates the statisticsSize
     *
     * @return statistics size
     */
    uint32_t calculateStatisticsSize() const;

    /** signature (signature) */
    uint32_t signature {FileSignature};

    /** sizeof(FileStatistics) */
    uint32_t statisticsSize {calculateStatisticsSize()};

    /**
     * BL API number
     *
     * This consists of major, minor, build, patch.
     * It's encoded as such: major * 1000000 + minor * 1000 + build * 100 + patch.
     * Example: 4010608 decodes to 4.1.6.8.
     */
    uint32_t apiNumber { 4080200 }; // 4.8.2.0

    /** application ID */
    uint8_t applicationId {};

    /**
     * compression level
     *
     * @note
     *   zlib defines maximum compression level 9.
     *   All files that show 10 here, actually have LogContainers with
     *   compression level 6, which is default for Vector BLF.
     */
    uint8_t compressionLevel {1};

    /** application major number */
    uint8_t applicationMajor {};

    /** application minor number */
    uint8_t applicationMinor {};

    /** (compressed) file size in bytes */
    uint64_t fileSize {};

    /** uncompressed file size in bytes */
    uint64_t uncompressedFileSize {};

    /** number of objects */
    uint32_t objectCount {};

    /**
     * application build number
     *
     * @todo The BL API function takes an uint8_t argument here.
     */
    uint32_t applicationBuild {};

    /** measurement start time */
    SYSTEMTIME measurementStartTime {};

    /** last object time */
    SYSTEMTIME lastObjectTime {};

    /**
     * @note
     *   The following variables are based on observations, as there is no
     *   public documentation available.
     */

    /**
     * This variable designates the file position of the (first) LogContainer
     * that contains the RestorePointContainer objects.
     *
     * If RestorePoints are not used, this defaults to 0.
     *
     * @see RestorePointContainer
     */
    uint64_t restorePointsOffset {};

    /** reserved */
    std::array<uint32_t, 16> reservedFileStatistics {};
};

}
}

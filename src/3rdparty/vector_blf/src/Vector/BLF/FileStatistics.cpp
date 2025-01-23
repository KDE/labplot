// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FileStatistics.h>

#include <cstdint>
#include <string>

#include <Vector/BLF/AbstractFile.h>
#include <Vector/BLF/Exceptions.h>

namespace Vector {
namespace BLF {

void FileStatistics::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&signature), sizeof(signature));
    if (signature != FileSignature)
        throw Exception("FileStatistics::read(): File signature doesn't match at this position.");
    is.read(reinterpret_cast<char *>(&statisticsSize), sizeof(statisticsSize));
    is.read(reinterpret_cast<char *>(&apiNumber), sizeof(apiNumber));
    is.read(reinterpret_cast<char *>(&applicationId), sizeof(applicationId));
    is.read(reinterpret_cast<char *>(&compressionLevel), sizeof(compressionLevel));
    is.read(reinterpret_cast<char *>(&applicationMajor), sizeof(applicationMajor));
    is.read(reinterpret_cast<char *>(&applicationMinor), sizeof(applicationMinor));
    is.read(reinterpret_cast<char *>(&fileSize), sizeof(fileSize));
    is.read(reinterpret_cast<char *>(&uncompressedFileSize), sizeof(uncompressedFileSize));
    is.read(reinterpret_cast<char *>(&objectCount), sizeof(objectCount));
    is.read(reinterpret_cast<char *>(&applicationBuild), sizeof(applicationBuild));
    is.read(reinterpret_cast<char *>(&measurementStartTime), sizeof(measurementStartTime));
    is.read(reinterpret_cast<char *>(&lastObjectTime), sizeof(lastObjectTime));
    is.read(reinterpret_cast<char *>(&restorePointsOffset), sizeof(restorePointsOffset));
    is.read(reinterpret_cast<char *>(reservedFileStatistics.data()), static_cast<std::streamsize>(reservedFileStatistics.size() * sizeof(uint32_t)));
}

void FileStatistics::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&signature), sizeof(signature));
    os.write(reinterpret_cast<char *>(&statisticsSize), sizeof(statisticsSize));
    os.write(reinterpret_cast<char *>(&apiNumber), sizeof(apiNumber));
    os.write(reinterpret_cast<char *>(&applicationId), sizeof(applicationId));
    os.write(reinterpret_cast<char *>(&compressionLevel), sizeof(compressionLevel));
    os.write(reinterpret_cast<char *>(&applicationMajor), sizeof(applicationMajor));
    os.write(reinterpret_cast<char *>(&applicationMinor), sizeof(applicationMinor));
    os.write(reinterpret_cast<char *>(&fileSize), sizeof(fileSize));
    os.write(reinterpret_cast<char *>(&uncompressedFileSize), sizeof(uncompressedFileSize));
    os.write(reinterpret_cast<char *>(&objectCount), sizeof(objectCount));
    os.write(reinterpret_cast<char *>(&applicationBuild), sizeof(applicationBuild));
    os.write(reinterpret_cast<char *>(&measurementStartTime), sizeof(measurementStartTime));
    os.write(reinterpret_cast<char *>(&lastObjectTime), sizeof(lastObjectTime));
    os.write(reinterpret_cast<char *>(&restorePointsOffset), sizeof(restorePointsOffset));
    os.write(reinterpret_cast<char *>(reservedFileStatistics.data()), static_cast<std::streamsize>(reservedFileStatistics.size() * sizeof(uint32_t)));
}

uint32_t FileStatistics::calculateStatisticsSize() const {
    return
        sizeof(signature) +
        sizeof(statisticsSize) +
        sizeof(apiNumber) +
        sizeof(applicationId) +
        sizeof(compressionLevel) +
        sizeof(applicationMajor) +
        sizeof(applicationMinor) +
        sizeof(fileSize) +
        sizeof(uncompressedFileSize) +
        sizeof(objectCount) +
        sizeof(applicationBuild) +
        sizeof(measurementStartTime) +
        sizeof(lastObjectTime) +
        sizeof(restorePointsOffset) +
        static_cast<uint32_t>(reservedFileStatistics.size() * sizeof(uint32_t));
}

}
}

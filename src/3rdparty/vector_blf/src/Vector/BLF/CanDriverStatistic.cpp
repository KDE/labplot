// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanDriverStatistic.h>

namespace Vector {
namespace BLF {

CanDriverStatistic::CanDriverStatistic() :
    ObjectHeader(ObjectType::CAN_STATISTIC) {
}

void CanDriverStatistic::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&busLoad), sizeof(busLoad));
    is.read(reinterpret_cast<char *>(&standardDataFrames), sizeof(standardDataFrames));
    is.read(reinterpret_cast<char *>(&extendedDataFrames), sizeof(extendedDataFrames));
    is.read(reinterpret_cast<char *>(&standardRemoteFrames), sizeof(standardRemoteFrames));
    is.read(reinterpret_cast<char *>(&extendedRemoteFrames), sizeof(extendedRemoteFrames));
    is.read(reinterpret_cast<char *>(&errorFrames), sizeof(errorFrames));
    is.read(reinterpret_cast<char *>(&overloadFrames), sizeof(overloadFrames));
    is.read(reinterpret_cast<char *>(&reservedCanDriverStatistic), sizeof(reservedCanDriverStatistic));
}

void CanDriverStatistic::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&busLoad), sizeof(busLoad));
    os.write(reinterpret_cast<char *>(&standardDataFrames), sizeof(standardDataFrames));
    os.write(reinterpret_cast<char *>(&extendedDataFrames), sizeof(extendedDataFrames));
    os.write(reinterpret_cast<char *>(&standardRemoteFrames), sizeof(standardRemoteFrames));
    os.write(reinterpret_cast<char *>(&extendedRemoteFrames), sizeof(extendedRemoteFrames));
    os.write(reinterpret_cast<char *>(&errorFrames), sizeof(errorFrames));
    os.write(reinterpret_cast<char *>(&overloadFrames), sizeof(overloadFrames));
    os.write(reinterpret_cast<char *>(&reservedCanDriverStatistic), sizeof(reservedCanDriverStatistic));
}

uint32_t CanDriverStatistic::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(busLoad) +
        sizeof(standardDataFrames) +
        sizeof(extendedDataFrames) +
        sizeof(standardRemoteFrames) +
        sizeof(extendedRemoteFrames) +
        sizeof(errorFrames) +
        sizeof(overloadFrames) +
        sizeof(reservedCanDriverStatistic);
}

}
}

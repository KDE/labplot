// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/A429Status.h>

namespace Vector {
namespace BLF {

A429Status::A429Status() :
    ObjectHeader(ObjectType::A429_STATUS) {
}

void A429Status::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedA429Status1), sizeof(reservedA429Status1));
    is.read(reinterpret_cast<char *>(&parity), sizeof(parity));
    is.read(reinterpret_cast<char *>(&reservedA429Status2), sizeof(reservedA429Status2));
    is.read(reinterpret_cast<char *>(&minGap), sizeof(minGap));
    is.read(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    is.read(reinterpret_cast<char *>(&minBitrate), sizeof(minBitrate));
    is.read(reinterpret_cast<char *>(&maxBitrate), sizeof(maxBitrate));
}

void A429Status::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedA429Status1), sizeof(reservedA429Status1));
    os.write(reinterpret_cast<char *>(&parity), sizeof(parity));
    os.write(reinterpret_cast<char *>(&reservedA429Status2), sizeof(reservedA429Status2));
    os.write(reinterpret_cast<char *>(&minGap), sizeof(minGap));
    os.write(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    os.write(reinterpret_cast<char *>(&minBitrate), sizeof(minBitrate));
    os.write(reinterpret_cast<char *>(&maxBitrate), sizeof(maxBitrate));
}

uint32_t A429Status::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedA429Status1) +
        sizeof(parity) +
        sizeof(reservedA429Status2) +
        sizeof(minGap) +
        sizeof(bitrate) +
        sizeof(minBitrate) +
        sizeof(maxBitrate);
}

}
}

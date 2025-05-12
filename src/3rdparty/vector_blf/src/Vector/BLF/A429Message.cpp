// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/A429Message.h>

namespace Vector {
namespace BLF {

A429Message::A429Message() :
    ObjectHeader(ObjectType::A429_MESSAGE) {
}

void A429Message::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(a429Data.data()), static_cast<std::streamsize>(a429Data.size()));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedA429Message1), sizeof(reservedA429Message1));
    is.read(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    is.read(reinterpret_cast<char *>(&errReason), sizeof(errReason));
    is.read(reinterpret_cast<char *>(&errPosition), sizeof(errPosition));
    is.read(reinterpret_cast<char *>(&reservedA429Message2), sizeof(reservedA429Message2));
    is.read(reinterpret_cast<char *>(&reservedA429Message3), sizeof(reservedA429Message3));
    is.read(reinterpret_cast<char *>(&frameGap), sizeof(frameGap));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&msgCtrl), sizeof(msgCtrl));
    is.read(reinterpret_cast<char *>(&reservedA429Message4), sizeof(reservedA429Message4));
    is.read(reinterpret_cast<char *>(&cycleTime), sizeof(cycleTime));
    is.read(reinterpret_cast<char *>(&error), sizeof(error));
    is.read(reinterpret_cast<char *>(&bitLenOfLastBit), sizeof(bitLenOfLastBit));
    is.read(reinterpret_cast<char *>(&reservedA429Message5), sizeof(reservedA429Message5));
}

void A429Message::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(a429Data.data()), static_cast<std::streamsize>(a429Data.size()));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedA429Message1), sizeof(reservedA429Message1));
    os.write(reinterpret_cast<char *>(&bitrate), sizeof(bitrate));
    os.write(reinterpret_cast<char *>(&errReason), sizeof(errReason));
    os.write(reinterpret_cast<char *>(&errPosition), sizeof(errPosition));
    os.write(reinterpret_cast<char *>(&reservedA429Message2), sizeof(reservedA429Message2));
    os.write(reinterpret_cast<char *>(&reservedA429Message3), sizeof(reservedA429Message3));
    os.write(reinterpret_cast<char *>(&frameGap), sizeof(frameGap));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&msgCtrl), sizeof(msgCtrl));
    os.write(reinterpret_cast<char *>(&reservedA429Message4), sizeof(reservedA429Message4));
    os.write(reinterpret_cast<char *>(&cycleTime), sizeof(cycleTime));
    os.write(reinterpret_cast<char *>(&error), sizeof(error));
    os.write(reinterpret_cast<char *>(&bitLenOfLastBit), sizeof(bitLenOfLastBit));
    os.write(reinterpret_cast<char *>(&reservedA429Message5), sizeof(reservedA429Message5));
}

uint32_t A429Message::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        static_cast<uint32_t>(a429Data.size()) +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedA429Message1) +
        sizeof(bitrate) +
        sizeof(errReason) +
        sizeof(errPosition) +
        sizeof(reservedA429Message2) +
        sizeof(reservedA429Message3) +
        sizeof(frameGap) +
        sizeof(frameLength) +
        sizeof(msgCtrl) +
        sizeof(reservedA429Message4) +
        sizeof(cycleTime) +
        sizeof(error) +
        sizeof(bitLenOfLastBit) +
        sizeof(reservedA429Message5);
}

}
}

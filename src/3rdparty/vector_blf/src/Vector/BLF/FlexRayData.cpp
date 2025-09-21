// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayData.h>

namespace Vector {
namespace BLF {

FlexRayData::FlexRayData() :
    ObjectHeader(ObjectType::FLEXRAY_DATA) {
}

void FlexRayData::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&mux), sizeof(mux));
    is.read(reinterpret_cast<char *>(&len), sizeof(len));
    is.read(reinterpret_cast<char *>(&messageId), sizeof(messageId));
    is.read(reinterpret_cast<char *>(&crc), sizeof(crc));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedFlexRayData1), sizeof(reservedFlexRayData1));
    is.read(reinterpret_cast<char *>(&reservedFlexRayData2), sizeof(reservedFlexRayData2));
    is.read(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
}

void FlexRayData::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&mux), sizeof(mux));
    os.write(reinterpret_cast<char *>(&len), sizeof(len));
    os.write(reinterpret_cast<char *>(&messageId), sizeof(messageId));
    os.write(reinterpret_cast<char *>(&crc), sizeof(crc));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedFlexRayData1), sizeof(reservedFlexRayData1));
    os.write(reinterpret_cast<char *>(&reservedFlexRayData2), sizeof(reservedFlexRayData2));
    os.write(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
}

uint32_t FlexRayData::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(mux) +
        sizeof(len) +
        sizeof(messageId) +
        sizeof(crc) +
        sizeof(dir) +
        sizeof(reservedFlexRayData1) +
        sizeof(reservedFlexRayData2) +
        static_cast<uint32_t>(dataBytes.size());
}

}
}

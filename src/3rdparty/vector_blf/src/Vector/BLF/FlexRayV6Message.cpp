// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayV6Message.h>

namespace Vector {
namespace BLF {

FlexRayV6Message::FlexRayV6Message() :
    ObjectHeader(ObjectType::FLEXRAY_MESSAGE) {
}

void FlexRayV6Message::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&lowTime), sizeof(lowTime));
    is.read(reinterpret_cast<char *>(&fpgaTick), sizeof(fpgaTick));
    is.read(reinterpret_cast<char *>(&fpgaTickOverflow), sizeof(fpgaTickOverflow));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayV6Message), sizeof(clientIndexFlexRayV6Message));
    is.read(reinterpret_cast<char *>(&clusterTime), sizeof(clusterTime));
    is.read(reinterpret_cast<char *>(&frameId), sizeof(frameId));
    is.read(reinterpret_cast<char *>(&headerCrc), sizeof(headerCrc));
    is.read(reinterpret_cast<char *>(&frameState), sizeof(frameState));
    is.read(reinterpret_cast<char *>(&length), sizeof(length));
    is.read(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    is.read(reinterpret_cast<char *>(&headerBitMask), sizeof(headerBitMask));
    is.read(reinterpret_cast<char *>(&reservedFlexRayV6Message1), sizeof(reservedFlexRayV6Message1));
    is.read(reinterpret_cast<char *>(&reservedFlexRayV6Message2), sizeof(reservedFlexRayV6Message2));
    is.read(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
}

void FlexRayV6Message::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&lowTime), sizeof(lowTime));
    os.write(reinterpret_cast<char *>(&fpgaTick), sizeof(fpgaTick));
    os.write(reinterpret_cast<char *>(&fpgaTickOverflow), sizeof(fpgaTickOverflow));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayV6Message), sizeof(clientIndexFlexRayV6Message));
    os.write(reinterpret_cast<char *>(&clusterTime), sizeof(clusterTime));
    os.write(reinterpret_cast<char *>(&frameId), sizeof(frameId));
    os.write(reinterpret_cast<char *>(&headerCrc), sizeof(headerCrc));
    os.write(reinterpret_cast<char *>(&frameState), sizeof(frameState));
    os.write(reinterpret_cast<char *>(&length), sizeof(length));
    os.write(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    os.write(reinterpret_cast<char *>(&headerBitMask), sizeof(headerBitMask));
    os.write(reinterpret_cast<char *>(&reservedFlexRayV6Message1), sizeof(reservedFlexRayV6Message1));
    os.write(reinterpret_cast<char *>(&reservedFlexRayV6Message2), sizeof(reservedFlexRayV6Message2));
    os.write(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
}

uint32_t FlexRayV6Message::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(lowTime) +
        sizeof(fpgaTick) +
        sizeof(fpgaTickOverflow) +
        sizeof(clientIndexFlexRayV6Message) +
        sizeof(clusterTime) +
        sizeof(frameId) +
        sizeof(headerCrc) +
        sizeof(frameState) +
        sizeof(length) +
        sizeof(cycle) +
        sizeof(headerBitMask) +
        sizeof(reservedFlexRayV6Message1) +
        sizeof(reservedFlexRayV6Message2) +
        static_cast<uint32_t>(dataBytes.size());
}

}
}

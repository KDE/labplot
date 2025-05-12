// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayVFrReceiveMsg.h>

namespace Vector {
namespace BLF {

FlexRayVFrReceiveMsg::FlexRayVFrReceiveMsg() :
    ObjectHeader(ObjectType::FR_RCVMESSAGE) {
}

void FlexRayVFrReceiveMsg::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&version), sizeof(version));
    is.read(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg1), sizeof(reservedFlexRayVFrReceiveMsg1));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayVFrReceiveMsg), sizeof(clientIndexFlexRayVFrReceiveMsg));
    is.read(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    is.read(reinterpret_cast<char *>(&frameId), sizeof(frameId));
    is.read(reinterpret_cast<char *>(&headerCrc1), sizeof(headerCrc1));
    is.read(reinterpret_cast<char *>(&headerCrc2), sizeof(headerCrc2));
    is.read(reinterpret_cast<char *>(&byteCount), sizeof(byteCount));
    is.read(reinterpret_cast<char *>(&dataCount), sizeof(dataCount));
    is.read(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg2), sizeof(reservedFlexRayVFrReceiveMsg2));
    is.read(reinterpret_cast<char *>(&tag), sizeof(tag));
    is.read(reinterpret_cast<char *>(&data), sizeof(data));
    is.read(reinterpret_cast<char *>(&frameFlags), sizeof(frameFlags));
    is.read(reinterpret_cast<char *>(&appParameter), sizeof(appParameter));
    is.read(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg3), sizeof(reservedFlexRayVFrReceiveMsg3));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg4), sizeof(reservedFlexRayVFrReceiveMsg4));
}

void FlexRayVFrReceiveMsg::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&version), sizeof(version));
    os.write(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg1), sizeof(reservedFlexRayVFrReceiveMsg1));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayVFrReceiveMsg), sizeof(clientIndexFlexRayVFrReceiveMsg));
    os.write(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    os.write(reinterpret_cast<char *>(&frameId), sizeof(frameId));
    os.write(reinterpret_cast<char *>(&headerCrc1), sizeof(headerCrc1));
    os.write(reinterpret_cast<char *>(&headerCrc2), sizeof(headerCrc2));
    os.write(reinterpret_cast<char *>(&byteCount), sizeof(byteCount));
    os.write(reinterpret_cast<char *>(&dataCount), sizeof(dataCount));
    os.write(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg2), sizeof(reservedFlexRayVFrReceiveMsg2));
    os.write(reinterpret_cast<char *>(&tag), sizeof(tag));
    os.write(reinterpret_cast<char *>(&data), sizeof(data));
    os.write(reinterpret_cast<char *>(&frameFlags), sizeof(frameFlags));
    os.write(reinterpret_cast<char *>(&appParameter), sizeof(appParameter));
    os.write(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg3), sizeof(reservedFlexRayVFrReceiveMsg3));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrReceiveMsg4), sizeof(reservedFlexRayVFrReceiveMsg4));
}

uint32_t FlexRayVFrReceiveMsg::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(version) +
        sizeof(channelMask) +
        sizeof(dir) +
        sizeof(reservedFlexRayVFrReceiveMsg1) +
        sizeof(clientIndexFlexRayVFrReceiveMsg) +
        sizeof(clusterNo) +
        sizeof(frameId) +
        sizeof(headerCrc1) +
        sizeof(headerCrc2) +
        sizeof(byteCount) +
        sizeof(dataCount) +
        sizeof(cycle) +
        sizeof(reservedFlexRayVFrReceiveMsg2) +
        sizeof(tag) +
        sizeof(data) +
        sizeof(frameFlags) +
        sizeof(appParameter) +
        static_cast<uint32_t>(dataBytes.size()) +
        sizeof(reservedFlexRayVFrReceiveMsg3) +
        sizeof(reservedFlexRayVFrReceiveMsg4);
}

}
}

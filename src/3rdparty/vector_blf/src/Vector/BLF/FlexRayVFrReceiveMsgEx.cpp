// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayVFrReceiveMsgEx.h>

namespace Vector {
namespace BLF {

FlexRayVFrReceiveMsgEx::FlexRayVFrReceiveMsgEx() :
    ObjectHeader(ObjectType::FR_RCVMESSAGE_EX) {
}

void FlexRayVFrReceiveMsgEx::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&version), sizeof(version));
    is.read(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayVFrReceiveMsgEx), sizeof(clientIndexFlexRayVFrReceiveMsgEx));
    is.read(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    is.read(reinterpret_cast<char *>(&frameId), sizeof(frameId));
    is.read(reinterpret_cast<char *>(&headerCrc1), sizeof(headerCrc1));
    is.read(reinterpret_cast<char *>(&headerCrc2), sizeof(headerCrc2));
    is.read(reinterpret_cast<char *>(&byteCount), sizeof(byteCount));
    is.read(reinterpret_cast<char *>(&dataCount), sizeof(dataCount));
    is.read(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    is.read(reinterpret_cast<char *>(&tag), sizeof(tag));
    is.read(reinterpret_cast<char *>(&data), sizeof(data));
    is.read(reinterpret_cast<char *>(&frameFlags), sizeof(frameFlags));
    is.read(reinterpret_cast<char *>(&appParameter), sizeof(appParameter));
    is.read(reinterpret_cast<char *>(&frameCrc), sizeof(frameCrc));
    is.read(reinterpret_cast<char *>(&frameLengthNs), sizeof(frameLengthNs));
    is.read(reinterpret_cast<char *>(&frameId1), sizeof(frameId1));
    is.read(reinterpret_cast<char *>(&pduOffset), sizeof(pduOffset));
    is.read(reinterpret_cast<char *>(&blfLogMask), sizeof(blfLogMask));
    is.read(reinterpret_cast<char *>(reservedFlexRayVFrReceiveMsgEx1.data()), static_cast<std::streamsize>(reservedFlexRayVFrReceiveMsgEx1.size() * sizeof(uint16_t)));
    dataBytes.resize(dataCount);
    is.read(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataCount));
    reservedFlexRayVFrReceiveMsgEx2.resize(objectSize - calculateObjectSize()); // all remaining data
    is.read(reinterpret_cast<char *>(reservedFlexRayVFrReceiveMsgEx2.data()), static_cast<std::streamsize>(reservedFlexRayVFrReceiveMsgEx2.size()));
}

void FlexRayVFrReceiveMsgEx::write(AbstractFile & os) {
    /* pre processing */
    dataCount = static_cast<uint16_t>(dataBytes.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&version), sizeof(version));
    os.write(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayVFrReceiveMsgEx), sizeof(clientIndexFlexRayVFrReceiveMsgEx));
    os.write(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    os.write(reinterpret_cast<char *>(&frameId), sizeof(frameId));
    os.write(reinterpret_cast<char *>(&headerCrc1), sizeof(headerCrc1));
    os.write(reinterpret_cast<char *>(&headerCrc2), sizeof(headerCrc2));
    os.write(reinterpret_cast<char *>(&byteCount), sizeof(byteCount));
    os.write(reinterpret_cast<char *>(&dataCount), sizeof(dataCount));
    os.write(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    os.write(reinterpret_cast<char *>(&tag), sizeof(tag));
    os.write(reinterpret_cast<char *>(&data), sizeof(data));
    os.write(reinterpret_cast<char *>(&frameFlags), sizeof(frameFlags));
    os.write(reinterpret_cast<char *>(&appParameter), sizeof(appParameter));
    os.write(reinterpret_cast<char *>(&frameCrc), sizeof(frameCrc));
    os.write(reinterpret_cast<char *>(&frameLengthNs), sizeof(frameLengthNs));
    os.write(reinterpret_cast<char *>(&frameId1), sizeof(frameId1));
    os.write(reinterpret_cast<char *>(&pduOffset), sizeof(pduOffset));
    os.write(reinterpret_cast<char *>(&blfLogMask), sizeof(blfLogMask));
    os.write(reinterpret_cast<char *>(reservedFlexRayVFrReceiveMsgEx1.data()), static_cast<std::streamsize>(reservedFlexRayVFrReceiveMsgEx1.size() * sizeof(uint16_t)));
    os.write(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataCount));
    os.write(reinterpret_cast<char *>(reservedFlexRayVFrReceiveMsgEx2.data()), static_cast<std::streamsize>(reservedFlexRayVFrReceiveMsgEx2.size()));
}

uint32_t FlexRayVFrReceiveMsgEx::calculateObjectSize() const {
    uint32_t size =
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(version) +
        sizeof(channelMask) +
        sizeof(dir) +
        sizeof(clientIndexFlexRayVFrReceiveMsgEx) +
        sizeof(clusterNo) +
        sizeof(frameId) +
        sizeof(headerCrc1) +
        sizeof(headerCrc2) +
        sizeof(byteCount) +
        sizeof(dataCount) +
        sizeof(cycle) +
        sizeof(tag) +
        sizeof(data) +
        sizeof(frameFlags) +
        sizeof(appParameter) +
        sizeof(frameCrc) +
        sizeof(frameLengthNs) +
        sizeof(frameId1) +
        sizeof(pduOffset) +
        sizeof(blfLogMask) +
        static_cast<uint32_t>(reservedFlexRayVFrReceiveMsgEx1.size() * sizeof(uint16_t)) +
        dataCount +
        static_cast<uint32_t>(reservedFlexRayVFrReceiveMsgEx2.size());

    return size;
}

}
}

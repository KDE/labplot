// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/EthernetFrameForwarded.h>

namespace Vector {
namespace BLF {

EthernetFrameForwarded::EthernetFrameForwarded() :
    ObjectHeader(ObjectType::ETHERNET_FRAME_FORWARDED) {
}

void EthernetFrameForwarded::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&structLength), sizeof(structLength));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    is.read(reinterpret_cast<char *>(&frameDuration), sizeof(frameDuration));
    is.read(reinterpret_cast<char *>(&frameChecksum), sizeof(frameChecksum));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&frameHandle), sizeof(frameHandle));
    is.read(reinterpret_cast<char *>(&reservedEthernetFrameForwarded), sizeof(reservedEthernetFrameForwarded));
    frameData.resize(frameLength);
    is.read(reinterpret_cast<char *>(frameData.data()), frameLength);
}

void EthernetFrameForwarded::write(AbstractFile & os) {
    /* pre processing */
    structLength = calculateStructLength();
    frameLength = static_cast<uint16_t>(frameData.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&structLength), sizeof(structLength));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&hardwareChannel), sizeof(hardwareChannel));
    os.write(reinterpret_cast<char *>(&frameDuration), sizeof(frameDuration));
    os.write(reinterpret_cast<char *>(&frameChecksum), sizeof(frameChecksum));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&frameHandle), sizeof(frameHandle));
    os.write(reinterpret_cast<char *>(&reservedEthernetFrameForwarded), sizeof(reservedEthernetFrameForwarded));
    os.write(reinterpret_cast<char *>(frameData.data()), frameLength);
}

uint32_t EthernetFrameForwarded::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(structLength) +
        sizeof(flags) +
        sizeof(channel) +
        sizeof(hardwareChannel) +
        sizeof(frameDuration) +
        sizeof(frameChecksum) +
        sizeof(dir) +
        sizeof(frameLength) +
        sizeof(frameHandle) +
        sizeof(reservedEthernetFrameForwarded) +
        frameLength;
}

uint16_t EthernetFrameForwarded::calculateStructLength() const {
    return
        sizeof(flags) +
        sizeof(channel) +
        sizeof(hardwareChannel) +
        sizeof(frameDuration) +
        sizeof(frameChecksum) +
        sizeof(dir) +
        sizeof(frameLength) +
        sizeof(frameHandle) +
        sizeof(reservedEthernetFrameForwarded);
}

}
}

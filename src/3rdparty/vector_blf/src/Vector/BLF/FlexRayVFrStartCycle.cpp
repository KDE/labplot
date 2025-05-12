// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayVFrStartCycle.h>

#include <Vector/BLF/AbstractFile.h>

namespace Vector {
namespace BLF {

FlexRayVFrStartCycle::FlexRayVFrStartCycle() :
    ObjectHeader(ObjectType::FR_STARTCYCLE) {
}

void FlexRayVFrStartCycle::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&version), sizeof(version));
    is.read(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayVFrStartCycle), sizeof(clientIndexFlexRayVFrStartCycle));
    is.read(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    is.read(reinterpret_cast<char *>(&nmSize), sizeof(nmSize));
    is.read(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrStartCycle1), sizeof(reservedFlexRayVFrStartCycle1));
    is.read(reinterpret_cast<char *>(&tag), sizeof(tag));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrStartCycle2), sizeof(reservedFlexRayVFrStartCycle2));
}

void FlexRayVFrStartCycle::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&version), sizeof(version));
    os.write(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayVFrStartCycle), sizeof(clientIndexFlexRayVFrStartCycle));
    os.write(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    os.write(reinterpret_cast<char *>(&nmSize), sizeof(nmSize));
    os.write(reinterpret_cast<char *>(dataBytes.data()), static_cast<std::streamsize>(dataBytes.size()));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrStartCycle1), sizeof(reservedFlexRayVFrStartCycle1));
    os.write(reinterpret_cast<char *>(&tag), sizeof(tag));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrStartCycle2), sizeof(reservedFlexRayVFrStartCycle2));
}

uint32_t FlexRayVFrStartCycle::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(version) +
        sizeof(channelMask) +
        sizeof(dir) +
        sizeof(cycle) +
        sizeof(clientIndexFlexRayVFrStartCycle) +
        sizeof(clusterNo) +
        sizeof(nmSize) +
        static_cast<uint32_t>(dataBytes.size()) +
        sizeof(reservedFlexRayVFrStartCycle1) +
        sizeof(tag) +
        static_cast<uint32_t>(data.size() * sizeof(uint32_t)) +
        sizeof(reservedFlexRayVFrStartCycle2);
}

}
}

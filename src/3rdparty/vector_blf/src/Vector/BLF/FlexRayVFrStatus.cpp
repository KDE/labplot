// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayVFrStatus.h>

namespace Vector {
namespace BLF {

FlexRayVFrStatus::FlexRayVFrStatus() :
    ObjectHeader(ObjectType::FR_STATUS) {
}

void FlexRayVFrStatus::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&version), sizeof(version));
    is.read(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    is.read(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrStatus1), sizeof(reservedFlexRayVFrStatus1));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayVFrStatus), sizeof(clientIndexFlexRayVFrStatus));
    is.read(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    is.read(reinterpret_cast<char *>(&wus), sizeof(wus));
    is.read(reinterpret_cast<char *>(&ccSyncState), sizeof(ccSyncState));
    is.read(reinterpret_cast<char *>(&tag), sizeof(tag));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    is.read(reinterpret_cast<char *>(reservedFlexRayVFrStatus2.data()), static_cast<std::streamsize>(reservedFlexRayVFrStatus2.size() * sizeof(uint16_t)));
}

void FlexRayVFrStatus::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&version), sizeof(version));
    os.write(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    os.write(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrStatus1), sizeof(reservedFlexRayVFrStatus1));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayVFrStatus), sizeof(clientIndexFlexRayVFrStatus));
    os.write(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    os.write(reinterpret_cast<char *>(&wus), sizeof(wus));
    os.write(reinterpret_cast<char *>(&ccSyncState), sizeof(ccSyncState));
    os.write(reinterpret_cast<char *>(&tag), sizeof(tag));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    os.write(reinterpret_cast<char *>(reservedFlexRayVFrStatus2.data()), static_cast<std::streamsize>(reservedFlexRayVFrStatus2.size() * sizeof(uint16_t)));
}

uint32_t FlexRayVFrStatus::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(version) +
        sizeof(channelMask) +
        sizeof(cycle) +
        sizeof(reservedFlexRayVFrStatus1) +
        sizeof(clientIndexFlexRayVFrStatus) +
        sizeof(clusterNo) +
        sizeof(wus) +
        sizeof(ccSyncState) +
        sizeof(tag) +
        static_cast<uint32_t>(data.size() * sizeof(uint32_t)) +
        static_cast<uint32_t>(reservedFlexRayVFrStatus2.size() * sizeof(uint16_t));
}

}
}

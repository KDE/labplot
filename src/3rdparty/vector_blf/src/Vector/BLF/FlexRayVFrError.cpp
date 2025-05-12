// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayVFrError.h>

namespace Vector {
namespace BLF {

FlexRayVFrError::FlexRayVFrError() :
    ObjectHeader(ObjectType::FR_ERROR) {
}

void FlexRayVFrError::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&version), sizeof(version));
    is.read(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    is.read(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrError1), sizeof(reservedFlexRayVFrError1));
    is.read(reinterpret_cast<char *>(&clientIndexFlexRayVFrError), sizeof(clientIndexFlexRayVFrError));
    is.read(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    is.read(reinterpret_cast<char *>(&tag), sizeof(tag));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    is.read(reinterpret_cast<char *>(&reservedFlexRayVFrError2), sizeof(reservedFlexRayVFrError2));
}

void FlexRayVFrError::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&version), sizeof(version));
    os.write(reinterpret_cast<char *>(&channelMask), sizeof(channelMask));
    os.write(reinterpret_cast<char *>(&cycle), sizeof(cycle));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrError1), sizeof(reservedFlexRayVFrError1));
    os.write(reinterpret_cast<char *>(&clientIndexFlexRayVFrError), sizeof(clientIndexFlexRayVFrError));
    os.write(reinterpret_cast<char *>(&clusterNo), sizeof(clusterNo));
    os.write(reinterpret_cast<char *>(&tag), sizeof(tag));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(uint32_t)));
    os.write(reinterpret_cast<char *>(&reservedFlexRayVFrError2), sizeof(reservedFlexRayVFrError2));
}

uint32_t FlexRayVFrError::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(version) +
        sizeof(channelMask) +
        sizeof(cycle) +
        sizeof(reservedFlexRayVFrError1) +
        sizeof(clientIndexFlexRayVFrError) +
        sizeof(clusterNo) +
        sizeof(tag) +
        static_cast<uint32_t>(data.size() * sizeof(uint32_t)) +
        sizeof(reservedFlexRayVFrError2);
}

}
}

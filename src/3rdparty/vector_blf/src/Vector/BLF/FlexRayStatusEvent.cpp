// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FlexRayStatusEvent.h>

namespace Vector {
namespace BLF {

FlexRayStatusEvent::FlexRayStatusEvent() :
    ObjectHeader(ObjectType::FLEXRAY_STATUS) {
}

void FlexRayStatusEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&version), sizeof(version));
    is.read(reinterpret_cast<char *>(&statusType), sizeof(statusType));
    is.read(reinterpret_cast<char *>(&infoMask1), sizeof(infoMask1));
    is.read(reinterpret_cast<char *>(&infoMask2), sizeof(infoMask2));
    is.read(reinterpret_cast<char *>(&infoMask3), sizeof(infoMask3));
    is.read(reinterpret_cast<char *>(reservedFlexRayStatusEvent.data()), static_cast<std::streamsize>(reservedFlexRayStatusEvent.size() * sizeof(uint16_t)));
}

void FlexRayStatusEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&version), sizeof(version));
    os.write(reinterpret_cast<char *>(&statusType), sizeof(statusType));
    os.write(reinterpret_cast<char *>(&infoMask1), sizeof(infoMask1));
    os.write(reinterpret_cast<char *>(&infoMask2), sizeof(infoMask2));
    os.write(reinterpret_cast<char *>(&infoMask3), sizeof(infoMask3));
    os.write(reinterpret_cast<char *>(reservedFlexRayStatusEvent.data()), static_cast<std::streamsize>(reservedFlexRayStatusEvent.size() * sizeof(uint16_t)));
}

uint32_t FlexRayStatusEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(version) +
        sizeof(statusType) +
        sizeof(infoMask1) +
        sizeof(infoMask2) +
        sizeof(infoMask3) +
        static_cast<uint32_t>(reservedFlexRayStatusEvent.size() * sizeof(uint16_t));
}

}
}

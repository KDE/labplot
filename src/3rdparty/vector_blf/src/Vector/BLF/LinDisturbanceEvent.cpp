// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinDisturbanceEvent.h>

namespace Vector {
namespace BLF {

LinDisturbanceEvent::LinDisturbanceEvent() :
    ObjectHeader(ObjectType::LIN_DISTURBANCE_EVENT) {
}

void LinDisturbanceEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&disturbingFrameId), sizeof(disturbingFrameId));
    is.read(reinterpret_cast<char *>(&disturbanceType), sizeof(disturbanceType));
    is.read(reinterpret_cast<char *>(&byteIndex), sizeof(byteIndex));
    is.read(reinterpret_cast<char *>(&bitIndex), sizeof(bitIndex));
    is.read(reinterpret_cast<char *>(&bitOffsetInSixteenthBits), sizeof(bitOffsetInSixteenthBits));
    is.read(reinterpret_cast<char *>(&disturbanceLengthInSixteenthBits), sizeof(disturbanceLengthInSixteenthBits));
    // @note might be extended in future versions
}

void LinDisturbanceEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&disturbingFrameId), sizeof(disturbingFrameId));
    os.write(reinterpret_cast<char *>(&disturbanceType), sizeof(disturbanceType));
    os.write(reinterpret_cast<char *>(&byteIndex), sizeof(byteIndex));
    os.write(reinterpret_cast<char *>(&bitIndex), sizeof(bitIndex));
    os.write(reinterpret_cast<char *>(&bitOffsetInSixteenthBits), sizeof(bitOffsetInSixteenthBits));
    os.write(reinterpret_cast<char *>(&disturbanceLengthInSixteenthBits), sizeof(disturbanceLengthInSixteenthBits));
}

uint32_t LinDisturbanceEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(id) +
        sizeof(disturbingFrameId) +
        sizeof(disturbanceType) +
        sizeof(byteIndex) +
        sizeof(bitIndex) +
        sizeof(bitOffsetInSixteenthBits) +
        sizeof(disturbanceLengthInSixteenthBits);
}

}
}

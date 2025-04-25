// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanFdMessage.h>

namespace Vector {
namespace BLF {

CanFdMessage::CanFdMessage() :
    ObjectHeader(ObjectType::CAN_FD_MESSAGE) {
}

void CanFdMessage::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&arbBitCount), sizeof(arbBitCount));
    is.read(reinterpret_cast<char *>(&canFdFlags), sizeof(canFdFlags));
    is.read(reinterpret_cast<char *>(&validDataBytes), sizeof(validDataBytes));
    is.read(reinterpret_cast<char *>(&reservedCanFdMessage1), sizeof(reservedCanFdMessage1));
    is.read(reinterpret_cast<char *>(&reservedCanFdMessage2), sizeof(reservedCanFdMessage2));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&reservedCanFdMessage3), sizeof(reservedCanFdMessage3));
    // @note might be extended in future versions
}

void CanFdMessage::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&arbBitCount), sizeof(arbBitCount));
    os.write(reinterpret_cast<char *>(&canFdFlags), sizeof(canFdFlags));
    os.write(reinterpret_cast<char *>(&validDataBytes), sizeof(validDataBytes));
    os.write(reinterpret_cast<char *>(&reservedCanFdMessage1), sizeof(reservedCanFdMessage1));
    os.write(reinterpret_cast<char *>(&reservedCanFdMessage2), sizeof(reservedCanFdMessage2));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&reservedCanFdMessage3), sizeof(reservedCanFdMessage3));
}

uint32_t CanFdMessage::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(dlc) +
        sizeof(id) +
        sizeof(frameLength) +
        sizeof(arbBitCount) +
        sizeof(canFdFlags) +
        sizeof(validDataBytes) +
        sizeof(reservedCanFdMessage1) +
        sizeof(reservedCanFdMessage2) +
        static_cast<uint32_t>(data.size()) +
        sizeof(reservedCanFdMessage3);
}

}
}

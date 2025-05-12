// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanMessage2.h>

namespace Vector {
namespace BLF {

CanMessage2::CanMessage2() :
    ObjectHeader(ObjectType::CAN_MESSAGE2) {
}

void CanMessage2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    data.resize(objectSize - calculateObjectSize()); // all remaining data
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    is.read(reinterpret_cast<char *>(&bitCount), sizeof(bitCount));
    is.read(reinterpret_cast<char *>(&reservedCanMessage1), sizeof(reservedCanMessage1));
    is.read(reinterpret_cast<char *>(&reservedCanMessage2), sizeof(reservedCanMessage2));
    // @note might be extended in future versions
}

void CanMessage2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&dlc), sizeof(dlc));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&frameLength), sizeof(frameLength));
    os.write(reinterpret_cast<char *>(&bitCount), sizeof(bitCount));
    os.write(reinterpret_cast<char *>(&reservedCanMessage1), sizeof(reservedCanMessage1));
    os.write(reinterpret_cast<char *>(&reservedCanMessage2), sizeof(reservedCanMessage2));
}

uint32_t CanMessage2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(dlc) +
        sizeof(id) +
        static_cast<uint32_t>(data.size()) +
        sizeof(frameLength) +
        sizeof(bitCount) +
        sizeof(reservedCanMessage1) +
        sizeof(reservedCanMessage2);
}

}
}

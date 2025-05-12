// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/J1708Message.h>

namespace Vector {
namespace BLF {

J1708Message::J1708Message() :
    ObjectHeader(ObjectType::J1708_MESSAGE) { // or J1708_VIRTUAL_MSG
}

void J1708Message::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&dir), sizeof(dir));
    is.read(reinterpret_cast<char *>(&reservedJ1708Message1), sizeof(reservedJ1708Message1));
    is.read(reinterpret_cast<char *>(&error), sizeof(error));
    is.read(reinterpret_cast<char *>(&size), sizeof(size));
    is.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    is.read(reinterpret_cast<char *>(&reservedJ1708Message2), sizeof(reservedJ1708Message2));
}

void J1708Message::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&dir), sizeof(dir));
    os.write(reinterpret_cast<char *>(&reservedJ1708Message1), sizeof(reservedJ1708Message1));
    os.write(reinterpret_cast<char *>(&error), sizeof(error));
    os.write(reinterpret_cast<char *>(&size), sizeof(size));
    os.write(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(data.size()));
    os.write(reinterpret_cast<char *>(&reservedJ1708Message2), sizeof(reservedJ1708Message2));
}

uint32_t J1708Message::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(dir) +
        sizeof(reservedJ1708Message1) +
        sizeof(error) +
        sizeof(size) +
        static_cast<uint32_t>(data.size()) +
        sizeof(reservedJ1708Message2);
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinBaudrateEvent.h>

namespace Vector {
namespace BLF {

LinBaudrateEvent::LinBaudrateEvent() :
    ObjectHeader(ObjectType::LIN_BAUDRATE) {
}

void LinBaudrateEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedLinBaudrateEvent), sizeof(reservedLinBaudrateEvent));
    is.read(reinterpret_cast<char *>(&baudrate), sizeof(baudrate));
}

void LinBaudrateEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedLinBaudrateEvent), sizeof(reservedLinBaudrateEvent));
    os.write(reinterpret_cast<char *>(&baudrate), sizeof(baudrate));
}

uint32_t LinBaudrateEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedLinBaudrateEvent) +
        sizeof(baudrate);
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostSystemEvent.h>

namespace Vector {
namespace BLF {

MostSystemEvent::MostSystemEvent() :
    ObjectHeader2(ObjectType::MOST_SYSTEM_EVENT) {
}

void MostSystemEvent::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&id), sizeof(id));
    is.read(reinterpret_cast<char *>(&value), sizeof(value));
    is.read(reinterpret_cast<char *>(&valueOld), sizeof(valueOld));
    is.read(reinterpret_cast<char *>(&reservedMostSystemEvent), sizeof(reservedMostSystemEvent));
}

void MostSystemEvent::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&id), sizeof(id));
    os.write(reinterpret_cast<char *>(&value), sizeof(value));
    os.write(reinterpret_cast<char *>(&valueOld), sizeof(valueOld));
    os.write(reinterpret_cast<char *>(&reservedMostSystemEvent), sizeof(reservedMostSystemEvent));
}

uint32_t MostSystemEvent::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(id) +
        sizeof(value) +
        sizeof(valueOld) +
        sizeof(reservedMostSystemEvent);
}

}
}

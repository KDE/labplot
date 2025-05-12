// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinWakeupEvent.h>

namespace Vector {
namespace BLF {

LinWakeupEvent::LinWakeupEvent() :
    ObjectHeader(ObjectType::LIN_WAKEUP) {
}

void LinWakeupEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&signal), sizeof(signal));
    is.read(reinterpret_cast<char *>(&external), sizeof(external));
    is.read(reinterpret_cast<char *>(&reservedLinWakeupEvent), sizeof(reservedLinWakeupEvent));
}

void LinWakeupEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&signal), sizeof(signal));
    os.write(reinterpret_cast<char *>(&external), sizeof(external));
    os.write(reinterpret_cast<char *>(&reservedLinWakeupEvent), sizeof(reservedLinWakeupEvent));
}

uint32_t LinWakeupEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(signal) +
        sizeof(external) +
        sizeof(reservedLinWakeupEvent);
}

}
}

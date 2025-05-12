// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinWakeupEvent2.h>

namespace Vector {
namespace BLF {

LinWakeupEvent2::LinWakeupEvent2() :
    ObjectHeader(ObjectType::LIN_WAKEUP2) {
}

void LinWakeupEvent2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinBusEvent::read(is);
    is.read(reinterpret_cast<char *>(&lengthInfo), sizeof(lengthInfo));
    is.read(reinterpret_cast<char *>(&signal), sizeof(signal));
    is.read(reinterpret_cast<char *>(&external), sizeof(external));
    is.read(reinterpret_cast<char *>(&reservedLinWakeupEvent1), sizeof(reservedLinWakeupEvent1));
    is.read(reinterpret_cast<char *>(&reservedLinWakeupEvent2), sizeof(reservedLinWakeupEvent2));
    // @note might be extended in future versions
}

void LinWakeupEvent2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinBusEvent::write(os);
    os.write(reinterpret_cast<char *>(&lengthInfo), sizeof(lengthInfo));
    os.write(reinterpret_cast<char *>(&signal), sizeof(signal));
    os.write(reinterpret_cast<char *>(&external), sizeof(external));
    os.write(reinterpret_cast<char *>(&reservedLinWakeupEvent1), sizeof(reservedLinWakeupEvent1));
    os.write(reinterpret_cast<char *>(&reservedLinWakeupEvent2), sizeof(reservedLinWakeupEvent2));
}

uint32_t LinWakeupEvent2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinBusEvent::calculateObjectSize() +
        sizeof(lengthInfo) +
        sizeof(signal) +
        sizeof(external) +
        sizeof(reservedLinWakeupEvent1) +
        sizeof(reservedLinWakeupEvent2);
}

}
}

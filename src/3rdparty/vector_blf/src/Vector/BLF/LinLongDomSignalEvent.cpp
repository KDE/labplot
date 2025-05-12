// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinLongDomSignalEvent.h>

namespace Vector {
namespace BLF {

LinLongDomSignalEvent::LinLongDomSignalEvent() :
    ObjectHeader(ObjectType::LIN_LONG_DOM_SIG) {
}

void LinLongDomSignalEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinBusEvent::read(is);
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent1), sizeof(reservedLinLongDomSignalEvent1));
    is.read(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent2), sizeof(reservedLinLongDomSignalEvent2));
    is.read(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent3), sizeof(reservedLinLongDomSignalEvent3));
}

void LinLongDomSignalEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinBusEvent::write(os);
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent1), sizeof(reservedLinLongDomSignalEvent1));
    os.write(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent2), sizeof(reservedLinLongDomSignalEvent2));
    os.write(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent3), sizeof(reservedLinLongDomSignalEvent3));
}

uint32_t LinLongDomSignalEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinBusEvent::calculateObjectSize() +
        sizeof(type) +
        sizeof(reservedLinLongDomSignalEvent1) +
        sizeof(reservedLinLongDomSignalEvent2) +
        sizeof(reservedLinLongDomSignalEvent3);
}

}
}

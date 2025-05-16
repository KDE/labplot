// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinLongDomSignalEvent2.h>

namespace Vector {
namespace BLF {

LinLongDomSignalEvent2::LinLongDomSignalEvent2() :
    ObjectHeader(ObjectType::LIN_LONG_DOM_SIG2) {
}

void LinLongDomSignalEvent2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinBusEvent::read(is);
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent1), sizeof(reservedLinLongDomSignalEvent1));
    is.read(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent2), sizeof(reservedLinLongDomSignalEvent2));
    is.read(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent3), sizeof(reservedLinLongDomSignalEvent3));
    is.read(reinterpret_cast<char *>(&length), sizeof(length));
    // @note might be extended in future versions
}

void LinLongDomSignalEvent2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinBusEvent::write(os);
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent1), sizeof(reservedLinLongDomSignalEvent1));
    os.write(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent2), sizeof(reservedLinLongDomSignalEvent2));
    os.write(reinterpret_cast<char *>(&reservedLinLongDomSignalEvent3), sizeof(reservedLinLongDomSignalEvent3));
    os.write(reinterpret_cast<char *>(&length), sizeof(length));
}

uint32_t LinLongDomSignalEvent2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinBusEvent::calculateObjectSize() +
        sizeof(type) +
        sizeof(reservedLinLongDomSignalEvent1) +
        sizeof(reservedLinLongDomSignalEvent2) +
        sizeof(reservedLinLongDomSignalEvent3) +
        sizeof(length);
}

}
}

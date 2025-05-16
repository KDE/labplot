// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinUnexpectedWakeup.h>

namespace Vector {
namespace BLF {

LinUnexpectedWakeup::LinUnexpectedWakeup() :
    ObjectHeader(ObjectType::LIN_UNEXPECTED_WAKEUP) {
}

void LinUnexpectedWakeup::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinBusEvent::read(is);
    is.read(reinterpret_cast<char *>(&width), sizeof(width));
    is.read(reinterpret_cast<char *>(&signal), sizeof(signal));
    is.read(reinterpret_cast<char *>(&reservedLinUnexpectedWakeup1), sizeof(reservedLinUnexpectedWakeup1));
    is.read(reinterpret_cast<char *>(&reservedLinUnexpectedWakeup2), sizeof(reservedLinUnexpectedWakeup2));
    is.read(reinterpret_cast<char *>(&reservedLinUnexpectedWakeup3), sizeof(reservedLinUnexpectedWakeup3));
    // @note might be extended in future versions
}

void LinUnexpectedWakeup::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinBusEvent::write(os);
    os.write(reinterpret_cast<char *>(&width), sizeof(width));
    os.write(reinterpret_cast<char *>(&signal), sizeof(signal));
    os.write(reinterpret_cast<char *>(&reservedLinUnexpectedWakeup1), sizeof(reservedLinUnexpectedWakeup1));
    os.write(reinterpret_cast<char *>(&reservedLinUnexpectedWakeup2), sizeof(reservedLinUnexpectedWakeup2));
    os.write(reinterpret_cast<char *>(&reservedLinUnexpectedWakeup3), sizeof(reservedLinUnexpectedWakeup3));
}

uint32_t LinUnexpectedWakeup::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinBusEvent::calculateObjectSize() +
        sizeof(width) +
        sizeof(signal) +
        sizeof(reservedLinUnexpectedWakeup1) +
        sizeof(reservedLinUnexpectedWakeup2) +
        sizeof(reservedLinUnexpectedWakeup3);
}

}
}

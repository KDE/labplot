// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinBusEvent.h>

namespace Vector {
namespace BLF {

void LinBusEvent::read(AbstractFile & is) {
    is.read(reinterpret_cast<char *>(&sof), sizeof(sof));
    is.read(reinterpret_cast<char *>(&eventBaudrate), sizeof(eventBaudrate));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedLinBusEvent), sizeof(reservedLinBusEvent));
}

void LinBusEvent::write(AbstractFile & os) {
    os.write(reinterpret_cast<char *>(&sof), sizeof(sof));
    os.write(reinterpret_cast<char *>(&eventBaudrate), sizeof(eventBaudrate));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedLinBusEvent), sizeof(reservedLinBusEvent));
}

uint32_t LinBusEvent::calculateObjectSize() const {
    return
        sizeof(sof) +
        sizeof(eventBaudrate) +
        sizeof(channel) +
        sizeof(reservedLinBusEvent);
}

}
}

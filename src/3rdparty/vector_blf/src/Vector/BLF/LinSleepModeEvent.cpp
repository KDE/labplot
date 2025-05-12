// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSleepModeEvent.h>

namespace Vector {
namespace BLF {

LinSleepModeEvent::LinSleepModeEvent() :
    ObjectHeader(ObjectType::LIN_SLEEP) {
}

void LinSleepModeEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reason), sizeof(reason));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&reservedLinSleepModeEvent), sizeof(reservedLinSleepModeEvent));
}

void LinSleepModeEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reason), sizeof(reason));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&reservedLinSleepModeEvent), sizeof(reservedLinSleepModeEvent));
}

uint32_t LinSleepModeEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reason) +
        sizeof(flags) +
        sizeof(reservedLinSleepModeEvent);
}

}
}

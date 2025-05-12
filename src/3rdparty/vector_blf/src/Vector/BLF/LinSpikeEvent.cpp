// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSpikeEvent.h>

namespace Vector {
namespace BLF {

LinSpikeEvent::LinSpikeEvent() :
    ObjectHeader(ObjectType::LIN_SPIKE_EVENT) {
}

void LinSpikeEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedLinSpikeEvent), sizeof(reservedLinSpikeEvent));
    is.read(reinterpret_cast<char *>(&width), sizeof(width));
}

void LinSpikeEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedLinSpikeEvent), sizeof(reservedLinSpikeEvent));
    os.write(reinterpret_cast<char *>(&width), sizeof(width));
}

uint32_t LinSpikeEvent::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedLinSpikeEvent) +
        sizeof(width);
}

}
}

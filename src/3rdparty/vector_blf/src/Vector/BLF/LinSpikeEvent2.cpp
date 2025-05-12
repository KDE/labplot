// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/LinSpikeEvent2.h>

namespace Vector {
namespace BLF {

LinSpikeEvent2::LinSpikeEvent2() :
    ObjectHeader(ObjectType::LIN_SPIKE_EVENT2) {
}

void LinSpikeEvent2::read(AbstractFile & is) {
    ObjectHeader::read(is);
    LinBusEvent::read(is);
    is.read(reinterpret_cast<char *>(&width), sizeof(width));
    is.read(reinterpret_cast<char *>(&internal), sizeof(internal));
    is.read(reinterpret_cast<char *>(&reservedLinSpikeEvent1), sizeof(reservedLinSpikeEvent1));
    is.read(reinterpret_cast<char *>(&reservedLinSpikeEvent2), sizeof(reservedLinSpikeEvent2));
    // @note might be extended in future versions
}

void LinSpikeEvent2::write(AbstractFile & os) {
    ObjectHeader::write(os);
    LinBusEvent::write(os);
    os.write(reinterpret_cast<char *>(&width), sizeof(width));
    os.write(reinterpret_cast<char *>(&internal), sizeof(internal));
    os.write(reinterpret_cast<char *>(&reservedLinSpikeEvent1), sizeof(reservedLinSpikeEvent1));
    os.write(reinterpret_cast<char *>(&reservedLinSpikeEvent2), sizeof(reservedLinSpikeEvent2));
}

uint32_t LinSpikeEvent2::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        LinBusEvent::calculateObjectSize() +
        sizeof(width) +
        sizeof(internal) +
        sizeof(reservedLinSpikeEvent1) +
        sizeof(reservedLinSpikeEvent2);
}

}
}

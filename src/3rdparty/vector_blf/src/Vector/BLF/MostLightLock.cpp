// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostLightLock.h>

namespace Vector {
namespace BLF {

MostLightLock::MostLightLock() :
    ObjectHeader(ObjectType::MOST_LIGHTLOCK) {
}

void MostLightLock::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&reservedMostLightLock), sizeof(reservedMostLightLock));
}

void MostLightLock::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&reservedMostLightLock), sizeof(reservedMostLightLock));
}

uint32_t MostLightLock::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(state) +
        sizeof(reservedMostLightLock);
}

}
}

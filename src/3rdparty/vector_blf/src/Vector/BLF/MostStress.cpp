// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostStress.h>

namespace Vector {
namespace BLF {

MostStress::MostStress() :
    ObjectHeader2(ObjectType::MOST_STRESS) {
}

void MostStress::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&mode), sizeof(mode));
    is.read(reinterpret_cast<char *>(&reservedMostStress), sizeof(reservedMostStress));
}

void MostStress::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&mode), sizeof(mode));
    os.write(reinterpret_cast<char *>(&reservedMostStress), sizeof(reservedMostStress));
}

uint32_t MostStress::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(state) +
        sizeof(mode) +
        sizeof(reservedMostStress);
}

}
}

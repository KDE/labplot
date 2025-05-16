// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostTxLight.h>

namespace Vector {
namespace BLF {

MostTxLight::MostTxLight() :
    ObjectHeader2(ObjectType::MOST_TXLIGHT) {
}

void MostTxLight::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&reservedMostTxLight), sizeof(reservedMostTxLight));
}

void MostTxLight::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&reservedMostTxLight), sizeof(reservedMostTxLight));
}

uint32_t MostTxLight::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(state) +
        sizeof(reservedMostTxLight);
}

}
}

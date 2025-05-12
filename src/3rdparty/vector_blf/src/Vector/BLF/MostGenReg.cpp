// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostGenReg.h>

namespace Vector {
namespace BLF {

MostGenReg::MostGenReg() :
    ObjectHeader2(ObjectType::MOST_GENREG) {
}

void MostGenReg::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&subType), sizeof(subType));
    is.read(reinterpret_cast<char *>(&reservedMostGenReg1), sizeof(reservedMostGenReg1));
    is.read(reinterpret_cast<char *>(&handle), sizeof(handle));
    is.read(reinterpret_cast<char *>(&regId), sizeof(regId));
    is.read(reinterpret_cast<char *>(&reservedMostGenReg2), sizeof(reservedMostGenReg2));
    is.read(reinterpret_cast<char *>(&reservedMostGenReg3), sizeof(reservedMostGenReg3));
    is.read(reinterpret_cast<char *>(&regValue), sizeof(regValue));
}

void MostGenReg::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&subType), sizeof(subType));
    os.write(reinterpret_cast<char *>(&reservedMostGenReg1), sizeof(reservedMostGenReg1));
    os.write(reinterpret_cast<char *>(&handle), sizeof(handle));
    os.write(reinterpret_cast<char *>(&regId), sizeof(regId));
    os.write(reinterpret_cast<char *>(&reservedMostGenReg2), sizeof(reservedMostGenReg2));
    os.write(reinterpret_cast<char *>(&reservedMostGenReg3), sizeof(reservedMostGenReg3));
    os.write(reinterpret_cast<char *>(&regValue), sizeof(regValue));
}

uint32_t MostGenReg::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(subType) +
        sizeof(reservedMostGenReg1) +
        sizeof(handle) +
        sizeof(regId) +
        sizeof(reservedMostGenReg2) +
        sizeof(reservedMostGenReg3) +
        sizeof(regValue);
}

}
}

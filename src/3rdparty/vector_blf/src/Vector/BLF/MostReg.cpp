// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostReg.h>

namespace Vector {
namespace BLF {

MostReg::MostReg() :
    ObjectHeader2(ObjectType::MOST_REG) {
}

void MostReg::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&subType), sizeof(subType));
    is.read(reinterpret_cast<char *>(&reservedMostReg), sizeof(reservedMostReg));
    is.read(reinterpret_cast<char *>(&handle), sizeof(handle));
    is.read(reinterpret_cast<char *>(&offset), sizeof(offset));
    is.read(reinterpret_cast<char *>(&chip), sizeof(chip));
    is.read(reinterpret_cast<char *>(&regDataLen), sizeof(regDataLen));
    is.read(reinterpret_cast<char *>(regData.data()), static_cast<std::streamsize>(regData.size()));
}

void MostReg::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&subType), sizeof(subType));
    os.write(reinterpret_cast<char *>(&reservedMostReg), sizeof(reservedMostReg));
    os.write(reinterpret_cast<char *>(&handle), sizeof(handle));
    os.write(reinterpret_cast<char *>(&offset), sizeof(offset));
    os.write(reinterpret_cast<char *>(&chip), sizeof(chip));
    os.write(reinterpret_cast<char *>(&regDataLen), sizeof(regDataLen));
    os.write(reinterpret_cast<char *>(regData.data()), static_cast<std::streamsize>(regData.size()));
}

uint32_t MostReg::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(subType) +
        sizeof(reservedMostReg) +
        sizeof(handle) +
        sizeof(offset) +
        sizeof(chip) +
        sizeof(regDataLen) +
        static_cast<uint32_t>(regData.size());
}

}
}

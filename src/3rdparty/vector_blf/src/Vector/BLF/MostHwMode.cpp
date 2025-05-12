// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostHwMode.h>

namespace Vector {
namespace BLF {

MostHwMode::MostHwMode() :
    ObjectHeader2(ObjectType::MOST_HWMODE) {
}

void MostHwMode::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedMostHwMode), sizeof(reservedMostHwMode));
    is.read(reinterpret_cast<char *>(&hwMode), sizeof(hwMode));
    is.read(reinterpret_cast<char *>(&hwModeMask), sizeof(hwModeMask));
}

void MostHwMode::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedMostHwMode), sizeof(reservedMostHwMode));
    os.write(reinterpret_cast<char *>(&hwMode), sizeof(hwMode));
    os.write(reinterpret_cast<char *>(&hwModeMask), sizeof(hwModeMask));
}

uint32_t MostHwMode::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedMostHwMode) +
        sizeof(hwMode) +
        sizeof(hwModeMask);
}

}
}

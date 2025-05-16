// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostStatisticEx.h>

namespace Vector {
namespace BLF {

MostStatisticEx::MostStatisticEx() :
    ObjectHeader2(ObjectType::MOST_STATISTICEX) {
}

void MostStatisticEx::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedMostStatisticEx1), sizeof(reservedMostStatisticEx1));
    is.read(reinterpret_cast<char *>(&codingErrors), sizeof(codingErrors));
    is.read(reinterpret_cast<char *>(&frameCounter), sizeof(frameCounter));
    is.read(reinterpret_cast<char *>(&reservedMostStatisticEx2), sizeof(reservedMostStatisticEx2));
}

void MostStatisticEx::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedMostStatisticEx1), sizeof(reservedMostStatisticEx1));
    os.write(reinterpret_cast<char *>(&codingErrors), sizeof(codingErrors));
    os.write(reinterpret_cast<char *>(&frameCounter), sizeof(frameCounter));
    os.write(reinterpret_cast<char *>(&reservedMostStatisticEx2), sizeof(reservedMostStatisticEx2));
}

uint32_t MostStatisticEx::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedMostStatisticEx1) +
        sizeof(codingErrors) +
        sizeof(frameCounter) +
        sizeof(reservedMostStatisticEx2);
}

}
}

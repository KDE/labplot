// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/MostDataLost.h>

namespace Vector {
namespace BLF {

MostDataLost::MostDataLost() :
    ObjectHeader2(ObjectType::MOST_DATALOST) {
}

void MostDataLost::read(AbstractFile & is) {
    ObjectHeader2::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedMostDataLost), sizeof(reservedMostDataLost));
    is.read(reinterpret_cast<char *>(&info), sizeof(info));
    is.read(reinterpret_cast<char *>(&lostMsgsCtrl), sizeof(lostMsgsCtrl));
    is.read(reinterpret_cast<char *>(&lostMsgsAsync), sizeof(lostMsgsAsync));
    is.read(reinterpret_cast<char *>(&lastGoodTimeStampNs), sizeof(lastGoodTimeStampNs));
    is.read(reinterpret_cast<char *>(&nextGoodTimeStampNs), sizeof(nextGoodTimeStampNs));
}

void MostDataLost::write(AbstractFile & os) {
    ObjectHeader2::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedMostDataLost), sizeof(reservedMostDataLost));
    os.write(reinterpret_cast<char *>(&info), sizeof(info));
    os.write(reinterpret_cast<char *>(&lostMsgsCtrl), sizeof(lostMsgsCtrl));
    os.write(reinterpret_cast<char *>(&lostMsgsAsync), sizeof(lostMsgsAsync));
    os.write(reinterpret_cast<char *>(&lastGoodTimeStampNs), sizeof(lastGoodTimeStampNs));
    os.write(reinterpret_cast<char *>(&nextGoodTimeStampNs), sizeof(nextGoodTimeStampNs));
}

uint32_t MostDataLost::calculateObjectSize() const {
    return
        ObjectHeader2::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedMostDataLost) +
        sizeof(info) +
        sizeof(lostMsgsCtrl) +
        sizeof(lostMsgsAsync) +
        sizeof(lastGoodTimeStampNs) +
        sizeof(nextGoodTimeStampNs);
}

}
}

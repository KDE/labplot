// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/AfdxStatus.h>

namespace Vector {
namespace BLF {

AfdxStatus::AfdxStatus() :
    ObjectHeader(ObjectType::AFDX_STATUS) {
}

void AfdxStatus::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedAfdxStatus1), sizeof(reservedAfdxStatus1));
    statusA.read(is);
    statusB.read(is);
    is.read(reinterpret_cast<char *>(&reservedAfdxStatus2), sizeof(reservedAfdxStatus2));
}

void AfdxStatus::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedAfdxStatus1), sizeof(reservedAfdxStatus1));
    statusA.write(os);
    statusB.write(os);
    os.write(reinterpret_cast<char *>(&reservedAfdxStatus2), sizeof(reservedAfdxStatus2));
}

uint32_t AfdxStatus::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedAfdxStatus1) +
        statusA.calculateObjectSize() +
        statusB.calculateObjectSize() +
        sizeof(reservedAfdxStatus2);
}

}
}

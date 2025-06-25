// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanDriverError.h>

namespace Vector {
namespace BLF {

CanDriverError::CanDriverError() :
    ObjectHeader(ObjectType::CAN_DRIVER_ERROR) {
}

void CanDriverError::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&txErrors), sizeof(txErrors));
    is.read(reinterpret_cast<char *>(&rxErrors), sizeof(rxErrors));
    is.read(reinterpret_cast<char *>(&errorCode), sizeof(errorCode));
}

void CanDriverError::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&txErrors), sizeof(txErrors));
    os.write(reinterpret_cast<char *>(&rxErrors), sizeof(rxErrors));
    os.write(reinterpret_cast<char *>(&errorCode), sizeof(errorCode));
}

uint32_t CanDriverError::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(txErrors) +
        sizeof(rxErrors) +
        sizeof(errorCode);
}

}
}

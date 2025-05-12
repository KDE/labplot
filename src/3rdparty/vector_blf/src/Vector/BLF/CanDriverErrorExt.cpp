// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanDriverErrorExt.h>

namespace Vector {
namespace BLF {

CanDriverErrorExt::CanDriverErrorExt() :
    ObjectHeader(ObjectType::CAN_DRIVER_ERROR_EXT) {
}

void CanDriverErrorExt::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&txErrors), sizeof(txErrors));
    is.read(reinterpret_cast<char *>(&rxErrors), sizeof(rxErrors));
    is.read(reinterpret_cast<char *>(&errorCode), sizeof(errorCode));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&state), sizeof(state));
    is.read(reinterpret_cast<char *>(&reservedCanDriverErrorExt1), sizeof(reservedCanDriverErrorExt1));
    is.read(reinterpret_cast<char *>(&reservedCanDriverErrorExt2), sizeof(reservedCanDriverErrorExt2));
    is.read(reinterpret_cast<char *>(reservedCanDriverErrorExt3.data()), static_cast<std::streamsize>(reservedCanDriverErrorExt3.size() * sizeof(uint32_t)));
}

void CanDriverErrorExt::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&txErrors), sizeof(txErrors));
    os.write(reinterpret_cast<char *>(&rxErrors), sizeof(rxErrors));
    os.write(reinterpret_cast<char *>(&errorCode), sizeof(errorCode));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&state), sizeof(state));
    os.write(reinterpret_cast<char *>(&reservedCanDriverErrorExt1), sizeof(reservedCanDriverErrorExt1));
    os.write(reinterpret_cast<char *>(&reservedCanDriverErrorExt2), sizeof(reservedCanDriverErrorExt2));
    os.write(reinterpret_cast<char *>(reservedCanDriverErrorExt3.data()), static_cast<std::streamsize>(reservedCanDriverErrorExt3.size() * sizeof(uint32_t)));
}

uint32_t CanDriverErrorExt::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(txErrors) +
        sizeof(rxErrors) +
        sizeof(errorCode) +
        sizeof(flags) +
        sizeof(state) +
        sizeof(reservedCanDriverErrorExt1) +
        sizeof(reservedCanDriverErrorExt2) +
        static_cast<uint32_t>(reservedCanDriverErrorExt3.size() * sizeof(uint32_t));
}

}
}

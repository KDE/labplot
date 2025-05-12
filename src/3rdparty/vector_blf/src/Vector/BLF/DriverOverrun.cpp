// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/DriverOverrun.h>

namespace Vector {
namespace BLF {

DriverOverrun::DriverOverrun() :
    ObjectHeader(ObjectType::OVERRUN_ERROR) {
}

void DriverOverrun::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&busType), sizeof(busType));
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedDriverOverrun), sizeof(reservedDriverOverrun));
}

void DriverOverrun::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&busType), sizeof(busType));
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedDriverOverrun), sizeof(reservedDriverOverrun));
}

uint32_t DriverOverrun::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(busType) +
        sizeof(channel) +
        sizeof(reservedDriverOverrun);
}

}
}

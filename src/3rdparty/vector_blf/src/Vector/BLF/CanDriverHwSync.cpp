// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanDriverHwSync.h>

namespace Vector {
namespace BLF {

CanDriverHwSync::CanDriverHwSync() :
    ObjectHeader(ObjectType::CAN_DRIVER_SYNC) {
}

void CanDriverHwSync::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&reservedCanDriverHwSync1), sizeof(reservedCanDriverHwSync1));
    is.read(reinterpret_cast<char *>(&reservedCanDriverHwSync2), sizeof(reservedCanDriverHwSync2));
}

void CanDriverHwSync::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&reservedCanDriverHwSync1), sizeof(reservedCanDriverHwSync1));
    os.write(reinterpret_cast<char *>(&reservedCanDriverHwSync2), sizeof(reservedCanDriverHwSync2));
}

uint32_t CanDriverHwSync::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(flags) +
        sizeof(reservedCanDriverHwSync1) +
        sizeof(reservedCanDriverHwSync2);
}

}
}

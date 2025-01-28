// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanOverloadFrame.h>

namespace Vector {
namespace BLF {

CanOverloadFrame::CanOverloadFrame() :
    ObjectHeader(ObjectType::CAN_OVERLOAD) {
}

void CanOverloadFrame::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&reservedCanOverloadFrame1), sizeof(reservedCanOverloadFrame1));
    is.read(reinterpret_cast<char *>(&reservedCanOverloadFrame2), sizeof(reservedCanOverloadFrame2));
}

void CanOverloadFrame::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&reservedCanOverloadFrame1), sizeof(reservedCanOverloadFrame1));
    os.write(reinterpret_cast<char *>(&reservedCanOverloadFrame2), sizeof(reservedCanOverloadFrame2));
}

uint32_t CanOverloadFrame::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(reservedCanOverloadFrame1) +
        sizeof(reservedCanOverloadFrame2);
}

}
}

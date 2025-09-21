// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/CanErrorFrame.h>

namespace Vector {
namespace BLF {

CanErrorFrame::CanErrorFrame() :
    ObjectHeader(ObjectType::CAN_ERROR) {
}

void CanErrorFrame::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&channel), sizeof(channel));
    is.read(reinterpret_cast<char *>(&length), sizeof(length));
    if (length > 0)
        is.read(reinterpret_cast<char *>(&reservedCanErrorFrame), sizeof(reservedCanErrorFrame));
}

void CanErrorFrame::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&channel), sizeof(channel));
    os.write(reinterpret_cast<char *>(&length), sizeof(length));
    if (length > 0)
        os.write(reinterpret_cast<char *>(&reservedCanErrorFrame), sizeof(reservedCanErrorFrame));
}

uint32_t CanErrorFrame::calculateObjectSize() const {
    uint32_t size =
        ObjectHeader::calculateObjectSize() +
        sizeof(channel) +
        sizeof(length);
    if (length > 0)
        size += sizeof(reservedCanErrorFrame);
    return size;
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/SerialEvent.h>

namespace Vector {
namespace BLF {

SerialEvent::SerialEvent() :
    ObjectHeader(ObjectType::SERIAL_EVENT) {
}

void SerialEvent::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&flags), sizeof(flags));
    is.read(reinterpret_cast<char *>(&port), sizeof(port));
    is.read(reinterpret_cast<char *>(&baudrate), sizeof(baudrate));
    is.read(reinterpret_cast<char *>(&reservedSerialEvent), sizeof(reservedSerialEvent));

    if (flags & Flags::SingleByte)
        singleByte.read(is);
    else {
        if (flags & Flags::CompactByte)
            compact.read(is);
        else
            general.read(is);
    }

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
    // @note might be extended in future versions
}

void SerialEvent::write(AbstractFile & os) {
    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&flags), sizeof(flags));
    os.write(reinterpret_cast<char *>(&port), sizeof(port));
    os.write(reinterpret_cast<char *>(&baudrate), sizeof(baudrate));
    os.write(reinterpret_cast<char *>(&reservedSerialEvent), sizeof(reservedSerialEvent));

    if (flags & Flags::SingleByte)
        singleByte.write(os);
    else {
        if (flags & Flags::CompactByte)
            compact.write(os);
        else
            general.write(os);
    }

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t SerialEvent::calculateObjectSize() const {
    uint32_t size =
        ObjectHeader::calculateObjectSize() +
        sizeof(flags) +
        sizeof(port) +
        sizeof(baudrate) +
        sizeof(reservedSerialEvent) +
        16; // size of union of singleByte/compact/general

    if (flags & ~(Flags::SingleByte | Flags::CompactByte))
        size += general.dataLength + general.timeStampsLength;

    return size;
}

}
}

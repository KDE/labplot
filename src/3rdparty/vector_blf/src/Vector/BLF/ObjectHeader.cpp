// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/ObjectHeader.h>

namespace Vector {
namespace BLF {

ObjectHeader::ObjectHeader(const ObjectType objectType, const uint16_t objectVersion) :
    ObjectHeaderBase(1, objectType),
    objectVersion(objectVersion) {
}

void ObjectHeader::read(AbstractFile & is) {
    ObjectHeaderBase::read(is);
    is.read(reinterpret_cast<char *>(&objectFlags), sizeof(objectFlags));
    is.read(reinterpret_cast<char *>(&clientIndex), sizeof(clientIndex));
    is.read(reinterpret_cast<char *>(&objectVersion), sizeof(objectVersion));
    is.read(reinterpret_cast<char *>(&objectTimeStamp), sizeof(objectTimeStamp));
}

void ObjectHeader::write(AbstractFile & os) {
    ObjectHeaderBase::write(os);
    os.write(reinterpret_cast<char *>(&objectFlags), sizeof(objectFlags));
    os.write(reinterpret_cast<char *>(&clientIndex), sizeof(clientIndex));
    os.write(reinterpret_cast<char *>(&objectVersion), sizeof(objectVersion));
    os.write(reinterpret_cast<char *>(&objectTimeStamp), sizeof(objectTimeStamp));
}

uint16_t ObjectHeader::calculateHeaderSize() const {
    return
        ObjectHeaderBase::calculateHeaderSize() +
        sizeof(objectFlags) +
        sizeof(clientIndex) +
        sizeof(objectVersion) +
        sizeof(objectTimeStamp);
}

uint32_t ObjectHeader::calculateObjectSize() const {
    return calculateHeaderSize();
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/ObjectHeader2.h>

namespace Vector {
namespace BLF {

ObjectHeader2::ObjectHeader2(const ObjectType objectType) :
    ObjectHeaderBase(2, objectType) {
}

void ObjectHeader2::read(AbstractFile & is) {
    ObjectHeaderBase::read(is);
    is.read(reinterpret_cast<char *>(&objectFlags), sizeof(objectFlags));
    is.read(reinterpret_cast<char *>(&timeStampStatus), sizeof(timeStampStatus));
    is.read(reinterpret_cast<char *>(&reservedObjectHeader), sizeof(reservedObjectHeader));
    is.read(reinterpret_cast<char *>(&objectVersion), sizeof(objectVersion));
    is.read(reinterpret_cast<char *>(&objectTimeStamp), sizeof(objectTimeStamp));
    is.read(reinterpret_cast<char *>(&originalTimeStamp), sizeof(originalTimeStamp));
}

void ObjectHeader2::write(AbstractFile & os) {
    ObjectHeaderBase::write(os);
    os.write(reinterpret_cast<char *>(&objectFlags), sizeof(objectFlags));
    os.write(reinterpret_cast<char *>(&timeStampStatus), sizeof(timeStampStatus));
    os.write(reinterpret_cast<char *>(&reservedObjectHeader), sizeof(reservedObjectHeader));
    os.write(reinterpret_cast<char *>(&objectVersion), sizeof(objectVersion));
    os.write(reinterpret_cast<char *>(&objectTimeStamp), sizeof(objectTimeStamp));
    os.write(reinterpret_cast<char *>(&originalTimeStamp), sizeof(originalTimeStamp));
}

uint16_t ObjectHeader2::calculateHeaderSize() const {
    return
        ObjectHeaderBase::calculateHeaderSize() +
        sizeof(objectFlags) +
        sizeof(timeStampStatus) +
        sizeof(reservedObjectHeader) +
        sizeof(objectVersion) +
        sizeof(objectTimeStamp) +
        sizeof(originalTimeStamp);
}

uint32_t ObjectHeader2::calculateObjectSize() const {
    return calculateHeaderSize();
}

}
}

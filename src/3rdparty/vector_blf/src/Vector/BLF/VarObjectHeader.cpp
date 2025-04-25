// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/VarObjectHeader.h>

namespace Vector {
namespace BLF {

VarObjectHeader::VarObjectHeader(const ObjectType objectType) :
    ObjectHeaderBase(1, objectType) {
}

void VarObjectHeader::read(AbstractFile & is) {
    ObjectHeaderBase::read(is);
    is.read(reinterpret_cast<char *>(&objectFlags), sizeof(objectFlags));
    is.read(reinterpret_cast<char *>(&objectStaticSize), sizeof(objectStaticSize));
    is.read(reinterpret_cast<char *>(&objectVersion), sizeof(objectVersion));
    is.read(reinterpret_cast<char *>(&objectTimeStamp), sizeof(objectTimeStamp));
}

void VarObjectHeader::write(AbstractFile & os) {
    ObjectHeaderBase::write(os);
    os.write(reinterpret_cast<char *>(&objectFlags), sizeof(objectFlags));
    os.write(reinterpret_cast<char *>(&objectStaticSize), sizeof(objectStaticSize));
    os.write(reinterpret_cast<char *>(&objectVersion), sizeof(objectVersion));
    os.write(reinterpret_cast<char *>(&objectTimeStamp), sizeof(objectTimeStamp));
}

uint16_t VarObjectHeader::calculateHeaderSize() const {
    return
        ObjectHeaderBase::calculateHeaderSize() +
        sizeof(objectFlags) +
        sizeof(objectStaticSize) +
        sizeof(objectVersion) +
        sizeof(objectTimeStamp);
}

uint32_t VarObjectHeader::calculateObjectSize() const {
    return calculateHeaderSize();
}

}
}

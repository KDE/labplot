// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/SystemVariable.h>

namespace Vector {
namespace BLF {

SystemVariable::SystemVariable() :
    ObjectHeader(ObjectType::SYS_VARIABLE) {
}

void SystemVariable::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&representation), sizeof(representation));
    is.read(reinterpret_cast<char *>(&reservedSystemVariable1), sizeof(reservedSystemVariable1));
    is.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    is.read(reinterpret_cast<char *>(&reservedSystemVariable2), sizeof(reservedSystemVariable2));
    name.resize(nameLength);
    is.read(const_cast<char *>(name.data()), nameLength);
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), dataLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void SystemVariable::write(AbstractFile & os) {
    /* pre processing */
    nameLength = static_cast<uint32_t>(name.size());
    dataLength = static_cast<uint32_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&representation), sizeof(representation));
    os.write(reinterpret_cast<char *>(&reservedSystemVariable1), sizeof(reservedSystemVariable1));
    os.write(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(reinterpret_cast<char *>(&reservedSystemVariable2), sizeof(reservedSystemVariable2));
    os.write(const_cast<char *>(name.data()), nameLength);
    os.write(reinterpret_cast<char *>(data.data()), dataLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t SystemVariable::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(type) +
        sizeof(representation) +
        sizeof(reservedSystemVariable1) +
        sizeof(nameLength) +
        sizeof(dataLength) +
        sizeof(reservedSystemVariable2) +
        nameLength +
        dataLength;
}

}
}

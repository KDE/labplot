// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <cstring>

#include <Vector/BLF/EnvironmentVariable.h>

namespace Vector {
namespace BLF {

EnvironmentVariable::EnvironmentVariable(/*const ObjectType objectType*/) :
    ObjectHeader(ObjectType::UNKNOWN) {
    /* can be one of:
     *   - objectType = ObjectType::ENV_INTEGER;
     *   - objectType = ObjectType::ENV_DOUBLE;
     *   - objectType = ObjectType::ENV_STRING;
     *   - objectType = ObjectType::ENV_DATA;
     */
}

void EnvironmentVariable::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    is.read(reinterpret_cast<char *>(&reservedEnvironmentVariable), sizeof(reservedEnvironmentVariable));
    name.resize(nameLength);
    is.read(const_cast<char *>(name.data()), nameLength);
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), dataLength);

    /* skip padding */
    is.seekg(objectSize % 4, std::ios_base::cur);
}

void EnvironmentVariable::write(AbstractFile & os) {
    /* pre processing */
    nameLength = static_cast<uint32_t>(name.size());
    dataLength = static_cast<uint32_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(reinterpret_cast<char *>(&reservedEnvironmentVariable), sizeof(reservedEnvironmentVariable));
    os.write(const_cast<char *>(name.data()), nameLength);
    os.write(reinterpret_cast<char *>(data.data()), dataLength);

    /* skip padding */
    os.skipp(objectSize % 4);
}

uint32_t EnvironmentVariable::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(nameLength) +
        sizeof(dataLength) +
        sizeof(reservedEnvironmentVariable) +
        nameLength +
        dataLength;
}

}
}

// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/FunctionBus.h>

namespace Vector {
namespace BLF {

FunctionBus::FunctionBus() :
    ObjectHeader(ObjectType::FUNCTION_BUS) {
}

void FunctionBus::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&functionBusObjectType), sizeof(functionBusObjectType));
    is.read(reinterpret_cast<char *>(&veType), sizeof(veType));
    is.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    is.read(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    name.resize(nameLength);
    is.read(const_cast<char *>(name.data()), nameLength);
    data.resize(dataLength);
    is.read(reinterpret_cast<char *>(data.data()), dataLength);
}

void FunctionBus::write(AbstractFile & os) {
    /* pre processing */
    nameLength = static_cast<uint32_t>(name.size());
    dataLength = static_cast<uint32_t>(data.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&functionBusObjectType), sizeof(functionBusObjectType));
    os.write(reinterpret_cast<char *>(&veType), sizeof(veType));
    os.write(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    os.write(reinterpret_cast<char *>(&dataLength), sizeof(dataLength));
    os.write(const_cast<char *>(name.data()), nameLength);
    os.write(reinterpret_cast<char *>(data.data()), dataLength);
}

uint32_t FunctionBus::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(functionBusObjectType) +
        sizeof(veType) +
        sizeof(nameLength) +
        sizeof(dataLength) +
        nameLength +
        dataLength;
}

}
}

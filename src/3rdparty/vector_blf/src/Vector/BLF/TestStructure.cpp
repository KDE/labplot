// SPDX-FileCopyrightText: 2013-2021 Tobias Lorenz <tobias.lorenz@gmx.net>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <Vector/BLF/TestStructure.h>

namespace Vector {
namespace BLF {

TestStructure::TestStructure() :
    ObjectHeader(ObjectType::TEST_STRUCTURE) {
}

void TestStructure::read(AbstractFile & is) {
    ObjectHeader::read(is);
    is.read(reinterpret_cast<char *>(&executionObjectIdentify), sizeof(executionObjectIdentify));
    is.read(reinterpret_cast<char *>(&type), sizeof(type));
    is.read(reinterpret_cast<char *>(&reservedTestStructure), sizeof(reservedTestStructure));
    is.read(reinterpret_cast<char *>(&uniqueNo), sizeof(uniqueNo));
    is.read(reinterpret_cast<char *>(&action), sizeof(action));
    is.read(reinterpret_cast<char *>(&result), sizeof(result));
    is.read(reinterpret_cast<char *>(&executingObjectNameLength), sizeof(executingObjectNameLength));
    is.read(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    is.read(reinterpret_cast<char *>(&textLength), sizeof(textLength));
    executingObjectName.resize(executingObjectNameLength);
    is.read(const_cast<char *>(reinterpret_cast<const char *>(executingObjectName.data())), executingObjectNameLength * sizeof(char16_t));
    name.resize(nameLength);
    is.read(const_cast<char *>(reinterpret_cast<const char *>(name.data())), nameLength * sizeof(char16_t));
    text.resize(textLength);
    is.read(const_cast<char *>(reinterpret_cast<const char *>(text.data())), textLength * sizeof(char16_t));
}

void TestStructure::write(AbstractFile & os) {
    /* pre processing */
    executingObjectNameLength = static_cast<uint32_t>(executingObjectName.size());
    nameLength = static_cast<uint32_t>(name.size());
    textLength = static_cast<uint32_t>(text.size());

    ObjectHeader::write(os);
    os.write(reinterpret_cast<char *>(&executionObjectIdentify), sizeof(executionObjectIdentify));
    os.write(reinterpret_cast<char *>(&type), sizeof(type));
    os.write(reinterpret_cast<char *>(&reservedTestStructure), sizeof(reservedTestStructure));
    os.write(reinterpret_cast<char *>(&uniqueNo), sizeof(uniqueNo));
    os.write(reinterpret_cast<char *>(&action), sizeof(action));
    os.write(reinterpret_cast<char *>(&result), sizeof(result));
    os.write(reinterpret_cast<char *>(&executingObjectNameLength), sizeof(executingObjectNameLength));
    os.write(reinterpret_cast<char *>(&nameLength), sizeof(nameLength));
    os.write(reinterpret_cast<char *>(&textLength), sizeof(textLength));
    os.write(const_cast<char *>(reinterpret_cast<const char *>(executingObjectName.data())), executingObjectNameLength * sizeof(char16_t));
    os.write(const_cast<char *>(reinterpret_cast<const char *>(name.data())), nameLength * sizeof(char16_t));
    os.write(const_cast<char *>(reinterpret_cast<const char *>(text.data())), textLength * sizeof(char16_t));
}

uint32_t TestStructure::calculateObjectSize() const {
    return
        ObjectHeader::calculateObjectSize() +
        sizeof(executionObjectIdentify) +
        sizeof(type) +
        sizeof(reservedTestStructure) +
        sizeof(uniqueNo) +
        sizeof(action) +
        sizeof(result) +
        sizeof(executingObjectNameLength) +
        sizeof(nameLength) +
        sizeof(textLength) +
        (executingObjectNameLength + nameLength + textLength) * sizeof(char16_t);
}

}
}
